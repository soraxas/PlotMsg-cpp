#pragma once

#include "plotmsg/template/core.hpp"
#include <ompl/base/PlannerData.h>
#include <ompl/geometric/PathGeometric.h>

namespace PlotMsg
{
    namespace OmplTemplate
    {

        namespace ob = ompl::base;
        namespace og = ompl::geometric;

        template <size_t StateDimNum, typename T = double>
        struct StateFormatter
        {
            virtual std::array<T, StateDimNum> getCoordinate(const ob::State *state) const = 0;

            virtual T getColour(const ob::State *state) const
            {
                return 0;
            }

            virtual bool hasColour() const
            {
                return false;
            }

            virtual bool hasEdgeColour() const
            {
                return false;
            }

            virtual T getEdgeColour(const ob::State *state1, const ob::State *state2) const
            {
                return 0;
            }
        };

        template <size_t StateDimNum, typename T = double>
        struct StateFormatterWithColour : public StateFormatter<StateDimNum, T>
        {
            virtual bool hasColour() const override
            {
                return true;
            }

            virtual T getColour(const ob::State *state) const override = 0;
        };

        template <size_t StateDimNum, typename T = double>
        using StateTransformationFunc_t =
            std::function<std::array<T, StateDimNum>(const ob::State *state)>;

        template <size_t StateDimNum, typename T = double>
        using StateTransformationFuncWithColour_t =
            std::function<std::pair<std::array<T, StateDimNum>, T>(const ob::State *state)>;

        /**
         * A template state transformation function that works for states that are RealVector-like.
         * i.e. if the state has a member double ptr named as 'values', which stores coordinates.
         *
         * @tparam StateType The concrete ompl state type
         * @tparam StateDimNum The number of state dimensions of the given state
         * @tparam T The data type of the stored state coordinate
         * @param state The state to be transformed
         * @return A standard array with size 'StateDimNum', which stores the transformed coordinate
         */
        template <typename StateType, size_t StateDimNum, typename T = double>
        std::array<T, StateDimNum> RealVectorLikeStateTransformationFunc(const ob::State *state)
        {
            std::array<T, StateDimNum> container;
            double *values = state->as<StateType>()->values;
            for (int i = 0; i < StateDimNum; ++i)
                container[i] = values[i];
            return std::move(container);
        }

        /*
         * Assume the given path have a 2/3-dimensional state
         *
         * ob::PlannerData pdata(si);
         * optimizingPlanner->getPlannerData(pdata);
         *
         * PlotMsg::Figure fig;
         * PlotMsg::OmplTemplate::plot_planner_data_graph<ob::RealVectorStateSpace::StateType,
         * 3>(fig, pdata);
         *
         */
        template <size_t StateDimNum, typename T = double>
        void plot_planner_data_graph(
            PlotMsg::Figure &fig, const ob::PlannerData &data,
            StateTransformationFunc_t<StateDimNum, T> transformation_func
        )
        {
            bool plot_edge_color = false;
            std::vector<double> edge_color;
            std::shared_ptr<ompl::base::Cost> weight;
            if (data.hasControls())
            {
                // add edge color
                plot_edge_color = true;
                edge_color.reserve(data.numEdges() * 3);

                weight = std::make_shared<ompl::base::Cost>();
            }

            // plot all edges
            std::array<std::vector<std::pair<double, double>>, StateDimNum> all_edges;

            std::vector<unsigned int> edgesList;
            for (uint i = 0; i < data.numVertices(); ++i)
            {
                const auto &v1 = data.getVertex(i);
                std::array<T, StateDimNum> v1_pos = transformation_func(v1.getState());
                data.getEdges(i, edgesList);
                for (auto &&j : edgesList)
                {
                    const auto &v2 = data.getVertex(j);
                    auto v2_pos = transformation_func(v2.getState());
                    for (int d = 0; d < StateDimNum; ++d)
                    {
                        all_edges[d].emplace_back(v1_pos[d], v2_pos[d]);
                    }
                    if (plot_edge_color)
                    {
                        bool ok = data.getEdgeWeight(i, j, weight.get());
                        assert(ok);
                        for (short i = 0; i < 3; ++i)
                            edge_color.push_back(weight->value());
                    }
                }
            }
            fig.add_trace(PlotMsg::TraceTemplate::edges<StateDimNum>(all_edges));
            fig.get_trace(-1)["name"] = "graph";
            if (plot_edge_color)
            {
                fig.get_trace(-1)["line_color"] = edge_color;
                fig.get_trace(-1)["line_showscale"] = true;
                fig.get_trace(-1)["line_width"] = 5;
            }

            // plot start/target
            std::array<std::vector<double>, StateDimNum> xs_across_dim;
            std::vector<std::string> cs;

            for (uint i = 0; i < data.numStartVertices(); ++i)
            {
                auto v_pos = transformation_func(data.getStartVertex(i).getState());
                for (int d = 0; d < StateDimNum; ++d)
                {
                    xs_across_dim[d].push_back(v_pos[d]);
                }
                cs.emplace_back("green");
            }
            for (uint i = 0; i < data.numGoalVertices(); ++i)
            {
                auto v_pos = transformation_func(data.getGoalVertex(i).getState());
                for (int d = 0; d < StateDimNum; ++d)
                {
                    xs_across_dim[d].push_back(v_pos[d]);
                }
                cs.emplace_back("red");
            }
            fig.add_trace(
                PlotMsg::TraceTemplate::vertices_with_colour<StateDimNum, double, std::string>(
                    xs_across_dim, cs
                )
            );
            fig.get_trace(-1)["showlegend"] = false;
            fig.get_trace(-1)["marker_showscale"] = false;
        }

        template <size_t StateDimNum, typename T, typename StateFormatterType>
        void plot_planner_data_graph_with_colour(
            PlotMsg::Figure &fig, const ob::PlannerData &data, const StateFormatterType &formatter
        )
        {
            //            static_assert(
            //                std::is_base_of_v<StateFormatterType, StateFormatter<StateDimNum, T>>,
            //                // "Incorrect Type"
            //            );

            std::vector<double> edge_color;

            // plot all edges
            std::array<std::vector<std::pair<double, double>>, StateDimNum> all_edges;

            std::vector<unsigned int> edgesList;
            std::vector<T> verticesColour;
            for (uint i = 0; i < data.numVertices(); ++i)
            {
                const auto &v1 = data.getVertex(i);
                auto v1_pos = formatter.getCoordinate(v1.getState());
                data.getEdges(i, edgesList);
                for (auto &&j : edgesList)
                {
                    const auto &v2 = data.getVertex(j);
                    auto v2_pos = formatter.getCoordinate(v2.getState());
                    for (int d = 0; d < StateDimNum; ++d)
                    {
                        all_edges[d].emplace_back(v1_pos[d], v2_pos[d]);
                    }

                    if (formatter.hasColour() || formatter.hasEdgeColour())
                    {
                        T colour1, colour2;
                        if (formatter.hasEdgeColour())
                        {
                            colour1 = formatter.getEdgeColour(v1.getState(), v2.getState());
                            colour2 = colour1;
                        }
                        else
                        {
                            colour1 = formatter.getColour(v1.getState());
                            colour2 = formatter.getColour(v2.getState());
                        }
                        edge_color.push_back(colour1);
                        edge_color.push_back(colour2);
                        edge_color.push_back(colour2);
                    }
                }
            }
            fig.add_trace(PlotMsg::TraceTemplate::edges<StateDimNum>(all_edges));
            fig.get_trace(-1)["name"] = "graph";
            if (formatter.hasColour())
            {
                fig.get_trace(-1)["line_color"] = edge_color;
            }
            fig.get_trace(-1)["line_showscale"] = true;
            fig.get_trace(-1)["line_width"] = 5;

            // plot start/target
            std::array<std::vector<double>, StateDimNum> xs_across_dim;
            std::vector<std::string> cs;

            for (uint i = 0; i < data.numStartVertices(); ++i)
            {
                auto v_pos = formatter.getCoordinate(data.getStartVertex(i).getState());
                for (int d = 0; d < StateDimNum; ++d)
                {
                    xs_across_dim[d].push_back(v_pos[d]);
                }
                cs.emplace_back("green");
            }
            for (uint i = 0; i < data.numGoalVertices(); ++i)
            {
                auto v_pos = formatter.getCoordinate(data.getGoalVertex(i).getState());
                for (int d = 0; d < StateDimNum; ++d)
                {
                    xs_across_dim[d].push_back(v_pos[d]);
                }
                cs.emplace_back("red");
            }
            fig.add_trace(
                PlotMsg::TraceTemplate::vertices_with_colour<StateDimNum, double, std::string>(
                    xs_across_dim, cs
                )
            );
            fig.get_trace(-1)["showlegend"] = false;
            fig.get_trace(-1)["marker_showscale"] = false;
        }

        /**
         *  using default real-vector-like state transformation function.
         *
         *  It can be used as follows.
         *  PlotMsg::OmplTemplate::plot_planner_data_graph<ob::OceanographyStateSpace::StateType, 3>
         *    (fig, pdata);
         *
         * @tparam StateType
         * @tparam StateDimNum
         * @tparam T
         * @param fig
         * @param data
         */
        //        template <typename StateType, size_t StateDimNum, typename T = double>
        //        void plot_planner_data_graph(PlotMsg::Figure &fig, const ob::PlannerData &data)
        //        {
        //            StateTransformationFunc_t<StateDimNum, T> func =
        //                RealVectorLikeStateTransformationFunc<StateType, StateDimNum, T>;
        //            plot_planner_data_graph(fig, data, func);
        //        }

        /*
         * Assume the given path have a 2-dimensional state
         * */
        template <size_t StateDimNum, typename T, typename StateFormatterType>
        void plot_path(
            PlotMsg::Figure &fig, const og::PathGeometric &path,
            const StateFormatterType &formatter, std::string colour = "blue",
            std::string name = "solution"
        )
        {
            std::array<std::vector<T>, StateDimNum> xs_across_dim;
            std::vector<std::string> cs;

            for (uint i = 0; i < path.getStateCount(); ++i)
            {
                std::array<T, StateDimNum> pos = formatter.getCoordinate(path.getState(i));
                for (int d = 0; d < StateDimNum; ++d)
                {
                    xs_across_dim[d].push_back(pos[d]);
                }
            }
            auto trace = PlotMsg::TraceTemplate::scatter<StateDimNum>(xs_across_dim);
            trace["name"] = name;
            trace["mode"] = "lines";
            trace["line_width"] = 1.5;
            trace["line_color"] = colour;

            fig.add_trace(trace);
        }

        /**
         * using default real-vector-like state transformation function
         *
         * It can be used as follows.
         * PlotMsg::OmplTemplate::plot_path<ob::OceanographyStateSpace::StateType, 3>(fig,
         * *sol_path);
         *
         * @tparam StateDimNum
         * @tparam T
         * @param fig
         * @param path
         * @param transformation_func
         * @param colour
         * @param name
         */
        template <typename StateType, size_t StateDimNum, typename T = double>
        void plot_path(
            PlotMsg::Figure &fig, const og::PathGeometric &path, std::string colour = "blue",
            std::string name = "solution"
        )
        {
            StateTransformationFunc_t<StateDimNum, T> func =
                RealVectorLikeStateTransformationFunc<StateType, StateDimNum, T>;
            plot_path(fig, path, func, colour, name);
        }

    }  // namespace OmplTemplate
}  // namespace PlotMsg