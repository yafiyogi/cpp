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

#ifndef yy_vector_util_h
# define yy_vector_util_h

#include <tuple>
#include <type_traits>
#include <array>
#include <vector>
#include <algorithm>

#include "yy_type_traits.h"

namespace yafiyogi {
namespace detail {

template<typename V,
         typename O,
         typename C>
class LessThanComp
{
public:
  LessThanComp(C comp):
    m_comp(std::move(comp))
  {
  }

  bool operator()(const O & o,
                  const V & v) const
  {
    return m_comp(o, v) < 0;
  }

private:
  C m_comp;
};

template<typename V,
         typename O,
         typename C>
class EqualToComp
{
public:
  EqualToComp(C comp):
    m_comp(std::move(comp))
  {
  }

  bool operator()(const O & o,
                  const V & v) const
  {
    return 0 == m_comp(o, v);
  }

private:
  C m_comp;
};

} // namespace detail

template<typename T,
         std::size_t N>
struct yy_is_vector<std::array<T, N>>:
    public std::true_type
{
};

template<typename T,
         std::size_t N>
struct yy_is_container<std::array<T, N>>:
    std::true_type
{
};

template<typename T,
         std::size_t N>
struct yy_container_type<std::array<T, N>>
{
  using type = typename std::array<T, N>::value_type;
};

template<typename T>
struct yy_is_vector<std::vector<T>>:
    public std::true_type
{
};

template<typename T>
struct yy_is_container<std::vector<T>>:
    std::true_type
{
};

template<typename T>
struct yy_container_type<std::vector<T>>
{
  using type = typename std::vector<T>::value_type;
};

template<typename T,
         typename V,
         typename C,
         std::enable_if_t<yy_is_vector_v<std::remove_cv_t<std::remove_reference_t<T>>>, bool> = true>
auto Find(T & container, const V & value, C && comp)
{
  using value_type = yy_container_type_t<std::remove_cv_t<std::remove_reference_t<T>>>;
  using less_than = detail::LessThanComp<V, value_type, C>;
  using equal_to = detail::EqualToComp<V, value_type, C>;

  auto iter = std::lower_bound( container.begin(),
                                container.end(),
                                value,
                                less_than{comp});
  bool found = (iter != container.end()) && equal_to{comp}(*iter, value);
  return std::make_tuple(iter, found);
}

template<typename T,
         typename V,
         std::enable_if_t<yy_is_vector_v<std::remove_cv_t<std::remove_reference_t<T>>>, bool> = true>
auto Find(T & container, const V & value)
{
  using value_type = yy_container_type_t<std::remove_cv_t<std::remove_reference_t<T>>>;

  auto comp = [](const auto & item,
                 const auto & v) -> int
  {
    if(v < item)
    {
      return -1;
    }
    return 0;
  };

  using less_than = detail::LessThanComp<V, value_type, decltype(comp)>;

  auto iter = std::lower_bound( container.begin(),
                                container.end(),
                                value,
                                less_than{comp});
  bool found = (iter != container.end()) && (*iter == value);
  return std::make_tuple(iter, found);
}

} // namespace yafiyogi
#endif // yy_vector_util_h
