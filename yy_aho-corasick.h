/*

  MIT License

  Copyright (c) 2021 Yafiyogi

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
# define yy_aho_corasick_h

#include <type_traits>
#include <memory>
#include <vector>
#include <deque>
#include <stdexcept>

#include "yy_type_traits.h"
#include "yy_utility.h"
#include "yy_vector_util.h"
#include "yy_span.h"

using namespace std;

namespace yafiyogi {
namespace detail {

template<typename K,
         typename PayloadType>
class trie_node;

template<typename K,
         typename PayloadType>
struct trie_node_shim;

template<typename K,
         typename PayloadType,
         std::enable_if_t<yy_is_container_v<K>, bool> = true>
struct trie_node_traits
{
  using key_type = K;
  using node_key_type = yy_container_type_t<key_type>;
  using node_type = trie_node<key_type, PayloadType>;
  using node_ptr = std::shared_ptr<node_type>;
  using node_queue = std::deque<node_ptr>;
  using node_shim = trie_node_shim<key_type, PayloadType>;
  using payload_type = PayloadType;
};

template<typename K,
         typename PayloadType>
struct trie_node_shim
{
  using traits = trie_node_traits<K, PayloadType>;
  using node_key_type = typename traits::node_key_type;
  using node_ptr = typename traits::node_ptr;

  node_key_type key;
  node_ptr node;

  bool operator==( const trie_node_shim & other) const
  {
    return key == other.key;
  }

  bool operator==( const node_key_type & other) const
  {
    return key == other;
  }
};

template<typename K,
         typename PayloadType>
class trie_node
{
public:
  using traits = trie_node_traits<K, PayloadType>;
  using key_type = typename traits::key_type;
  using node_key_type = typename traits::node_key_type;
  using node_ptr = typename traits::node_ptr;
  using payload_type = typename traits::payload_type;
  using node_shim = trie_node_shim<key_type, payload_type>;

  trie_node() = delete;
  trie_node(const trie_node & node) = delete;
  trie_node(trie_node && node) = delete;
  virtual ~trie_node() = default;

  trie_node(node_key_type key,
            node_ptr fail):
    m_key( std::move(key)),
    m_fail( std::move(fail)),
    m_id( ++id)
  {

  }

  node_ptr add( node_key_type key, const node_ptr & fail)
  {
    auto [iter, found] = Find( m_children, key, comp);

    if(found)
    {
      return iter->node;
    }

    auto node = std::make_shared<trie_node>( key, fail);

    m_children.emplace( iter, node_shim{std::move(key), node});

    return node;
  }

  void add( node_ptr child)
  {
    auto & key = child->key();
    auto [iter, found] = Find( m_children, key, comp);

    if(!found)
    {
      m_children.emplace( iter, node_shim{key, std::move(child)});
    }
    else
    {
      auto && node = iter->node;
      std::swap( node->m_fail, child->m_fail);
      std::swap( node->m_children, child->m_children);
      node = child;
    }
  }

  node_ptr get( const node_key_type & key) const
  {
    auto [iter, found] = Find( m_children, key, comp);
    node_ptr node;

    if( !found)
    {
      return node_ptr{};
    }

    return iter->node;
  }

  node_ptr get( const node_key_type & key, node_ptr def) const
  {
    auto node = get( key);

    if( !node)
    {
      return def;
    }

    return node;
  }

  node_ptr exists( const node_key_type & key) const
  {
    auto [iter, found] = Find( m_children, key, comp);

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
    for( auto && child: m_children)
    {
      visitor.visit(child.node);
    }
  }

  const node_key_type & key() const
  {
    return m_key;
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

  int m_id;

private:
  static bool comp(const node_shim & shim,
                   const node_key_type & value)
  {
    return value < shim.key;
  };
  node_key_type m_key;
  node_ptr m_fail;
  std::vector<node_shim> m_children;
  static inline int id = 0;
};

template<typename K,
         typename PayloadType>
class add_children_visitor
{
public:
  using traits = trie_node_traits<K, PayloadType>;
  using node_ptr = typename traits::node_ptr;
  using node_queue = typename traits::node_queue;

  add_children_visitor(node_queue * queue):
    m_queue(queue)
  {
  }

  void visit(node_ptr & child)
  {
    m_queue->emplace_back(child);
  }

private:
  node_queue * m_queue;
};

template<typename K,
         typename PayloadType>
class compile_visitor
{
public:
  using traits = trie_node_traits<K, PayloadType>;
  using node_ptr = typename traits::node_ptr;
  using node_queue = typename traits::node_queue;

  compile_visitor(node_queue * queue,
                  node_ptr fail,
                  node_ptr root):
    m_queue(queue),
    m_fail(std::move(fail)),
    m_root(std::move(root))
  {
  }

  void visit(node_ptr & child)
  {
    m_queue->emplace_back(child);

    auto state = m_fail;
    auto fail = m_root;

    while(fail == m_root)
    {
      fail = state->get(child->key(), m_root);
      state = state->fail();

      if( state == m_root)
      {
        break;
      }
    }

    child->fail(fail);
  }

private:
  node_queue * m_queue;
  node_ptr m_fail;
  node_ptr m_root;
};

} // namespace detail

template<typename K,
         typename PayloadType>
class trie
{
public:
  using traits = typename detail::trie_node_traits<K,PayloadType>;
  using key_type = typename traits::key_type;
  using node_key_type = typename traits::node_key_type;
  using node_type = typename traits::node_type;
  using node_ptr = typename traits::node_ptr;
  using node_queue = typename traits::node_queue;
  using payload_type = typename traits::payload_type;

  class Automaton
  {
  public:
    Automaton( node_ptr root):
      m_root( std::move( root)),
      m_state( m_root) {}

    void next( const node_key_type ch)
    {
      auto node = m_state;

      while(true)
      {
        auto && child = node->get( ch);

        if( child)
        {
          node = child;
          break;
        }
        else if( node == m_root)
        {
          break;
        }
        node = node->fail();
      }

      m_state = std::move(node);
    }

    bool empty() const
    {
      return m_state->empty();
    }

    template<typename V>
    void visit( V && visitor) const
    {
      if( m_state != m_root)
      {
        if( !m_state->empty())
        {
          visitor( m_state->value());
        }
      }
    }

    template<typename V>
    void visit_all( V && visitor) const
    {
      auto node = m_state;

      while( node != m_root)
      {
        if( !node.empty())
        {
          auto && payload = static_cast<Payload &>( *m_state);

          visitor( payload.value());
        }

        node = node->fail();
      }
    }

  private:
    const node_ptr m_root;
    node_ptr m_state;
  };

  trie():
    m_root( std::make_shared<RootNode>())
  {
    m_root->fail( m_root);
  }

  void add( yy_span<key_type> word,
            PayloadType && value)
  {
    if( !word.empty())
    {
      node_ptr parent = m_root;

      for( auto && key : make_range( word.begin(),
                                     std::prev( word.end())))
      {
        parent = parent->add( key, m_root);
      }

      node_ptr child = std::make_shared<Payload>( word.back(),
                                                  m_root,
                                                  std::forward<PayloadType>(value));

      parent->add( std::move( child));
    }
  }

  void compile()
  {
    node_queue queue;
    m_root->visit(detail::add_children_visitor<key_type, payload_type>(&queue));

    while(!queue.empty())
    {
      auto node = queue.front();
      node->visit(detail::compile_visitor<key_type, payload_type>(&queue, node->fail(), m_root));
      queue.pop_front();
    }
  }

  Automaton create_automaton()
  {
    return Automaton{m_root};
  }

private:
  class RootNode:
    public node_type
  {
  public:
    RootNode():
      node_type( node_key_type{}, node_ptr{})
    {
    }

    RootNode(const RootNode &) = delete;
    RootNode(RootNode &&) = delete;
  };

  class Payload:
    public node_type
  {
  public:
    Payload(node_key_type key,
            node_ptr fail,
            PayloadType && payload):
      node_type(std::move(key), std::move(fail)),
      m_payload(std::move(payload))
    {
    }

    bool empty() const  override
    {
      return false;
    }

    const PayloadType & value() const override
    {
      return m_payload;
    }

    PayloadType & value()  override
    {
      return m_payload;
    }

  private:
    PayloadType m_payload;
  };

  node_ptr m_root;
};

} // namespace yafiyogi

#endif // yy_aho_corasick_h
