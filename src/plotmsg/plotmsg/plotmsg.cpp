
#include "plotmsg/main.hpp"

namespace PlotMsg
{

    void initialise_publisher(int sleep_after_bind, const std::string &addr)
    {
        if (static_publisher != nullptr)
            return;
        static_context = std::make_unique<zmq::context_t>();
        static_publisher = std::make_unique<zmq::socket_t>(*static_context, ZMQ_PUB);
        static_publisher->bind(addr);
        if (sleep_after_bind > 0)
            std::this_thread::sleep_for(std::chrono::milliseconds(sleep_after_bind));
    }

    ////////////////////////////////////////
    // implementation of Dictionary
    ////////////////////////////////////////

    void _deep_copy_helper(const DictionaryMsgData &ori_dict, DictionaryMsgData &new_dict)
    {
        /*
         * Copy contents from ori_dict to new_dict
         */
        for (auto &&it = ori_dict.begin(); it != ori_dict.end(); ++it)
        {
            auto key = it->first;
            auto itemVal = it->second;

            if (itemVal.value_case() == DictItemValMsg::kSeriesD)
            {
                // TODO
                auto *series_d = new SeriesDMsg();
                series_d->ParseFromString(itemVal.series_d().SerializeAsString());
                new_dict[key].set_allocated_series_d(series_d);
            }
            else if (itemVal.value_case() == DictItemValMsg::kSeriesI)
            {
                // TODO
                auto *series_i = new SeriesIMsg();
                series_i->ParseFromString(itemVal.series_i().SerializeAsString());
                new_dict[key].set_allocated_series_i(series_i);
                itemVal.series_i().New(itemVal.GetArena());
            }
            else if (itemVal.value_case() == DictItemValMsg::kBool)
            {
                _set_DictItemVal(new_dict[key], itemVal.bool_());
            }
            else if (itemVal.value_case() == DictItemValMsg::kDict)
            {
                auto *nested_dict = new DictionaryMsg();
                _deep_copy_helper(itemVal.dict().data(), (*(*nested_dict).mutable_data()));
                new_dict[key].set_allocated_dict(nested_dict);
            }
            else if (itemVal.value_case() == DictItemValMsg::kDouble)
            {
                _set_DictItemVal(new_dict[key], itemVal.double_());
            }
            else if (itemVal.value_case() == DictItemValMsg::kString)
            {
                std::string new_str = itemVal.string();
                _set_DictItemVal(new_dict[key], new_str);
            }
            else
            {
                throw std::runtime_error("Unimplemented DictItemValMsg " +
                                         std::to_string(static_cast<int>(itemVal.value_case())));
            };
        }
    }

    ////////////////////////////////////////
    // implementation of PlotMsg Figure
    ////////////////////////////////////////

    void Figure::send(zmq::send_flags send_flags)
    {
        initialise_publisher();
        // swap kwargs in the dictionary container with the protobuf internal msg
        auto _fig = m_msg.mutable_fig();
        _fig->set_uuid(m_uuid);

        for (uint i = 0; i < size(); ++i)
        {
            auto trace = _fig->add_traces();
            trace->mutable_kwargs()->Swap(m_traces[i].m_kwargs.release_ptr());
            trace->set_method(m_traces[i].m_method);
            trace->set_method_func(m_traces[i].m_method_func);
        }

        std::string encoded_msg;
        m_msg.SerializeToString(&encoded_msg);
        zmq::message_t zmq_msg(encoded_msg.size());
        memcpy((void *)zmq_msg.data(), encoded_msg.c_str(), encoded_msg.size());
        static_publisher->send(zmq_msg, send_flags);

        reset();
    }

    void Figure::reset()
    {
        m_msg.Clear();
        m_traces.clear();
        m_msg.set_allocated_fig(new PlotlyFigureMsg());
        set_uuid(m_uuid);
    }
    zmq::message_t Figure::build_zmq_msg()
    {
        // serial protobuf to string
        std::string encoded_msg;

        std::cout << "|>" << m_msg.mutable_fig()->mutable_traces()->Get(0).kwargs().data() << std::endl;
        m_msg.SerializeToString(&encoded_msg);
        // memcpy to a zmq m_msg
        std::cout << "size   " << encoded_msg.size() << std::endl;
        zmq::message_t zmq_msg(encoded_msg.size());
        memcpy((void *)zmq_msg.data(), encoded_msg.c_str(), encoded_msg.size());
        return zmq_msg;
    }
    std::ostream &operator<<(std::ostream &out, Figure const &fig)
    {
        out << "Figure<" << fig.m_uuid << "|";
        for (auto &&trace : fig.m_traces)
        {
            out << trace;
        }
        out << ">";
        return out;
    }

    void Figure::set_trace_kwargs(uint idx, Dictionary &value)
    {
        if (idx >= size())
            throw std::out_of_range("Given index exceed the number of traces.");
        m_traces[idx].m_kwargs.set_kwargs(value);
    }

    /*
    void Figure::add_trace(Dictionary &value) {
      // auto add trace if it's just one less than what we had
      _add_trace();
      m_traces[size() - 1].m_kwargs.set_kwargs(value);
    }
    void Figure::add_trace(Dictionary &&value) {
      Dictionary new_dict(std::forward<Dictionary>(value));
      add_trace(new_dict);
    }
    */

    int Figure::_add_trace()
    {
        m_traces.emplace_back();
        return size();
    }

    Trace &Figure::trace(int idx)
    {
        // return a reference to a trace
        if (idx < 0)
        {
            // negative indexing, wraps around.
            idx = size() + idx;
            if (idx < 0)
                throw std::out_of_range("Given negative index exceed the number of traces.");
        }
        if (static_cast<uint>(idx) >= size())
        {
            throw std::out_of_range("Given index exceed the number of traces.");
        }
        return m_traces[idx];
    }

    void Figure::add_command(const std::string &func, Dictionary &value)
    {
        auto cmd = m_msg.mutable_fig()->add_commands();
        cmd->set_func(func);
        cmd->mutable_kwargs()->Swap(value.release_ptr());
    }

    void Figure::add_command(const std::string &func, Dictionary &&value)
    {
        Dictionary lvalue(value);
        add_command(func, lvalue);
    }

    ////////////////////////////////////////
    // Helpers
    ////////////////////////////////////////
    SeriesDMsg *vec_to_allocated_seriesD(std::vector<double> value)
    {
        google::protobuf::RepeatedField<double> data(value.begin(), value.end());
        auto *series = new SeriesDMsg();
        series->mutable_data()->Swap(&data);
        return series;
    }

    SeriesIMsg *vec_to_allocated_seriesI(std::vector<int> value)
    {
        google::protobuf::RepeatedField<int> data(value.begin(), value.end());
        auto *series = new SeriesIMsg();
        series->mutable_data()->Swap(&data);
        return series;
    }

    SeriesStringMsg *vec_to_allocated_seriesString(std::vector<std::string> value)
    {
        google::protobuf::RepeatedPtrField<std::string> data(value.begin(), value.end());
        auto *series = new SeriesStringMsg();
        series->mutable_data()->Swap(&data);
        return series;
    }

    SeriesAnyMsg *vec_to_allocated_seriesAny(std::vector<SeriesAnyMsg_value> value)
    {
        google::protobuf::RepeatedPtrField<SeriesAnyMsg_value> data(value.begin(), value.end());
        auto *series = new SeriesAnyMsg();
        series->mutable_data()->Swap(&data);
        return series;
    }

    void _set_DictItemVal(DictItemValMsg &item_val, bool value)
    {
        item_val.set_bool_(value);
    }

    void _set_DictItemVal(DictItemValMsg &item_val, double value)
    {
        item_val.set_double_(value);
    }

    void _set_DictItemVal(DictItemValMsg &item_val, int value)
    {
        item_val.set_int_(value);
    }

    void _set_DictItemVal(DictItemValMsg &item_val, const std::vector<double> &value)
    {
        item_val.set_allocated_series_d(vec_to_allocated_seriesD(value));
    }

    void _set_DictItemVal(DictItemValMsg &item_val, const std::vector<int> &value)
    {
        item_val.set_allocated_series_i(vec_to_allocated_seriesI(value));
    }

    void _set_DictItemVal(DictItemValMsg &item_val, const std::vector<std::string> &value)
    {
        item_val.set_allocated_series_string(vec_to_allocated_seriesString(value));
    }

    void _set_DictItemVal(DictItemValMsg &item_val, const std::vector<SeriesAnyMsg_value> &value)
    {
        item_val.set_allocated_series_any(vec_to_allocated_seriesAny(value));
    }

    void _set_DictItemVal(DictItemValMsg &item_val, PlotMsg::Dictionary &value)
    {
        item_val.set_allocated_dict(value.release_ptr());
    }

    // r-value, uses l-value definition
    void _set_DictItemVal(DictItemValMsg &item_val, PlotMsg::Dictionary &&value)
    {
        _set_DictItemVal(item_val, value);
    }

    std::ostream &operator<<(std::ostream &out, DictionaryMsgData const &dict)
    {
        out << "Dict(";
        bool first_item = true;
        for (auto &&it = dict.begin(); it != dict.end(); ++it)
        {
            if (first_item)
                first_item = false;
            else
                out << ", ";

            out << it->first << ":";
            auto itemVal = it->second;

            switch (itemVal.value_case())
            {
                case DictItemValMsg::kSeriesD:
                    out << "seriesD<..>";
                    break;
                case DictItemValMsg::kSeriesI:
                    out << "seriesI<..>";
                    break;
                case DictItemValMsg::kSeriesString:
                    out << "seriesString<..>";
                    break;
                case DictItemValMsg::kSeriesAny:
                    out << "seriesAny<..>";
                    break;
                case DictItemValMsg::kBool:
                    out << itemVal.bool_();
                    break;
                case DictItemValMsg::kDict:
                    out << itemVal.dict().data();
                    break;
                case DictItemValMsg::kDouble:
                    out << itemVal.double_();
                    break;
                case DictItemValMsg::kInt:
                    out << itemVal.int_();
                    break;
                case DictItemValMsg::kString:
                    out << itemVal.string();
                    break;
                case DictItemValMsg::VALUE_NOT_SET:
                    break;
                default:
                    throw std::runtime_error("Unimplemented DictItemValMsg " +
                                             std::to_string(static_cast<int>(itemVal.value_case())));
            };
        }
        out << ")";
        return out;
    }

    //////////////////////////////////////////////////////////////////////////
    Trace::Trace(PlotlyTrace::CreationMethods method, std::string method_func, Dictionary &kwargs)
      : m_method(method), m_method_func(std::move(method_func))
    {
        m_kwargs.set_kwargs(kwargs);
    }
    Trace::Trace(PlotlyTrace::CreationMethods method, std::string method_func, Dictionary &&kwargs)
      : m_method(method), m_method_func(std::move(method_func))
    {
        m_kwargs.set_kwargs(std::forward<Dictionary>(kwargs));
    }

    std::ostream &operator<<(std::ostream &out, Trace const &fig)
    {
        out << "trace<" << fig.m_method << "|" << fig.m_method_func << "|" << fig.m_kwargs << ">";
        return out;
    }

    void seriesAny_vector_push_back(std::vector<SeriesAnyMsg_value> &vec, NullValueType null)
    {
        // set null value
        vec.emplace_back();
        vec.back().set_null(SeriesAnyMsg_value_NullValue_NULL_VALUE);
    }
    void seriesAny_vector_push_back(std::vector<SeriesAnyMsg_value> &vec, const std::string &val)
    {
        vec.emplace_back();
        vec.back().set_string(val);
    }
    void seriesAny_vector_push_back(std::vector<SeriesAnyMsg_value> &vec, double val)
    {
        vec.emplace_back();
        vec.back().set_double_(val);
    }
    void seriesAny_vector_push_back(std::vector<SeriesAnyMsg_value> &vec, int val)
    {
        vec.emplace_back();
        vec.back().set_int_(val);
    }

};  // namespace PlotMsg