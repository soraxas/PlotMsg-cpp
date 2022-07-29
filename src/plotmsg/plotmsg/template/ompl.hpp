#pragma once

#include "plotmsg/template/core.hpp"
#include <ompl/base/PlannerData.h>

namespace PlotMsg
{
    namespace OmplTemplate
    {

        namespace ob = ompl::base;
        namespace og = ompl::geometric;

        template <size_t StateDimNum, typename T = double>
        using StateTransformationFunc_t =
            std::function<std::array<T, StateDimNum>(const ob::State *state)>;

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
                }
            }
            fig.add_trace(PlotMsg::TraceTemplate::edges<StateDimNum>(all_edges));
            fig.get_trace(-1)["name"] = "graph";

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
        template <typename StateType, size_t StateDimNum, typename T = double>
        void plot_planner_data_graph(PlotMsg::Figure &fig, const ob::PlannerData &data)
        {
            StateTransformationFunc_t<StateDimNum, T> func =
                RealVectorLikeStateTransformationFunc<StateType, StateDimNum, T>;
            plot_planner_data_graph(fig, data, func);
        }

        /*
         * Assume the given path have a 2-dimensional state
         * */
        template <size_t StateDimNum, typename T = double>
        void plot_path(
            PlotMsg::Figure &fig, const og::PathGeometric &path,
            StateTransformationFunc_t<StateDimNum, T> transformation_func,
            std::string colour = "blue", std::string name = "solution"
        )
        {
            std::array<std::vector<T>, StateDimNum> xs_across_dim;
            std::vector<std::string> cs;

            for (uint i = 0; i < path.getStateCount(); ++i)
            {
                std::array<T, StateDimNum> pos = transformation_func(path.getState(i));
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