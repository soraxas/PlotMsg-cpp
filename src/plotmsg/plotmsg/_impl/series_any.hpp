#pragma once

#include "helpers.hpp"

namespace PlotMsg
{
    enum NullValueType
    {
        value
    };
    NullValueType NullValue = NullValueType::value;

    std::vector<SeriesAnyMsg_value> seriesAny_vector_create()
    {
        return std::vector<SeriesAnyMsg_value>();
    }

    void seriesAny_vector_push_back(std::vector<SeriesAnyMsg_value> &vec, NullValueType null);

    void seriesAny_vector_push_back(std::vector<SeriesAnyMsg_value> &vec, const std::string &val);

    void seriesAny_vector_push_back(std::vector<SeriesAnyMsg_value> &vec, double val);

    void seriesAny_vector_push_back(std::vector<SeriesAnyMsg_value> &vec, int val);

    //////////
    template <typename T, typename... Ts>
    void seriesAny_vector(std::vector<SeriesAnyMsg_value> &vec, T first, Ts... args)
    {
        // intermediate worker
        seriesAny_vector_push_back(vec, first);
        seriesAny_vector(vec, args...);
    }

    template <typename T>
    void seriesAny_vector(std::vector<SeriesAnyMsg_value> &vec, T first)
    {
        // last worker
        seriesAny_vector_push_back(vec, first);
    }

    template <typename... Ts>
    std::vector<SeriesAnyMsg_value> seriesAny_vector(Ts... args)
    {
        // entrypoint
        std::vector<SeriesAnyMsg_value> vec = seriesAny_vector_create();
        seriesAny_vector(vec, args...);
        return vec;
    }
}  // namespace PlotMsg