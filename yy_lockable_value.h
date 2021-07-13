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

#ifndef yy_lockable_value_h
#define yy_lockable_value_h

#include <utility>

#include "yy_type_traits.h"

namespace yafiyogi {
namespace traits {

template<typename Value,
         typename Visitor,
         typename Enable=void>
struct LockTypeVisitor
{
  static void visit(Value && value,
                    Visitor && visitor)
  {
    visitor(std::forward<Value>(value));
  }
};

template<typename Value,
         typename Visitor>
struct LockTypeVisitor<Value, Visitor, std::enable_if_t<std::is_pointer_v<Value> || is_smart_ptr_v<Value>>>
{
  static void visit(Value && value,
                    Visitor && visitor)
  {
    if(value)
    {
      visitor(*value);
    }
  }
};

template<typename Value,
         typename Visitor>
struct LockTypeVisitor<Value, Visitor, std::enable_if_t<is_optional_v<Value>>>
{
  static void visit(Value && value,
                    Visitor && visitor)
  {
    if(value.has_value())
    {
      visitor(value.value());
    }
  }
};

} // namespace traits

template<typename ValueType,
         typename MutexType>
class LockableValue
{
public:
  using value_type = ValueType;
  using mutex_type = MutexType;

  LockableValue() = default;
  LockableValue(const LockableValue &) = delete;
  LockableValue(LockableValue &&) = default;

  LockableValue & operator=(const LockableValue &) = delete;
  LockableValue & operator=(const LockableValue && rhs)
  {
    if( this != &rhs)
    {
      m_value = std::move(rhs.m_value);
      m_mtx = std::move(rhs.m_mtx);
    }

    return *this;
  }

  template<typename... Args>
  LockableValue(Args && ...args):
    m_value(std::forward<Args>(args)...),
    m_mtx() {}

  template<typename LockType>
  value_type get() const
  {
    LockType lck(m_mtx);

    return m_value;
  }

  template<typename LockType>
  value_type set(value_type && value)
  {
    LockType lck(m_mtx);

    return m_value = std::forward<value_type>(value);
  }

  template<typename LockType>
  value_type exchange(value_type value)
  {
    LockType lck(m_mtx);

    std::swap(m_value, value);

    return value;
  }

private:
  value_type m_value;
  mutable mutex_type m_mtx;
};

template<typename LockableValue,
         typename GuardType>
class LockType
{
public:
  using value_type = typename LockableValue::value_type;
  using lock_type = GuardType;

  static value_type get(const LockableValue & lockable)
  {
    return lockable.template get<lock_type>();
  }

  static value_type set(LockableValue & lockable,
                        value_type value)
  {
    return lockable.template set<lock_type>(std::move(value));
  }

  static value_type exchange(LockableValue & lockable, value_type value)
  {
    return lockable.template exchange<lock_type>(std::move(value));
  }

  template<typename Visitor>
  static void visit(const LockableValue & lockable,
                    Visitor && visitor)
  {
    using Traits = traits::LockTypeVisitor<typename LockableValue::value_type, Visitor>;

    auto value = get(lockable);

    Traits::visit(std::move(value), std::forward<Visitor>(visitor));
  }
};

} // namespace yafiyogi

#endif // yy_lockable_value_h
