import asyncio
import time

try:
    import ipywidgets
except ImportError:
    pass

import numpy as np
import plotly.graph_objs as go
import zmq
from IPython.display import display

from . import msg_pb2

try:
    if msg_pb2.IS_PLOTMSG_PROTOBUF_MSG_PLACE_HOLDER:
        raise RuntimeError(
            f"The file '{msg_pb2.__file__}' appears to be a placeholder file. "
            "Have you compiled this module correctly and installed the corresponding python package?"
        )
except AttributeError:
    pass

CPP2PY_ADDRESS = "tcp://127.0.0.1:5557"
CPP2PY_MODE_DEFAULT = "default"
CPP2PY_MODE_ASYNC = "async"
CPP2PY_MODE_WIDGET = "ipywidget"


# helper decorator to only execute ipywidget related code
def ipywidget_mode(warn=False):
    def decorator(f):
        def wrapper(self, *args, **kwargs):
            if self.mode == CPP2PY_MODE_WIDGET:
                return f(self, *args, **kwargs)
            if warn:
                print("Only works in jupyter notebook (ipywidget mode)")
            return

        return wrapper

    return decorator


# helper function to check whether it's currently within jupyter notebook
def inside_notebook():
    try:
        shell = get_ipython().__class__.__name__
        if shell == "ZMQInteractiveShell":
            return True  # Jupyter notebook or qtconsole
        elif shell == "TerminalInteractiveShell":
            return False  # Terminal running IPython
        else:
            return False  # Other type (?)
    except NameError:
        return False  # Probably standard Python


class DummyCtxMgr:
    def __enter__(self):
        pass

    def __exit__(self, exc_type, exc_val, exc_tb):
        pass


class DummyClass:
    dummy_func = lambda *args, **kwargs: None

    def __getattr__(self, attr):
        return self.dummy_func


class Cpp2PyReciever:
    """A class that listen to message from cpp"""

    def __init__(self, address=CPP2PY_ADDRESS, ctx_mgr=None):
        self.address = address
        self.socket = None
        self.mode = None
        if ctx_mgr is None:
            ctx_mgr = DummyCtxMgr()
        self.ctx_mgr = ctx_mgr

    @staticmethod
    def unpack_msg(msg):
        """Recursive unpack method"""

        def unpack(inputs):
            inputs_t = type(inputs)
            if inputs_t is msg_pb2.Dictionary:
                return {k: unpack(v) for (k, v) in inputs.data.items()}
            if inputs_t is msg_pb2.DictItemVal:
                return unpack(getattr(inputs, inputs.WhichOneof("value")))
            elif inputs_t in (msg_pb2.SeriesI, msg_pb2.SeriesD):
                return np.array(inputs.data)
            elif inputs_t is msg_pb2.SeriesString:
                return list(inputs.data)
            elif inputs_t is msg_pb2.SeriesAny:
                out = []
                for d in inputs.data:
                    which = d.WhichOneof("value")
                    if which == "null":
                        out.append(None)
                    else:
                        out.append(getattr(d, which))
                return out
            elif inputs_t in (bool, str, float, int):
                return inputs
            elif inputs_t == msg_pb2.Trace:
                return dict(
                    method=msg_pb2.Trace.CreationMethods.Name(inputs.method),
                    func=inputs.method_func,
                    kwargs=unpack(inputs.kwargs),
                )
            elif inputs_t == msg_pb2.Figure:
                return dict(
                    uuid=inputs.uuid,
                    traces=[unpack(t) for t in inputs.traces],
                    commands=[
                        dict(func=cmd.func, kwargs=unpack(cmd.kwargs))
                        for cmd in inputs.commands
                    ],
                )
            else:
                raise RuntimeError("Unrecognised type {}".format(inputs_t))

        return unpack(getattr(msg, msg.WhichOneof("message")))

    # noinspection PyUnresolvedReferences
    def initialise(self, sleep=1, mode=CPP2PY_MODE_DEFAULT):
        if self.mode == mode:
            return
        if mode == CPP2PY_MODE_ASYNC:
            context = zmq.asyncio.Context()
        elif mode == CPP2PY_MODE_DEFAULT:
            context = zmq.Context()
        else:
            raise ValueError("Unknown mode {}".format(mode))
        self.mode = mode
        socket = context.socket(zmq.SUB)
        socket.connect(self.address)
        socket.setsockopt_string(zmq.SUBSCRIBE, "")
        self.socket = socket
        time.sleep(sleep)

    def _get_msg(self, encoded_msg):
        msg = msg_pb2.MessageContainer()
        msg.ParseFromString(encoded_msg)
        return self.unpack_msg(msg)  # uuid, fig_kwargs

    def get_msg_func(self, flags=0):
        """Return a function that process the incoming encoded msg"""
        self.initialise(mode=CPP2PY_MODE_DEFAULT)
        encoded_msg = self.socket.recv(flags=flags)
        return lambda: self._get_msg(encoded_msg)

    def get_msg(self, flags=0):
        return self.get_msg_func(flags)()

    async def get_msg_async_func(self):
        """Return a function that process the incoming encoded msg, asyncly"""
        self.initialise(mode=CPP2PY_MODE_ASYNC)
        encoded_msg = await self.socket.recv()
        return lambda: self._get_msg(encoded_msg)

    async def get_msg_async(self):
        return (await self.get_msg_async_func())()


class Cpp2PyPlotly:
    class InfoLabelCtxMgr:
        """A class that represent a context manager for usage during processing msg."""

        def __init__(self, c2p: "Cpp2PyPlotly"):
            self.label = ipywidgets.HTML()
            self.c2p = c2p
            self.label_format = (
                "<i class='fa fa-{icon}'></i> <b>{short_txt}</b>  |  "
                "<b>Last msg:</b> {timestamp}  |  "
                "<b><u>Stored</u> figs:</b> {num_figs} "
                "<b>msgs:</b> {num_msgs}  |  "
                "<b>Historic msgs:</b> {hist_num_msgs}"
            )
            self.last_msg_ts = "No msg."

        def __enter__(self):
            # self.last_msg_ts = time.strftime('%d-%m_%H:%M', time.localtime())
            self.last_msg_ts = time.strftime("%H:%M", time.localtime())
            self.update_label("Processing", "retweet")
            self.c2p.hist_num_msgs += 1

        def __exit__(self, exc_type, exc_value, exc_traceback):
            icon = "check"
            short_txt = "OK!"
            if exc_type is not None:
                icon = "times"
                short_txt = "Error occured (check captured log)"
            self.update_label(short_txt, icon)

        def update_label(self, short_txt, icon="check"):
            self.label.value = self.label_format.format(
                icon=icon,
                short_txt=short_txt,
                timestamp=self.last_msg_ts,
                num_figs=self.c2p.num_figs,
                num_msgs=self.c2p.num_msgs,
                hist_num_msgs=self.c2p.hist_num_msgs,
            )

    class ProgressBarCtxMgr:
        """A class that represent a context manager for usage during processing msg."""

        def __init__(self):
            self.w_progress_bar = ipywidgets.IntProgress(
                value=0, min=0, max=1, description="Progress:", bar_style="info"
            )
            self.w_progress_bar.layout.display = "none"

        def add(self):
            self.w_progress_bar.value += 1
            if self.w_progress_bar.value == self.w_progress_bar.max:
                self.w_progress_bar.bar_style = "success"

        def start(self, num: int):
            self.w_progress_bar.layout.display = ""
            self.w_progress_bar.value = 0
            self.w_progress_bar.max = num

    #################################################

    def __init__(
        self,
        address: str = CPP2PY_ADDRESS,
        mode: str = CPP2PY_MODE_WIDGET,
        initialise: bool = True,
        figure_type=None,
    ):
        # the stored figs is a singleton
        if not hasattr(self.__class__, "stored_figs"):
            self.__class__.stored_figs = {}
            self.__class__.stored_msgs = []
        self.figs = self.__class__.stored_figs
        self.msgs = self.__class__.stored_msgs
        self.hist_num_msgs = 0

        if mode == CPP2PY_MODE_WIDGET:
            if not inside_notebook():
                # cannot runs in ipywidget mode
                mode = CPP2PY_MODE_DEFAULT

        if mode.startswith(CPP2PY_MODE_WIDGET):
            mode = CPP2PY_MODE_WIDGET
            self._initialise_as_ipywidgets()
            self.reciever = Cpp2PyReciever(
                address=address, ctx_mgr=self.ctx_mgr_info_label
            )
            self.goFigClass = go.FigureWidget

        elif mode == CPP2PY_MODE_DEFAULT:
            self.reciever = Cpp2PyReciever(address=address)
            self.ctx_mgr_chained = DummyCtxMgr
            self.ctx_mgr_pbar = DummyClass()
            self.goFigClass = go.Figure
        else:
            raise RuntimeError("Unrecognised mode '{}'".format(mode))
        self.mode = mode
        if figure_type is not None:
            if figure_type == "Figure":
                self.goFigClass = go.Figure
            elif figure_type == "FigureWidget":
                self.goFigClass = go.FigureWidget
            else:
                raise RuntimeError("Unrecognised figure_type '{}'".format(figure_type))
        # reciever for msg from cpp side
        self.async_task = None
        if initialise:
            self.initialise()

    def __del__(self):
        # clean up any running async task
        if self.async_task is not None:
            self.async_task.cancel()

    def initialise(self):
        self.reciever.initialise()

    def _initialise_as_ipywidgets(self):
        self.w_multi_fig_sel = None
        self.w_single_fig_sel = None
        #######################
        self.w_refresh_btn = ipywidgets.Button(
            description="Refresh",
            disabled=False,
            button_style="",  # 'success', 'info', 'warning', 'danger' or ''
            tooltip="Process plotly incoming messages",
            icon="retweet",
        )
        self.w_refresh_btn.on_click(lambda x: self.spin_once(verbose=False))
        ##
        self.ctx_mgr_captured_log = ipywidgets.widgets.Output(
            layout={"border": "1px solid black"}
        )
        #######################
        self.w_resize_auto_toggle = ipywidgets.Checkbox(
            value=True,
            description="Auto-resize",
            tooltip="auto resize figures to fit screen",
            icon="arrows-alt-h",
            indent=False,
            layout=dict(margin="0px 0px 0px 20px"),
        )
        self.w_resize_width = ipywidgets.IntText(
            value=600,
            step=10,
            description="width:",
            layout=dict(width="120pt"),
        )
        self.w_resize_height = ipywidgets.IntText(
            value=600,
            step=10,
            description="height:",
            layout=dict(width="120pt"),
        )

        def resize_figure(figure):
            figure.update_layout(
                autosize=False,
                width=self.w_resize_width.value,
                height=self.w_resize_height.value,
            )

        def chkbox_on_change(event):
            self.w_resize_width.disabled = event["new"]
            self.w_resize_height.disabled = event["new"]
            if self.w_single_fig_sel.value is None:
                return
            active_fig = self.figs[self.w_single_fig_sel.value]
            # allow controlling of the figure size
            if event["new"]:
                active_fig.update_layout(autosize=True, width=None, height=None)
            else:
                resize_figure(active_fig)

        def resize_input_box_on_change(event):
            if self.w_single_fig_sel.value is None:
                return
            active_fig = self.figs[self.w_single_fig_sel.value]
            resize_figure(active_fig)

        self.w_resize_auto_toggle.observe(chkbox_on_change, "value")
        self.w_resize_width.observe(resize_input_box_on_change, "value")
        self.w_resize_height.observe(resize_input_box_on_change, "value")
        #######################
        self.ctx_mgr_info_label = self.__class__.InfoLabelCtxMgr(self)
        self.ctx_mgr_info_label.update_label("Initialised")
        self.ctx_mgr_pbar = self.__class__.ProgressBarCtxMgr()
        w_clear_msgs = ipywidgets.Button(
            description="Clear msgs",
            button_style="warning",  # 'danger' or ''
            tooltip="Process plotly incoming messages",
            icon="envelope-square",
        )
        w_clear_figs = ipywidgets.Button(
            description="Clear figs",
            button_style="danger",  # 'danger' or ''
            tooltip="Process plotly incoming messages",
            icon="file-image",
        )

        def on_clear_msgs(_):
            self.msgs.clear()
            self.ctx_mgr_info_label.update_label("Cleared msgs")

        def on_clear_figs(_):
            self.remove_figure_widget([uuid for uuid in self.figs])
            self.ctx_mgr_info_label.update_label("Cleared figs")
            self._update_selection()

        w_clear_msgs.on_click(on_clear_msgs)
        w_clear_figs.on_click(on_clear_figs)
        self.w_info_containers = ipywidgets.Tab(
            children=[
                ipywidgets.VBox(
                    children=[
                        ipywidgets.HBox(
                            children=[
                                self.w_refresh_btn,
                                w_clear_msgs,
                                w_clear_figs,
                                self.w_resize_width,
                                self.w_resize_height,
                                self.w_resize_auto_toggle,
                                self.ctx_mgr_pbar.w_progress_bar,
                            ]
                        ),
                        self.ctx_mgr_info_label.label,
                    ]
                ),
                self.ctx_mgr_captured_log,
            ]
        )
        self.w_info_containers.set_title(0, "Controls")
        self.w_info_containers.set_title(1, "Logs")
        ##
        from contextlib import contextmanager

        @contextmanager
        def ctx_mgr_chained():
            with self.ctx_mgr_captured_log:
                with self.ctx_mgr_info_label:
                    yield

        self.ctx_mgr_chained = ctx_mgr_chained

    @property
    def spinning_asyncly(self) -> bool:
        if self.async_task is not None and not self.async_task.cancelled():
            return True
        return False

    @property
    def num_figs(self) -> int:
        return len(self.figs)

    @property
    def num_msgs(self) -> int:
        return len(self.msgs)

    def parse_msg_to_plotly_fig(self, msg):
        """Give a parsed msg (in terms of dict and friends), add a plotly figure."""
        self.msgs.append([False, msg])
        if "uuid" not in msg:
            # not a fig message
            return
        traces = []
        uuid = msg["uuid"]
        # setup progress bar widget
        self.ctx_mgr_pbar.start(len(msg["traces"]))
        for t in msg["traces"]:
            method = t["method"]
            func = t["func"]
            if func == "":  # default to scatter
                func = "scatter"
            if method == "graph_objects":
                import plotly.graph_objects

                # func = func.title()  # this should be specified by the library itself
                traces.append(getattr(plotly.graph_objects, func)(**t["kwargs"]))
            elif method == "plotly_express":
                import plotly.express

                traces.extend(getattr(plotly.express, func)(**t["kwargs"]).data)
            elif method == "figure_factory":
                import plotly.figure_factory

                traces.extend(getattr(plotly.figure_factory, func)(**t["kwargs"]).data)
            else:
                raise NotImplementedError(method)
            self.ctx_mgr_pbar.add()  # update progress

        # successfully parsed message. Update stored_msgs
        # create the actual figure
        plotly_fig = self.goFigClass(traces)
        # operates action on the figure object
        for cmd in msg["commands"]:
            getattr(plotly_fig, cmd["func"])(**cmd["kwargs"])

        self.msgs[-1][0] = True
        # self.update_figure_widget(plotly_fig, uuid=uuid)
        self.add_figure_widget(plotly_fig, uuid=uuid)
        if self.mode == CPP2PY_MODE_DEFAULT:
            plotly_fig.show()

    def spin_once(self, verbose=False):
        """spin once to process all pending messsages"""
        # noinspection PyTypeChecker,PyUnresolvedReferences
        return self.spin(flags=zmq.NOBLOCK, exception_to_except=zmq.Again)

    def spin(self, flags=0, exception_to_except=KeyboardInterrupt):
        """spin forever until user interupt"""
        if self.spinning_asyncly:
            print("Already spinning asyncly.")
            return
        while True:
            try:
                process_msg_func = self.reciever.get_msg_func(flags)
            except exception_to_except as e:
                break
            with self.ctx_mgr_chained():
                self.parse_msg_to_plotly_fig(process_msg_func())

    @ipywidget_mode(True)
    def spin_async(self, display_log=False):
        if self.mode == CPP2PY_MODE_DEFAULT:
            return
        # default display log
        if display_log:
            self.display_parsing_log()
        if self.spinning_asyncly:
            print("Already spinning asyncly.")
            return

        async def _spin_async():
            while True:
                process_msg_func = await self.reciever.get_msg_async_func()
                with self.ctx_mgr_chained():
                    self.parse_msg_to_plotly_fig(process_msg_func())

        self.async_task = asyncio.create_task(_spin_async())

    ################################################################################

    @ipywidget_mode(True)
    def _update_selection(self):
        # force refresh by unsetting and setting the selection
        for widget in (self.w_multi_fig_sel, self.w_single_fig_sel):
            if widget is not None:
                widget.options = self.figs.keys()
                # store current selection, unset then set to force refresh
                prev_sel = widget.value
                if type(widget) is ipywidgets.widgets.widget_selection.SelectMultiple:
                    widget.value = []
                elif type(widget) is ipywidgets.widgets.widget_selection.Dropdown:
                    widget.value = None
                if prev_sel not in widget.options:
                    prev_sel = None  # not exists anymore
                    if len(widget.options) > 0:
                        prev_sel = widget.options[0]  # default to first item
                if prev_sel:
                    widget.value = prev_sel

    @ipywidget_mode(True)
    def remove_figure_widget(self, uuids):
        """Overwrite matching widget."""
        # remove selection options
        if type(uuids) == str:
            uuids = [uuids]
        for uuid in uuids:
            try:
                wid = self.figs[uuid]
                if type(wid) is go.FigureWidget:
                    wid.close()
                del self.figs[uuid]
            except KeyError:
                pass
        self._update_selection()

    @ipywidget_mode(False)
    def add_figure_widget(self, widget, uuid="default"):
        """Overwrite any existing widget."""
        assert type(widget) is self.goFigClass, type(widget)

        if uuid in self.figs:
            print("FIX THIS")
            return self.update_figure_widget(widget, uuid)

        # remove selection options
        self.remove_figure_widget(uuid)
        self.figs[uuid] = widget

        self._update_selection()

    @ipywidget_mode(False)
    def update_figure_widget(self, widget, uuid="default"):
        assert type(widget) is go.FigureWidget, type(widget)
        """WARN: Assumes the line sequence are in the same order"""
        assert uuid in self.figs
        assert len(self.figs[uuid].data) == len(widget.data)

        def _update_attr(existing, new):
            for _attr in new:
                cur_attr = existing[_attr]
                new_attr = new[_attr]
                # actual update of existing plotly figure is slow.
                # so we will opt to only update attribute that are
                # different (with overhead of checking equality)
                # if type is np array, we don't bother to check for equality
                # nope. we will check shape and eq_val
                if cur_attr is None or new_attr is None:
                    # if any is None, type will obviously be different. Update this.
                    pass
                elif type(cur_attr) != type(new_attr):
                    # NOT possible to update this.
                    print(
                        f"WARN: The attr {_attr} for a new incoming msg is "
                        f"different than the existing one. "
                        f"Was type {type(cur_attr)}, now {type(new_attr)}"
                    )
                    raise NotImplementedError("Should recreate the figure instead.")
                elif isinstance(new_attr, np.ndarray):
                    if np.array_equal(cur_attr, new_attr):
                        continue
                elif type(new_attr) is dict:
                    _update_attr(cur_attr, new_attr)
                    continue
                elif cur_attr == new_attr:
                    continue
                existing[_attr] = new_attr

        for stored_seq, new_widget_seq in zip(self.figs[uuid].data, widget.data):
            _update_attr(stored_seq, new_widget_seq)

        self._update_selection()

    def __repr__(self):
        return (
            f"{self.__class__.__name__}<figs:{self.num_figs}|msgs:{self.num_msgs}|"
            f"total_recieved:{self.hist_num_msgs}>"
        )

    ########################################
    ## multi-figs widget
    ########################################
    @ipywidget_mode()
    def display_ipywidget_multi_figs(self):
        self.w_multi_fig_sel = ipywidgets.widgets.SelectMultiple(
            options=[], value=[], description="Multi Fig(s)"
        )
        self.w_multi_fig_sel.options = self.figs.keys()
        if self.mode == CPP2PY_MODE_WIDGET:
            display(self.w_info_containers)

        @ipywidgets.interact(widget_names=self.w_multi_fig_sel)
        def on_change(widget_names):
            return ipywidgets.HBox(children=[self.figs[name] for name in widget_names])

    ########################################
    ## single-fig widget
    ########################################
    @ipywidget_mode()
    def display_ipywidget_single_fig(self):
        self.w_single_fig_sel = ipywidgets.widgets.Dropdown(
            options=[], description="Show Fig"
        )
        self.w_single_fig_sel.options = self.figs.keys()
        if self.mode == CPP2PY_MODE_WIDGET:
            display(self.w_info_containers)

        @ipywidgets.interact(widget_names=self.w_single_fig_sel)
        def on_change(widget_names):
            if widget_names:
                return self.figs[widget_names]

    @ipywidget_mode()
    def display_parsing_log(self):
        display(self.ctx_mgr_captured_log)

    @ipywidget_mode()
    def clear_parsing_log(self):
        self.ctx_mgr_captured_log.clear_output()

    ################################################################################

    @ipywidget_mode()
    def OLD_get_ipywidget_multi_figs(self):
        self.w_multi_fig_sel = ipywidgets.widgets.SelectMultiple(
            options=[], value=[], description="Multi Fig(s)"
        )
        inner_figs_container = ipywidgets.HBox(children=[])
        inner_figs_container = ipywidgets.widgets.GridBox(
            children=[],
            layout=ipywidgets.Layout(grid_template_columns="repeat(2, 1fr)"),
        )

        def on_change(event):
            inner_figs_container.children = [self.figs[name] for name in event["new"]]

        self.w_multi_fig_sel.observe(on_change, "value")
        self.w_multi_fig_sel.options = self.figs.keys()
        ## Outer widget
        widget = ipywidgets.VBox(children=[self.w_multi_fig_sel, inner_figs_container])
        display(widget)

    @ipywidget_mode()
    def OLD_get_ipywidget_single_fig(self):
        self.w_single_fig_sel = ipywidgets.widgets.Dropdown(
            options=[], description="Show Fig"
        )
        inner_figs_container = ipywidgets.HBox(children=[])

        def on_change(event):
            inner_figs_container.children = [self.figs[event["new"]]]

        self.w_single_fig_sel.observe(on_change, "value")
        self.w_single_fig_sel.options = self.figs.keys()
        ## Outer widget
        widget = ipywidgets.VBox(children=[self.w_single_fig_sel, inner_figs_container])
        display(widget)


# TODO: work on re-selecting previous figure after updating/creating figure(s)
