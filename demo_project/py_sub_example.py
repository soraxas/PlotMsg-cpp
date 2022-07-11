import sys; sys.path.insert(0, "built_python_pkg")

from plotmsg_dash import PlotMsgPlotly

sub = PlotMsgPlotly()
sub.initialise()
sub.spin()
