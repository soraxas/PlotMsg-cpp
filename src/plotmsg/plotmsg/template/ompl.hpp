#pragma once

#include <ompl/base/PlannerData.h>

#include "plotmsg/template/core.hpp"

namespace PlotMsg
{
    namespace OmplTemplate
    {

        namespace ob = ompl::base;
        namespace og = ompl::geometric;

        /*
         * Assume the given path have a 2/3-dimensional state
         *
         * ob::PlannerData pdata(si);
         * optimizingPlanner->getPlannerData(pdata);
         *
         * PlotMsg::Figure fig;
         * PlotMsg::OmplTemplate::plot_planner_data_graph<ob::RealVectorStateSpace::StateType, 3>(fig, pdata);
         *
         */
        template <typename StateType, int StateDimNum>
        void plot_planner_data_graph(PlotMsg::Figure &fig, const ob::PlannerData &data)
        {
            // plot all edges
            std::vector<std::vector<std::pair<double, double>>> all_edges;
            all_edges.resize(StateDimNum);

            std::vector<unsigned int> edgesList;
            for (uint i = 0; i < data.numVertices(); ++i)
            {
                const auto &v1 = data.getVertex(i);
                auto v1_pos = v1.getState()->as<StateType>()->values;
                data.getEdges(i, edgesList);
                for (auto &&j : edgesList)
                {
                    const auto &v2 = data.getVertex(j);
                    auto v2_pos = v2.getState()->as<StateType>()->values;
                    for (int d = 0; d < StateDimNum; ++d)
                    {
                        all_edges[d].emplace_back(v1_pos[d], v2_pos[d]);
                    }
                }
            }
            fig.add_trace(PlotMsg::TraceTemplate::edges<StateDimNum>(all_edges));
            fig.get_trace(-1)["name"] = "graph";

            // plot start/target
            std::vector<std::vector<double>> xs_across_dim;
            xs_across_dim.resize(StateDimNum);
            std::vector<std::string> cs;

            for (uint i = 0; i < data.numStartVertices(); ++i)
            {
                auto v_pos = data.getStartVertex(i).getState()->as<StateType>()->values;
                for (int d = 0; d < StateDimNum; ++d)
                {
                    xs_across_dim[d].push_back(v_pos[d]);
                }
                cs.emplace_back("green");
            }
            for (uint i = 0; i < data.numGoalVertices(); ++i)
            {
                auto v_pos = data.getGoalVertex(i).getState()->as<StateType>()->values;
                for (int d = 0; d < StateDimNum; ++d)
                {
                    xs_across_dim[d].push_back(v_pos[d]);
                }
                cs.emplace_back("red");
            }
            fig.add_trace(
                PlotMsg::TraceTemplate::vertices_with_colour<StateDimNum, double, std::string>(xs_across_dim, cs));
            fig.get_trace(-1)["showlegend"] = false;
        }

        /*
         * Assume the given path have a 2-dimensional state
         * */
        template <typename StateType, int StateDimNum>
        void plot_path(PlotMsg::Figure &fig, const og::PathGeometric &path, std::string colour = "blue",
                       std::string name = "solution")
        {
            std::vector<std::vector<double>> xs_across_dim;
            xs_across_dim.resize(StateDimNum);
            std::vector<std::string> cs;

            for (uint i = 0; i < path.getStateCount(); ++i)
            {
                auto pos = path.getState(i)->as<StateType>()->values;
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

    }  // namespace OmplTemplate
}  // namespace PlotMsg