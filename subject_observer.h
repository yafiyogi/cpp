/*

  MIT License

  Copyright (c) 2019-2022 Yafiyogi

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

#ifndef SubjectObserver_h
#define SubjectObserver_h

#include <algorithm>
#include <memory>
#include <unordered_map>

#include "yy_func_traits.h"

namespace yafiyogi::yy_data {

template<typename R, typename... Args>
class observer_base
{
  public:
    observer_base() = default;
    observer_base(const observer_base &) = default;
    observer_base(observer_base &&) noexcept = default;
    observer_base & operator=(const observer_base &) = default;
    observer_base & operator=(observer_base &&) noexcept = default;

    virtual ~observer_base() = default;
    using ptr_type = std::unique_ptr<observer_base<R, Args...>>;

    virtual R event(const void * data, Args && ...args) = 0;
};

template<typename T, typename R, typename A, typename... Args>
class observer_class_method final: public observer_base<R, Args...>
{
  public:
    using method_ptr = R (T::*)(const A *, Args &&...);
    using object_ptr = std::shared_ptr<T>;

    observer_class_method(const object_ptr & obj, method_ptr method) :
      m_obj(obj),
      m_method(method)
    {
    }

    R event(const void * data, Args && ...args) override
    {
      T * obj = m_obj.get();

      return (obj->*m_method)(reinterpret_cast<const A *>(data), std::forward<Args>(args)...);
    }

  private:
    object_ptr m_obj;
    method_ptr m_method;
};

template<typename T, typename R, typename... Args>
class observer_func final: public observer_base<R, Args...>
{
  public:
    using func_traits = yy_traits::func_traits<T>;
    using arg_type = typename func_traits::arg_types::arg_type<0>;

    observer_func(T func) :
      m_func(func)
    {
    }

    R event(const void * data, Args && ...args) override
    {
      return m_func(reinterpret_cast<arg_type>(data), std::forward<Args>(args)...);
    }

  private:
    T m_func;
};

template<typename K, typename R, typename... Args>
class subject
{
  public:
    using observer_type = observer_base<R, Args...>;
    using map_type = std::unordered_map<K, typename observer_type::ptr_type>;

    std::tuple<bool, R>
    event(const K & key, const void * data, Args && ...args)
    {
      auto found = m_observers.find(key);

      if(m_observers.end() != found)
      {
        return {true, found->second->event(data, std::forward<Args>(args)...)};
      }
      return {false, R{}};
    }

    // Add object method.
    template<typename T, typename A>
    bool add(const K & key,
             typename std::shared_ptr<T> & obj,
             R (T::*method)(const A *, Args && ...args))
    {
      bool rv = false;
      if(obj)
      {
        typename map_type::iterator not_used;
        std::tie(not_used, rv) = m_observers.try_emplace(
          key,
          std::make_unique<observer_class_method<T, R, A, Args...>>(obj,
                                                                    method));
      }
      return rv;
    }

    template<typename T>
    bool add(const K & key, T && func)
    {
      auto [not_used, rv] = m_observers.try_emplace(
        key,
        std::make_unique<observer_func<T, R, Args...>>(std::forward<T>(func)));

      return rv;
    }

    void erase(const K & key)
    {
      m_observers.erase(key);
    }

    map_type m_observers;
};

template<typename K, typename... Args>
class subject<K, void, Args...>
{
  public:
    using observer_ptr = typename observer_base<void, Args...>::ptr_type;
    using map_type = std::unordered_map<K, observer_ptr>;

    bool event(const K & key, const void * data, Args && ...args)
    {
      auto found = m_observers.find(key);

      const bool call = m_observers.end() != found;
      if(call)
      {
        found->second->event(data, std::forward<Args>(args)...);
      }

      return call;
    }

    // Add object method.
    template<typename T, typename A>
    bool add(const K & key,
             typename std::shared_ptr<T> & obj,
             void (T::*method)(const A *, Args && ...args))
    {
      bool rv = false;
      if(obj)
      {
        typename map_type::iterator not_used;
        std::tie(not_used, rv) = m_observers.try_emplace(
          key,
          std::make_unique<observer_class_method<T, void, A, Args...>>(obj,
                                                                       method));
      }
      return rv;
    }

    template<typename T>
    bool add(const K & key, T func)
    {
      auto [not_used, rv] = m_observers.try_emplace(
        key,
        std::make_unique<observer_func<T, void, Args...>>(func));

      return rv;
    }

    void erase(const K & key)
    {
      m_observers.erase(key);
    }

    map_type m_observers;
};

} // namespace yafiyogi::yy_data

#endif // SubjectObserver_h
