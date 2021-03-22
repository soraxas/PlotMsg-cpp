# import sys
# import os
import sys
import time

import ipywidgets

sys.path.insert(0, "/home/soraxas/git-repo/cpp2py_plotly/build/src/protobuf_msg/")

import msg_pb2
import numpy as np
import plotly.graph_objs as go
import zmq
import asyncio
from IPython.display import display

CPP2PY_ADDRESS = "tcp://127.0.0.1:5557"
CPP2PY_MODE_DEFAULT = "default"
CPP2PY_MODE_ASYNC = "async"
CPP2PY_MODE_WIDGET = "ipywidget"


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


class DummyCtxMgr:
    def __enter__(self):
        pass

    def __exit__(self, exc_type, exc_val, exc_tb):
        pass


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
            if type(inputs) is msg_pb2.Dictionary:
                return {k: unpack(v) for (k, v) in inputs.data.items()}
            if type(inputs) is msg_pb2.DictItemVal:
                return unpack(getattr(inputs, inputs.WhichOneof('value')))
            elif type(inputs) in (msg_pb2.SeriesI, msg_pb2.SeriesD):
                return np.array(inputs.data)
            elif type(inputs) in (bool, str, float, int):
                return inputs
            else:
                raise RuntimeError("Unrecognised type {}".format(type(inputs)))

        return msg.uuid, unpack(msg.kwargs)

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
        msg = msg_pb2.Figure()
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

        def __init__(self, label, stored_figs):
            self.label = label
            self.stored_figs = stored_figs
            self.num_msgs = 0
            self.label_format = "<i class='fa fa-{icon}'></i> <b>{short_txt}</b>  " \
                                "|  <b>Last msg:</b> {timestamp}  " \
                                "|  <b>Total num figs:</b> {num_fig}, " \
                                "<b>num msgs:</b> {num_msgs}"
            self.last_msg_ts = "No msg."

        def __enter__(self):
            # self.last_msg_ts = time.strftime('%d-%m_%H:%M', time.localtime())
            self.last_msg_ts = time.strftime('%H:%M', time.localtime())
            self.label.value = self.label_format.format(
                icon='retweet',
                short_txt='Processing',
                timestamp=self.last_msg_ts,
                num_fig=len(self.stored_figs),
                num_msgs=self.num_msgs,
            )
            self.num_msgs += 1

        def __exit__(self, exc_type, exc_value, exc_traceback):
            icon = 'check'
            short_txt = 'OK!'
            if exc_type is not None:
                icon = 'times'
                short_txt = 'Error occured (check captured log)'
            self.label.value = self.label_format.format(
                icon=icon,
                short_txt=short_txt,
                timestamp=self.last_msg_ts,
                num_fig=len(self.stored_figs),
                num_msgs=self.num_msgs,
            )

    def __init__(self, address=CPP2PY_ADDRESS, mode=CPP2PY_MODE_WIDGET):
        # the stored figs is a singleton
        if not hasattr(self.__class__, "stored_figs"):
            self.__class__.stored_figs = {}
        self.stored_figs = self.__class__.stored_figs

        if mode.startswith(CPP2PY_MODE_WIDGET):
            mode = CPP2PY_MODE_WIDGET
            self._initialise_as_ipywidgets()
            self.reciever = Cpp2PyReciever(address=address,
                                           ctx_mgr=self.ctx_mgr_info_label)
            self.goFigClass = go.FigureWidget
        elif mode == CPP2PY_MODE_DEFAULT:
            self.reciever = Cpp2PyReciever(address=address)
            self.ctx_mgr_chained = DummyCtxMgr
            self.goFigClass = go.Figure
        else:
            raise RuntimeError("Unrecognised mode {}".format(mode))
        self.mode = mode
        # reciever for msg from cpp side
        self.async_task = None

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
            description='Refresh',
            disabled=False,
            button_style='',  # 'success', 'info', 'warning', 'danger' or ''
            tooltip='Process plotly incoming messages',
            icon='retweet'
        )
        self.w_refresh_btn.on_click(lambda x: self.spin_once(verbose=False))
        self.w_info_label = ipywidgets.HTML(value="Started.")
        ##
        self.ctx_mgr_captured_log = ipywidgets.widgets.Output(
            layout={'border': '1px solid black'})
        self.ctx_mgr_info_label = self.__class__.InfoLabelCtxMgr(self.w_info_label,
                                                                 self.stored_figs)
        self.w_info_containers = ipywidgets.HBox(
            children=[
                self.w_refresh_btn,
                self.w_info_label,
            ]
        )
        ##
        from contextlib import contextmanager

        @contextmanager
        def ctx_mgr_chained():
            with self.ctx_mgr_captured_log:
                with self.ctx_mgr_info_label:
                    yield

        self.ctx_mgr_chained = ctx_mgr_chained

    @property
    def spinning_asyncly(self):
        if self.async_task is not None and not self.async_task.cancelled():
            return True
        return False

    def parse_msg_to_plotly_fig(self, msg):
        """Give a parsed msg (in terms of dict and friends), add a plotly figure."""
        uuid, fig_kwargs = msg
        plotly_fig_widget = self.goFigClass(
            go.Scatter(
                **fig_kwargs
            )
        )
        #                 self.update_figure_widget(plotly_fig_widget, uuid=uuid)
        self.add_figure_widget(plotly_fig_widget, uuid=uuid)
        if self.mode == CPP2PY_MODE_DEFAULT:
            plotly_fig_widget.show()

    def spin_once(self, verbose=False):
        """spin once to process all pending messsages"""
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
    def spin_async(self, display_log=True):
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
        if self.w_multi_fig_sel is not None:
            prev_sel = self.w_multi_fig_sel.value
            self.w_multi_fig_sel.options = self.stored_figs.keys()
            # self.multi_figs_selections.value = []
            self.w_multi_fig_sel.value = prev_sel
        if self.w_single_fig_sel is not None:
            prev_sel = self.w_single_fig_sel.value
            self.w_single_fig_sel.options = self.stored_figs.keys()
            # self.single_fig_selections.value = None
            self.w_single_fig_sel.value = prev_sel

    @ipywidget_mode(True)
    def remove_figure_widget(self, uuid):
        """Overwrite matching widget."""
        # remove selection options
        if uuid in self.stored_figs:
            wid = self.stored_figs[uuid]
            wid.close()

        self._update_selection()

    @ipywidget_mode(True)
    def add_figure_widget(self, widget, uuid='default'):
        """Overwrite any existing widget."""
        assert type(widget) is self.goFigClass, type(widget)

        if uuid in self.stored_figs:
            print("FIX THIS")
            return self.update_figure_widget(widget, uuid)

        # remove selection options
        self.remove_figure_widget(uuid)
        self.stored_figs[uuid] = widget

        self._update_selection()

    @ipywidget_mode(True)
    def update_figure_widget(self, widget, uuid='default'):
        assert type(widget) is go.FigureWidget, type(widget)
        """WARN: Assumes the line sequence are in the same order"""
        assert uuid in self.stored_figs
        assert len(self.stored_figs[uuid].data) == len(widget.data)

        def _update_attr(existing, new):
            for _attr in new:
                # actual update of existing plotly figure is slow.
                # so we will opt to only update attribute that are
                # different (with overhead of checking equality)
                # if type is np array, we don't bother to check for equality
                # nope. we will check shape and eq_val
                if type(new[_attr]) is np.ndarray and np.array_equal(
                        existing[_attr], new[_attr]):
                    continue
                elif type(new[_attr]) is dict:
                    _update_attr(existing[_attr], new[_attr])
                elif existing[_attr] == new[_attr]:
                    continue
                else:
                    existing[_attr] = new[_attr]

        for stored_seq, new_widget_seq in zip(self.stored_figs[uuid].data, widget.data):
            _update_attr(stored_seq, new_widget_seq)

        self._update_selection()

    ########################################
    ## multi-figs widget
    ########################################
    @ipywidget_mode()
    def display_ipywidget_multi_figs(self):
        self.w_multi_fig_sel = ipywidgets.widgets.SelectMultiple(
            options=[], value=[],
            description='Multi Fig(s)'
        )
        self.w_multi_fig_sel.options = self.stored_figs.keys()
        if self.mode == CPP2PY_MODE_WIDGET:
            display(self.w_info_containers)

        @ipywidgets.interact(widget_names=self.w_multi_fig_sel)
        def on_change(widget_names):
            return ipywidgets.HBox(
                children=[self.stored_figs[name] for name in widget_names])

    ########################################
    ## single-fig widget
    ########################################
    @ipywidget_mode()
    def display_ipywidget_single_fig(self):
        self.w_single_fig_sel = ipywidgets.widgets.Dropdown(
            options=[],
            description='Show Fig'
        )
        self.w_single_fig_sel.options = self.stored_figs.keys()
        if self.mode == CPP2PY_MODE_WIDGET:
            display(self.w_info_containers)

        @ipywidgets.interact(widget_names=self.w_single_fig_sel)
        def on_change(widget_names):
            if widget_names:
                return self.stored_figs[widget_names]

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
            options=[],
            value=[],
            description='Multi Fig(s)'
        )
        inner_figs_container = ipywidgets.HBox(children=[])
        inner_figs_container = ipywidgets.widgets.GridBox(
            children=[],
            layout=ipywidgets.Layout(grid_template_columns="repeat(2, 1fr)")
        )

        def on_change(event):
            inner_figs_container.children = [self.stored_figs[name] for name in
                                             event['new']]

        self.w_multi_fig_sel.observe(on_change, 'value')
        self.w_multi_fig_sel.options = self.stored_figs.keys()
        ## Outer widget
        widget = ipywidgets.VBox(
            children=[self.w_multi_fig_sel, inner_figs_container])
        display(widget)

    @ipywidget_mode()
    def OLD_get_ipywidget_single_fig(self):
        self.w_single_fig_sel = ipywidgets.widgets.Dropdown(
            options=[],
            description='Show Fig'
        )
        inner_figs_container = ipywidgets.HBox(children=[])

        def on_change(event):
            inner_figs_container.children = [self.stored_figs[event['new']]]

        self.w_single_fig_sel.observe(on_change, 'value')
        self.w_single_fig_sel.options = self.stored_figs.keys()
        ## Outer widget
        widget = ipywidgets.VBox(
            children=[self.w_single_fig_sel, inner_figs_container])
        display(widget)

# TODO: work on re-selecting previous figure after updating/creating figure(s)
