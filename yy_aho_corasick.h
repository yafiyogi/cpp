/*

  MIT License

  Copyright (c) 2021-2022 Yafiyogi

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.

*/

#ifndef yy_aho_corasick_h
#define yy_aho_corasick_h

#include <deque>
#include <memory>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>

#include "yy_span.h"
#include "yy_type_traits.h"
#include "yy_utility.h"
#include "yy_vector_util.h"

namespace yafiyogi::yy_data {
namespace detail {

template<typename K, typename PayloadType>
class trie_node;

template<typename K, typename PayloadType>
struct trie_node_shim;

template<typename K,
         typename PayloadType,
         std::enable_if_t<yy_traits::is_container_v<K>, bool> = true>
struct trie_node_traits
{
    using key_type = K;
    using node_key_type = yy_traits::container_type_t<key_type>;
    using node_type = trie_node<key_type, PayloadType>;
    using node_ptr = std::shared_ptr<node_type>;
    using node_shim = trie_node_shim<key_type, PayloadType>;
    using node_queue = std::deque<node_shim>;
    using payload_type = PayloadType;
};

template<typename K, typename PayloadType>
struct trie_node_shim
{
  public:
    using traits = trie_node_traits<K, PayloadType>;
    using node_key_type = typename traits::node_key_type;
    using node_ptr = typename traits::node_ptr;

    node_key_type key;
    node_ptr node;

    bool operator==(const trie_node_shim & other) const
    {
      return key == other.key;
    }

    bool operator==(const node_key_type & other) const
    {
      return key == other;
    }
};

template<typename K, typename PayloadType>
class trie_node
{
  public:
    using traits = trie_node_traits<K, PayloadType>;
    using key_type = typename traits::key_type;
    using node_key_type = typename traits::node_key_type;
    using node_ptr = typename traits::node_ptr;
    using payload_type = typename traits::payload_type;
    using node_shim = typename traits::node_shim;

    trie_node() = delete;
    trie_node(const trie_node & node) = delete;
    trie_node(trie_node && node) = delete;
    trie_node operator=(const trie_node & node) = delete;
    trie_node operator=(trie_node && node) = delete;
    virtual ~trie_node() = default;

    trie_node(node_ptr fail) :
      m_fail(std::move(fail))
    {
    }

    node_ptr add(node_key_type key, const node_ptr & fail)
    {
      auto [iter, found] = yy_util::find(m_children, key, comp);

      if(found)
      {
        return iter->node;
      }

      auto node = std::make_shared<trie_node>(fail);

      m_children.emplace(iter, node_shim{std::move(key), node});

      return node;
    }

    void add(node_shim child)
    {
      auto & key = child.key;
      auto [iter, found] = yy_util::find(m_children, key, comp);

      if(!found)
      {
        m_children.emplace(iter, std::move(child));
      }
      else
      {
        auto & shim = *iter;
        std::swap(shim.node->m_fail, child.node->m_fail);
        std::swap(shim.node->m_children, child.node->m_children);
        shim = child;
      }
    }

    node_ptr get(const node_key_type & key) const
    {
      auto [iter, found] = yy_util::find(m_children, key, comp);
      node_ptr node;

      if(!found)
      {
        return node_ptr{};
      }

      return iter->node;
    }

    node_ptr get(const node_key_type & key, node_ptr def) const
    {
      auto node = get(key);

      if(!node)
      {
        return def;
      }

      return node;
    }

    node_ptr exists(const node_key_type & key) const
    {
      auto [iter, found] = yy_util::find(m_children, key, comp);

      return found;
    }

    auto & fail() const
    {
      return m_fail;
    }

    void fail(node_ptr f)
    {
      m_fail = f;
    }

    template<typename V>
    void visit(V && visitor)
    {
      for(auto & child: m_children)
      {
        visitor.visit(child);
      }
    }

    virtual bool empty() const
    {
      return true;
    }

    virtual const PayloadType & value() const
    {
      throw std::runtime_error("Invalid value");
    }

    virtual PayloadType & value()
    {
      throw std::runtime_error("Invalid value");
    }

  private:
    static int comp(const node_shim & shim, const node_key_type & value)
    {
      if(value < shim.key)
      {
        return -1;
      }
      else if(value == shim.key)
      {
        return 0;
      }
      return 1;
    }

    node_ptr m_fail;
    std::vector<node_shim> m_children;
};

template<typename K, typename PayloadType>
class add_children_visitor
{
  public:
    using traits = trie_node_traits<K, PayloadType>;
    using node_ptr = typename traits::node_ptr;
    using node_queue = typename traits::node_queue;
    using node_shim = typename traits::node_shim;

    add_children_visitor(node_queue * queue) :
      m_queue(queue)
    {
    }

    void visit(node_shim & child)
    {
      m_queue->emplace_back(child);
    }

  private:
    node_queue * m_queue;
};

template<typename K, typename PayloadType>
class compile_visitor
{
  public:
    using traits = trie_node_traits<K, PayloadType>;
    using node_ptr = typename traits::node_ptr;
    using node_queue = typename traits::node_queue;
    using node_shim = typename traits::node_shim;

    compile_visitor(node_queue * queue, node_ptr fail, node_ptr root) :
      m_queue(queue),
      m_fail(std::move(fail)),
      m_root(std::move(root))
    {
    }

    void visit(node_shim & child)
    {
      m_queue->emplace_back(child);

      auto state = m_fail;
      auto fail = m_root;

      while(fail == m_root)
      {
        fail = state->get(child.key, m_root);
        state = state->fail();

        if(state == m_root)
        {
          break;
        }
      }

      child.node->fail(fail);
    }

  private:
    node_queue * m_queue;
    node_ptr m_fail;
    node_ptr m_root;
};

} // namespace detail

template<typename K, typename PayloadType>
class ac_trie
{
  public:
    using traits = typename detail::trie_node_traits<K, PayloadType>;
    using key_type = typename traits::key_type;
    using node_key_type = typename traits::node_key_type;
    using node_type = typename traits::node_type;
    using node_ptr = typename traits::node_ptr;
    using node_queue = typename traits::node_queue;
    using node_shim = typename traits::node_shim;
    using payload_type = typename traits::payload_type;

    class Automaton
    {
      public:
        Automaton(node_ptr root) :
          m_root(std::move(root)),
          m_state(m_root)
        {
        }

        void next(const node_key_type ch)
        {
          auto node = m_state;

          while(true)
          {
            const node_ptr child = node->get(ch);

            if(child)
            {
              node = child;
              break;
            }
            else if(node == m_root)
            {
              break;
            }
            node = node->fail();
          }

          m_state = std::move(node);
        }

        bool word(const node_key_type * key)
        {
          return word(yy_util::span<key_type>(key));
        }

        bool word(const yy_util::span<key_type> key)
        {
          if(!key.empty())
          {
            auto begin = key.begin();
            auto ch = *begin;
            next(ch);

            if(m_state != m_root)
            {
              ++begin;
              const auto end = key.end();

              auto node = m_state;

              while(begin != end)
              {
                const auto & child = node->get(ch);

                if(!child)
                {
                  // Not found child node.
                  return false;
                }

                node = child;
              }

              m_state = std::move(node);

              if(end == begin)
              {
                return empty();
              }
            }
          }

          return false;
        }

        bool empty() const
        {
          return m_state->empty();
        }

        template<typename V>
        void visit(V && visitor) const
        {
          if(m_state != m_root)
          {
            if(!m_state->empty())
            {
              visitor(m_state->value());
            }
          }
        }

        template<typename V>
        void visit_all(V && visitor) const
        {
          auto node = m_state;

          while(node != m_root)
          {
            if(!node.empty())
            {
              const auto & payload = static_cast<Payload &>(*m_state);

              visitor(payload.value());
            }

            node = node->fail();
          }
        }

      private:
        const node_ptr m_root;
        node_ptr m_state;
    };

    ac_trie() :
      m_root(std::make_shared<RootNode>())
    {
      m_root->fail(m_root);
    }

    void add(const node_key_type * word)
    {
      add(yy_util::span<key_type>(word));
    }

    void add(const yy_util::span<key_type> word, PayloadType && value)
    {
      if(!word.empty())
      {
        node_ptr parent = m_root;

        for(const auto & key:
            yy_util::make_range(word.begin(), std::prev(word.end())))
        {
          parent = parent->add(key, m_root);
        }

        auto child = node_shim{
          word.back(),
          std::make_shared<Payload>(m_root, std::forward<PayloadType>(value))};

        parent->add(std::move(child));
      }
    }

    void compile()
    {
      node_queue queue;
      m_root->visit(
        detail::add_children_visitor<key_type, payload_type>(&queue));

      while(!queue.empty())
      {
        auto shim = queue.front();
        queue.pop_front();
        const node_ptr node = shim.node;

        node->visit(
          detail::compile_visitor<key_type, payload_type>(&queue,
                                                          node->fail(),
                                                          m_root));
      }
    }

    Automaton create_automaton()
    {
      return Automaton{m_root};
    }

  private:
    class RootNode: public node_type
    {
      public:
        RootNode() :
          node_type(node_ptr{})
        {
        }

        RootNode(const RootNode &) = delete;
        RootNode(RootNode &&) = delete;
    };

    class Payload: public node_type
    {
      public:
        Payload(node_ptr fail, PayloadType && payload) :
          node_type(std::move(fail)),
          m_payload(std::move(payload))
        {
        }

        bool empty() const override
        {
          return false;
        }

        const PayloadType & value() const override
        {
          return m_payload;
        }

        PayloadType & value() override
        {
          return m_payload;
        }

      private:
        PayloadType m_payload;
    };

    node_ptr m_root;
};

} // namespace yafiyogi::yy_data

#endif // yy_aho_corasick_h
