#pragma once

#include "plotmsg/main.hpp"

#include <cmath>

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
            return PlotMsg::Trace(                       //
                PlotlyTrace::graph_objects, "Scatter",   //
                PlotMsg::Dictionary(                     //
                    "mode", "markers",                   //
                    "marker_size", 10,                   //
                                                         // "marker_opacity", 0.9,
                    "marker_colorscale", "Viridis",      //
                    "marker_colorbar_title", "Colorbar"  //
                )                                        //
            );
        }

        template <typename T>
        PlotMsg::Trace scatter(const std::vector<T> &x, const std::vector<T> &y)
        {
            PlotMsg::Trace trace = scatter();
            trace["x"] = x;
            trace["y"] = y;
            return trace;
        }

        template <typename T>
        PlotMsg::Trace
        scatter(const std::vector<T> &x, const std::vector<T> &y, const std::vector<T> &z)
        {
            return PlotMsg::Trace(
                PlotlyTrace::graph_objects,  //
                "Scatter3d",                 //
                PlotMsg::Dictionary(         //
                    "x", x,                  //
                    "y", y,                  //
                    "z", z                   //
                )                            //
            );
        }

        template <size_t StateDimNum, typename T>
        PlotMsg::Trace scatter(const std::array<std::vector<T>, StateDimNum> &points_across_dim)
        {
            static_assert(StateDimNum == 2 || StateDimNum == 3, "Not supported");

            if (StateDimNum == 2)
                return scatter(points_across_dim[0], points_across_dim[1]);
            else if (StateDimNum == 3)
                return scatter(points_across_dim[0], points_across_dim[1], points_across_dim[2]);
        }

        template <typename T1, typename T2>
        PlotMsg::Trace
        scatter_with_colour(std::vector<T1> &x, std::vector<T1> &y, std::vector<T2> &c)
        {
            PlotMsg::Trace trace = scatter(x, y);
            trace["marker_color"] = c;
            return trace;
        }

        template <typename T>
        PlotMsg::Trace contour(
            std::vector<T> &x, std::vector<T> &y, std::vector<T> &c, bool label = false,
            bool continuous_coloring = false
        )
        {
            auto trace = PlotMsg::Trace(  //
                PlotlyTrace::graph_objects, "Contour",
                PlotMsg::Dictionary(  //
                    "x", x,           //
                    "y", y,           //
                    "z", c            //
                )
            );
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
            return PlotMsg::Trace(
                PlotlyTrace::graph_objects, "Heatmap",
                PlotMsg::Dictionary(  //
                    "x", x,           //
                    "y", y,           //
                    "z", c            //
                )
            );
        }

        /**
         * Plot the given list of edges
         * @tparam T data type of the container (should be able to infer this)
         * @param pair_of_edges_across_dim a list of d-dimensional pair of edges. E.g.,
         *        [x~[[1, 2], [3, 4]], y~[[5, 6], [7, 8]]] represents a 2D edge list with
         *        data point (1,5) connects to (2,6) and (3, 7) connects to (4,8)
         * @return a trace that contain the formatted edges
         */
        template <size_t StateDimNum, typename T>
        PlotMsg::Trace
        edges(const std::array<std::vector<std::pair<T, T>>, StateDimNum> &pair_of_edges_across_dim)
        {
            static_assert(StateDimNum == 2 || StateDimNum == 3, "Not supported");

            // x's size is used as the final size
            size_t _reference_size = pair_of_edges_across_dim[0].size();

            std::vector<std::vector<SeriesAnyMsg_value>> edge_series;
            std::vector<double> edge_weight;
            edge_series.resize(StateDimNum);
            for (int d = 0; d < StateDimNum; ++d)
            {
                edge_series[d].reserve(_reference_size);
            }

            for (uint i = 0; i < _reference_size; ++i)
            {
                for (int d = 0; d < StateDimNum; ++d)
                {
                    seriesAny_vector_push_back(
                        edge_series[d], pair_of_edges_across_dim[d][i].first
                    );
                    seriesAny_vector_push_back(
                        edge_series[d], pair_of_edges_across_dim[d][i].second
                    );
                    seriesAny_vector_push_back(edge_series[d], PlotMsg::NullValue);
                }
            }

            PlotMsg::Trace trace;
            if (StateDimNum == 2)
            {
                trace = scatter(edge_series[0], edge_series[1]);
            }
            else if (StateDimNum == 3)
            {
                trace = scatter(edge_series[0], edge_series[1], edge_series[2]);
            }
            trace.m_kwargs.update_kwargs(PlotMsg::Dictionary(  //
                "line_width", 0.5,                             //
                "line_color", "#888",                          //
                "hoverinfo", "none",                           //
                "mode", "lines+markers",                       //
                "marker_size", 1                               //
            ));
            return trace;
        }

        template <typename T>
        PlotMsg::Trace
        edges(const std::vector<std::pair<T, T>> &x, const std::vector<std::pair<T, T>> &y)
        {
            return edges<2, T>({x, y});
        }

        /**
         * Plot the given list of nodes
         * @tparam T data type of the container (should be able to infer this)
         * @param nodes_across_dim a list of d-dimensional pair of edges. E.g.,
         *        [x~[[1, 2], [3, 4]], y~[[5, 6], [7, 8]]] represents a 2D node list with
         *        data point (1,5) connects to (2,6) and (3, 7) connects to (4,8)
         * @return a trace that contain the formatted edges
         */
        template <size_t StateDimNum, typename T>
        PlotMsg::Trace vertices(const std::array<std::vector<T>, StateDimNum> &nodes_across_dim)
        {
            static_assert(StateDimNum == 2 || StateDimNum == 3, "Not supported");

            auto trace = scatter<StateDimNum, T>(nodes_across_dim);
            trace.m_kwargs.update_kwargs(                //
                PlotMsg::Dictionary(                     //
                    "mode", "markers",                   //
                    "hoverinfo", "text",                 //
                    "marker_showscale", true,            //
                    "marker_colorscale", "YlGnBu",       //
                    "marker_reversescale", true,         //
                    "marker_color", std::vector<int>(),  // why?
                    "marker_size", 10,                   //
                    "marker_line_width", 2               //
                )
            );
            return trace;
        }

        template <typename T>
        PlotMsg::Trace vertices(const std::vector<T> &x, const std::vector<T> &y)
        {
            return vertices<2, T>({x, y});
        }

        template <size_t StateDimNum, typename T1, typename T2>
        PlotMsg::Trace vertices_with_colour(
            const std::array<std::vector<T1>, StateDimNum> &nodes_across_dim,
            const std::vector<T2> &c
        )
        {
            auto trace = vertices<StateDimNum, T1>(nodes_across_dim);
            auto overriding_dict = PlotMsg::Dictionary(  //
                "marker_color", c,                       //
                "marker_colorbar_thickness", 15,         //
                "marker_colorbar_title", "Vertices",     //
                "marker_colorbar_xanchor", "left",       //
                "marker_colorbar_titleside", "right"     //

            );
            trace.m_kwargs.update_kwargs(overriding_dict);
            return trace;
        }

        //        template <typename T1, typename T2>
        //        PlotMsg::Trace vertices_with_colour(const std::vector<T1> &x, const
        //        std::vector<T1> &y,
        //                                            const std::vector<T2> &c)
        //        {
        //            return vertices_with_colour<2, T1, T2>({x, y}, c);
        //        }

        enum VectorFieldType
        {
            cone,
            line_segments
        };

        template <
            typename T, size_t StateDimNum, VectorFieldType vectorFieldType = VectorFieldType::cone>
        std::vector<PlotMsg::Trace> vector_field(
            std::array<std::vector<T>, StateDimNum> &origin,
            std::array<std::vector<T>, StateDimNum> &vectors, double scale = 1
        )
        {
            static_assert(StateDimNum == 2 || StateDimNum == 3, "Not supported");

            if (StateDimNum == 2)
            {
                return PlotMsg::Trace(
                    PlotlyTrace::figure_factory, "create_quiver",
                    PlotMsg::Dictionary(    //
                        "x", origin[0],     //
                        "y", origin[1],     //
                        "u", vectors[0],    //
                        "v", vectors[1],    //
                        "scale", .25,       //
                        "arrow_scale", .4,  //
                        "name", "quiver",   //
                        "line_width", 1     //
                    )
                );
            }
            else if (StateDimNum == 3)
            {
                switch (vectorFieldType)
                {
                    case VectorFieldType::cone:
                    {
                        return PlotMsg::Trace(
                            PlotlyTrace::graph_objects, "Cone",
                            PlotMsg::Dictionary(         //
                                "x", origin[0],          //
                                "y", origin[1],          //
                                "z", origin[2],          //
                                "u", vectors[0],         //
                                "v", vectors[1],         //
                                "w", vectors[2],         //
                                "sizemode", "absolute",  //
                                "sizeref", 2,            //
                                "name", "quiver"         //
                                //                        "line_width", 1          //
                            )
                        );
                    }

                    case VectorFieldType::line_segments:
                    {
                        return PlotMsg::Trace(
                            PlotMsg::PlotlyTrace::plotmsg_custom, "vector_field",
                            PlotMsg::Dictionary(  //
                                "x", origin[0],   //
                                "y", origin[1],   //
                                "z", origin[2],   //
                                "u", vectors[0],  //
                                "v", vectors[1],  //
                                "w", vectors[2],  //
                                "scale", scale
                            )
                        );
                    }
                        /*
                        {
                            auto bounds0 = get_bounding_element(origin[0]);
                            auto bounds1 = get_bounding_element(origin[1]);
                            auto bounds2 = get_bounding_element(origin[2]);

                            T max_bounds = std::max(
                                bounds0.second.second - bounds0.second.first,
                                bounds1.second.second - bounds1.second.first
                            );
                            max_bounds =
                                std::max(max_bounds, bounds2.second.second - bounds2.second.first);

                            //////////////
                            //                    double scale = std::pow(
                            // ((bounds0.second.second -
                            // bounds0.second.first) *
                            // (bounds1.second.second -
                            // bounds1.second.first) *
                            // (bounds2.second.second -
                            // bounds2.second.first)),
                            //                                       1 / 3
                            //                                   ) *
                            //                                   10;

                            size_t n_datapoints = origin[0].size();
                            // use edge to build a vector field
                            std::array<std::vector<std::pair<T, T>>, StateDimNum>
                                pair_of_edges_across_dim;

                            // stores the norms of the vector
                            std::vector<T> norms(n_datapoints);
                            // stores the directional unit vector
                            std::array<std::vector<T>, StateDimNum> directional_vector;
                            for (size_t d = 0; d < StateDimNum; ++d)
                            {
                                directional_vector[d].resize(n_datapoints);
                            }
                            //                    sizeref = 1;

                            for (size_t i = 0; i < n_datapoints; ++i)
                            {
                                norms[i] = 0;
                                for (size_t d = 0; d < StateDimNum; ++d)
                                {
                                    T endpoint = origin[d][i] + vectors[d][i] * scale;
                                    pair_of_edges_across_dim[d].emplace_back(
                                        origin[d][i], endpoint
                                    );

                                    directional_vector[d][i] = endpoint - origin[d][i];
                                    norms[i] += std::pow(vectors[d][i], 2);
                                }
                                norms[i] = std::sqrt(norms[i]);
                            }
                            auto line_trace = edges(pair_of_edges_across_dim);
                            line_trace.m_kwargs.update_kwargs(PlotMsg::Dictionary(
                                //
                                "line_width", 5,  // "line_color", norms,
                                // "mode", "lines", // "line_showscale",
                                true,                               //
                                "line_colorscale", "viridis",       //
                                "line_colorbar_thickness", 15,      //
                                "line_colorbar_title", "line",      //
                                "line_colorbar_xanchor", "left",    //
                                "line_colorbar_titleside", "right"  //
                            ));

                            for (size_t d = 0; d < StateDimNum; ++d)
                            {
                                for (size_t i = 0; i < n_datapoints; ++i)
                                {
                                    directional_vector[d][i] /= norms[i];
                                }
                            }

                            //                    auto vec =
                            PlotMsg::seriesAny_vector_create();
                            //
                            // PlotMsg::seriesAny_vector(vec, 0, "grey");

                            std::string color = "grey";
                            //                    auto colorscale_msg =
                            // PlotMsg::seriesAny_vector_create();
                            std::vector<std::vector<SeriesAnyMsg_value>> colorscale;
                            {
                                auto vec = PlotMsg::seriesAny_vector_create();
                                PlotMsg::seriesAny_vector_push_back(vec, 0);
                                PlotMsg::seriesAny_vector_push_back(vec, "grey");
                                colorscale.push_back(vec);
                            }
                            {
                                auto vec = PlotMsg::seriesAny_vector_create();
                                PlotMsg::seriesAny_vector_push_back(vec, 1);
                                PlotMsg::seriesAny_vector_push_back(vec, "grey");
                                colorscale.push_back(vec);
                            }

                            auto arrow_head_trace = PlotMsg::Trace(
                                PlotlyTrace::graph_objects, "Cone",
                                PlotMsg::Dictionary(             //
                                    "x", origin[0],              //
                                    "y", origin[1],              //
                                    "z", origin[2],              //
                                    "u", directional_vector[0],  //
                                    "v", directional_vector[1],  //
                                    "w", directional_vector[2],  //
                                    "anchor", "tip",             //
                                    "sizemode", "scaled",        //
                                    "sizeref", 2,                //
                                    // "colorscale", {{0, "grey"}, {1,
                                    // "grey"}}, "colorscale",
                                    // colorscale,

                                    //                            //
                                    "showscale", false  //
                                )
                            );

                            return {line_trace, arrow_head_trace};
                        }
                        */
                }
            }
        }

    }  // namespace TraceTemplate

    namespace FigureTemplate
    {

        void set_equal_axis(Figure &fig)
        {
            // set same scale for x and y axis
            fig.add_command(
                "update_yaxes",
                PlotMsg::Dictionary(     //
                    "scaleanchor", "x",  //
                    "scaleratio", 1      //
                )
            );
        }

        // the tuple is "stats name", "(x) vec of values' group", "(y) vec of values"
        template <
            class Numeric = double,
            typename = std::enable_if_t<std::is_arithmetic<Numeric>::value, Numeric>>
        using boxplot_datatype =
            std::tuple<std::string, std::vector<std::string>, std::vector<Numeric>>;

        template <typename T = double>
        void boxplot(Figure &fig, std::vector<boxplot_datatype<T>> data)
        {
            for (auto &&[plot_name, x_label, y_value] : data)
            {
                auto dict = PlotMsg::Trace(             //
                    PlotlyTrace::graph_objects, "Box",  //
                    PlotMsg::Dictionary(
                        "y", y_value,       // actual values
                        "name", plot_name,  // name of that boxplot
                        "notched", true     // add notch at median
                    )
                );
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
