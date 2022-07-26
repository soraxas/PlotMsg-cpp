#pragma once

#include "core.hpp"
#include "helpers.hpp"
#include "index_proxy_access.hpp"

namespace PlotMsg
{

    // class that represents dict representation in protobuf
    class Dictionary
    {
    public:
        struct DictionaryItemPair
        {
        public:
            template <typename T>
            DictionaryItemPair(const std::basic_string<char> &key, T value)
            {
                m_key = key;
                PlotMsg::_set_DictItemVal(m_item_val, std::forward<T>(value));
            }

            std::basic_string<char> m_key;
            DictItemValMsg m_item_val;
        };

        Dictionary()
        {
            reset();
        }

        // template to create dictionary with arbitrary amount of item pair
        template <typename T, typename... Types>
        Dictionary(const std::basic_string<char> &key, T val, Types... rest) : Dictionary(rest...)
        {
            add_kwargs(key, val);
        }

        // template bases-case
        template <typename T>
        Dictionary(const std::basic_string<char> &key, T val) : Dictionary()
        {
            add_kwargs(key, val);
        }

        // template to create dictionary with dict item pair
        explicit Dictionary(DictionaryItemPair pair) : Dictionary()
        {
            add_kwargs(pair);
        }

        // template bases-case
        template <typename... Ts, typename = DictionaryItemPair>
        explicit Dictionary(DictionaryItemPair pair, Ts... rest) : Dictionary(rest...)
        {
            add_kwargs(pair);
        }

        // copy-construct
        Dictionary(const Dictionary &dict)
        {
            reset();
            m_msg->ParseFromString(dict.m_msg->SerializeAsString());
        }

        // swap-construct
        Dictionary(Dictionary &dict)
        {
            reset();
            m_msg.swap(dict.m_msg);
        }

        // rvalue-construct
        Dictionary(Dictionary &&dict) noexcept
        {
            m_msg = std::move(dict.m_msg);
        }

        //// ONLY ENABLE FOR CERTAIN CLASS
        //  template <bool...> struct bool_pack {};
        //  template <class... Ts>
        //  using conjunction = std::is_same<bool_pack<true, Ts::value...>,
        //                                   bool_pack<Ts::value..., true>>;
        //
        //  template <typename... Ts>
        //  using AllDictionaryItemPair = typename std::enable_if<
        //      conjunction<std::is_convertible<Ts,
        //      DictionaryItemPair>...>::value>::type;

        // methods to add kwargs into the dictionary

        template <typename T>
        void add_kwargs(const std::basic_string<char> &key, T value)
        {
            // pass the DictItemValMsg reference to helper function as template
            PlotMsg::_set_DictItemVal((*m_msg->mutable_data())[key], std::forward<T>(value));
        }

        void add_kwargs(DictionaryItemPair &value) const;

        void update_kwargs(Dictionary &value) const;

        void update_kwargs(Dictionary &&value) const;

        // directly set kwargs
        void set_kwargs(Dictionary &value) const;

        void set_kwargs(Dictionary &&value) const;

        std::unique_ptr<Dictionary> deep_copy() const;

        IndexAccessProxy operator[](const std::basic_string<char> &key) const
        {
            return IndexAccessProxy(*m_msg->mutable_data(), key);
        }

        ///////////////////////////////////////////////////

        friend std::ostream &operator<<(std::ostream &out, Dictionary const &dict);

        void reset();

        DictionaryMsg *release_ptr();

        // variables
        std::unique_ptr<DictionaryMsg> m_msg;
    };

    void Dictionary::add_kwargs(Dictionary::DictionaryItemPair &value) const
    {
        (*m_msg->mutable_data())[value.m_key].Swap(&value.m_item_val);
    }

    void Dictionary::update_kwargs(Dictionary &&value) const
    {
        Dictionary lvalue(value);
        update_kwargs(lvalue);
    }

    /**
     * Like Python's dictionary update function.
     * @param value
     */
    void Dictionary::update_kwargs(Dictionary &value) const
    {
        for (auto &kv_pair : (*value.m_msg->mutable_data()))
        {
            (*m_msg->mutable_data())[kv_pair.first].Swap(&kv_pair.second);
        }
    }

    void Dictionary::set_kwargs(Dictionary &&value) const
    {
        Dictionary lvalue(value);
        set_kwargs(lvalue);
    }

    void Dictionary::set_kwargs(Dictionary &value) const
    {
        m_msg->mutable_data()->swap(*value.m_msg->mutable_data());
    }

    std::unique_ptr<Dictionary> Dictionary::deep_copy() const
    {
        Dictionary dict;
        _deep_copy_helper(m_msg->data(), *dict.m_msg->mutable_data());
        return std::make_unique<Dictionary>(dict);
    }

    void Dictionary::reset()
    {
        m_msg = std::make_unique<DictionaryMsg>();
    }

    DictionaryMsg *Dictionary::release_ptr()
    {
        return m_msg.release();
    }

    std::ostream &operator<<(std::ostream &out, Dictionary const &dict)
    {
        return out << (*dict.m_msg).data();
    }

    void send(Dictionary &container, zmq::send_flags send_flags = zmq::send_flags::dontwait)
    {
        initialise_publisher();

        MessageContainer msg;
        msg.mutable_dict()->Swap(container.release_ptr());

        std::string encoded_msg;
        msg.SerializeToString(&encoded_msg);
        zmq::message_t zmq_msg(encoded_msg.size());
        memcpy((void *)zmq_msg.data(), encoded_msg.c_str(), encoded_msg.size());
        static_publisher->send(zmq_msg, send_flags);
    }

}  // namespace PlotMsg