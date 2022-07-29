import sys

sys.path.insert(0, "built_python_pkg")


from plotmsg_dash import PlotMsgPlotly

p_msg = PlotMsgPlotly()

while True:
    p_msg.spin_once()
    for k in p_msg.figs:
        print(f"Displaying figure {k}...")
        p_msg.figs.pop(k).show()
