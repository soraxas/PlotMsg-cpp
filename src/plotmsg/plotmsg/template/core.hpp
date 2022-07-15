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
        /**
         * Plot the given list of edges
         * @tparam T data type of the container (should be able to infer this)
         * @param pair_of_edges_across_dim a list of d-dimensional pair of edges. E.g.,
         *        [x~[[1, 2], [3, 4]], y~[[5, 6], [7, 8]]] represents a 2D edge list with
         *        data point (1,5) connects to (2,6) and (3, 7) connects to (4,8)
         * @return a trace that contain the formatted edges
         */
        template <int StateDimNum, typename T>
        PlotMsg::Trace edges(const std::vector<std::vector<std::pair<T, T>>> &pair_of_edges_across_dim)
        {
            static_assert(StateDimNum == 2 || StateDimNum == 3, "Not supported");

            // x's size is used as the final size
            size_t _reference_size = pair_of_edges_across_dim[0].size();

            std::vector<std::vector<SeriesAnyMsg_value>> edge_series;
            edge_series.resize(StateDimNum);
            for (int d = 0; d < StateDimNum; ++d)
            {
                edge_series[d].reserve(_reference_size);
            }

            for (uint i = 0; i < _reference_size; ++i)
            {
                for (int d = 0; d < StateDimNum; ++d)
                {
                    seriesAny_vector_push_back(edge_series[d], pair_of_edges_across_dim[d][i].first);
                    seriesAny_vector_push_back(edge_series[d], pair_of_edges_across_dim[d][i].second);
                    seriesAny_vector_push_back(edge_series[d], PlotMsg::NullValue);
                }
            }
            auto dict = PlotMsg::Dictionary(  //
                "line_width", 0.5,            //
                "line_color", "#888",         //
                "hoverinfo", "none",          //
                "mode", "lines"               //
            );
            if (StateDimNum == 2)
            {
                dict["x"] = edge_series[0];
                dict["y"] = edge_series[1];
            }
            else if (StateDimNum == 3)
            {
                dict["x"] = edge_series[0];
                dict["y"] = edge_series[1];
                dict["z"] = edge_series[2];
            }

            return PlotMsg::Trace(PlotlyTrace::graph_objects, "Scatter", dict);
        }

        template <typename T>
        PlotMsg::Trace edges(const std::vector<std::pair<T, T>> &x, const std::vector<std::pair<T, T>> &y)
        {
            return edges<2, T>({x, y});
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
