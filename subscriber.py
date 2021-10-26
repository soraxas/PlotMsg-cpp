import sys

sys.path.insert(0, "built_python_pkg")


from plotmsg_dash import PlotMsgPlotly

p_msg = PlotMsgPlotly()
p_msg.spin_once()
