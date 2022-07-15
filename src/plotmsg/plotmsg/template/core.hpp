#pragma once

#include "plotmsg/main.hpp"

/*
 * Implements tempalte message for easy usage
 */

namespace PlotMsg
{
    namespace TraceTemplate
    {
        using ditem = PlotMsg::Dictionary::DictionaryItemPair;

        PlotMsg::Trace scatter()
        {
            return PlotMsg::Trace(  //
                PlotlyTrace::graph_objects, "Scatter",
                PlotMsg::Dictionary(                     //
                    "mode", "markers",                   //
                    "marker_size", 10,                   //
                                                         //          "marker_opacity", 0.9,              //
                    "marker_colorscale", "Viridis",      //
                    "marker_colorbar_title", "Colorbar"  //
                    )                                    //
            );
        }

        template <typename T1, typename T2>
        PlotMsg::Trace scatter(std::vector<T1> &x, std::vector<T2> &y)
        {
            PlotMsg::Trace trace = scatter();
            trace["x"] = x;
            trace["y"] = y;
            return trace;
        }

        template <typename T1, typename T2>
        PlotMsg::Trace scatter_with_colour(std::vector<T1> &x, std::vector<T1> &y, std::vector<T2> &c)
        {
            PlotMsg::Trace trace = scatter(x, y);
            trace["marker_color"] = c;
            return trace;
        }

        template <typename T>
        PlotMsg::Trace vector_field(std::vector<T> &x, std::vector<T> &y, std::vector<T> &u, std::vector<T> &v)
        {
            return PlotMsg::Trace(PlotlyTrace::figure_factory, "create_quiver",
                                  PlotMsg::Dictionary(    //
                                      "x", x,             //
                                      "y", y,             //
                                      "u", u,             //
                                      "v", v,             //
                                      "scale", .25,       //
                                      "arrow_scale", .4,  //
                                      "name", "quiver",   //
                                      "line_width", 1     //
                                      ));
        }
        // alias of vector field
        template <typename... T>
        PlotMsg::Trace quiver(T... args)
        {
            return vector_field(args...);
        }

        template <typename T>
        PlotMsg::Trace contour(std::vector<T> &x, std::vector<T> &y, std::vector<T> &c, bool label = false,
                               bool continuous_coloring = false)
        {
            auto trace = PlotMsg::Trace(  //
                PlotlyTrace::graph_objects, "Contour",
                PlotMsg::Dictionary(  //
                    "x", x,           //
                    "y", y,           //
                    "z", c            //
                    ));
            if (continuous_coloring)
                trace["contours_coloring"] = "heatmap";  // can also be 'lines', or 'none'
            if (label)
            {
                trace["contours_showlabels"] = true;
                trace["contours_labelfont_size"] = 12;
                trace["contours_labelfont_color"] = "white";
            }
            return trace;
        }

        template <typename T>
        PlotMsg::Trace heatmap(std::vector<T> &x, std::vector<T> &y, std::vector<T> &c)
        {
            return PlotMsg::Trace(PlotlyTrace::graph_objects, "Heatmap",
                                  PlotMsg::Dictionary(  //
                                      "x", x,           //
                                      "y", y,           //
                                      "z", c            //
                                      ));
        }

        template <typename T>
        PlotMsg::Trace edges(std::vector<std::pair<T, T>> &x, std::vector<std::pair<T, T>> &y)
        {
            // x's size is used as the final size
            std::vector<SeriesAnyMsg_value> edge_x, edge_y;
            edge_x.reserve(x.size());
            edge_y.reserve(x.size());
            for (uint i = 0; i < x.size(); ++i)
            {
                seriesAny_vector_push_back(edge_x, x[i].first);
                seriesAny_vector_push_back(edge_x, x[i].second);
                seriesAny_vector_push_back(edge_x, PlotMsg::NullValue);
                seriesAny_vector_push_back(edge_y, y[i].first);
                seriesAny_vector_push_back(edge_y, y[i].second);
                seriesAny_vector_push_back(edge_y, PlotMsg::NullValue);
            }
            return PlotMsg::Trace(PlotlyTrace::graph_objects, "Scatter",
                                  PlotMsg::Dictionary(       //
                                      "x", edge_x,           //
                                      "y", edge_y,           //
                                      "line_width", 0.5,     //
                                      "line_color", "#888",  //
                                      "hoverinfo", "none",   //
                                      "mode", "lines"        //
                                      ));
        }

        template <typename T>
        PlotMsg::Trace vertices(std::vector<T> &x, std::vector<T> &y)
        {
            return PlotMsg::Trace(PlotlyTrace::graph_objects, "Scatter",
                                  PlotMsg::Dictionary(                     //
                                      "x", x,                              //
                                      "y", y,                              //
                                      "mode", "markers",                   //
                                      "hoverinfo", "text",                 //
                                      "marker_showscale", true,            //
                                      "marker_colorscale", "YlGnBu",       //
                                      "marker_reversescale", true,         //
                                      "marker_color", std::vector<int>(),  // why?
                                      "marker_size", 10,                   //
                                      "marker_line_width", 2               //
                                      ));
        }

        template <typename T1, typename T2>
        PlotMsg::Trace vertices(std::vector<T1> &x, std::vector<T1> &y, std::vector<T2> &c)
        {
            auto trace = vertices(x, y);
            trace["marker_color"] = c;
            trace["marker"]["colorbar"]["thickness"] = 15;
            trace["marker"]["colorbar"]["title"] = "Node Connections";
            trace["marker"]["colorbar"]["xanchor"] = "left";
            trace["marker"]["colorbar"]["titleside"] = "right";
            return trace;
        }

    }  // namespace TraceTemplate

    namespace FigureTemplate
    {

        void set_equal_axis(Figure &fig)
        {
            // set same scale for x and y axis
            fig.add_command("update_yaxes",
                            PlotMsg::Dictionary(     //
                                "scaleanchor", "x",  //
                                "scaleratio", 1      //
                                ));
        }

        // the tuple is "stats name", "(x) vec of values' group", "(y) vec of values"
        template <class Numeric = double, typename = std::enable_if_t<std::is_arithmetic<Numeric>::value, Numeric>>
        using boxplot_datatype = std::tuple<std::string, std::vector<std::string>, std::vector<Numeric>>;

        template <typename T = double>
        void boxplot(Figure &fig, std::vector<boxplot_datatype<T>> data)
        {
            for (auto &&[plot_name, x_label, y_value] : data)
            {
                auto dict = PlotMsg::Trace(                 //
                    PlotlyTrace::graph_objects, "Box",      //
                    PlotMsg::Dictionary("y", y_value,       // actual values
                                        "name", plot_name,  // name of that boxplot
                                        "notched", true     // add notch at median
                                        ));
                // allows the caller to omit x_label by passing an empty vector
                if (!x_label.empty())
                    dict["x"] = x_label;
                fig.add_trace(dict);
            }
            // default to group box plot and display points alongside the bars
            fig.add_command("update_traces", Dictionary("boxpoints", "all"));
            fig.add_command("update_layout", Dictionary("boxmode", "group"));
        }

        // template <typename T1>
        // void __boxplot(Figure &fig, const std::string &plot_name,
        //               std::vector<std::string> &x_label, std::vector<T1> &y_value) {
        //  auto dict = PlotMsg::Trace(                  //
        //      PlotlyTrace::graph_objects, "Box", //
        //      PlotMsg::Dictionary("y", y_value,        // actual values
        //                         "name", plot_name,   // name of that boxplot
        //                         "notched", true      // add notch at median
        //                         ));
        //  // allows the caller to omit x_label by passing an empty vector
        //  if (!x_label.empty())
        //    dict["x"] = x_label;
        //
        //  fig.add_trace(dict);
        //}
        //
        // template <typename T1, typename... Ts>
        // void __boxplot(Figure &fig, const std::string &plot_name,
        //               std::vector<std::string> &x_label, std::vector<T1> &y_value,
        //               Ts... rest) {
        //  __boxplot(fig, plot_name, x_label, y_value); // call base
        //  __boxplot(fig, rest...);                     // recursive call
        //}
        //
        // template <typename... Ts> void boxplot(Figure &fig, Ts... rest) {
        //  /* This is the entrance point */
        //  __boxplot(fig, rest...);
        //
        //}

    }  // namespace FigureTemplate

}  // namespace PlotMsg
