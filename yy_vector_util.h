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
#include <vector>
#include <algorithm>

#include "yy_type_traits.h"

namespace yafiyogi {

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

template<typename T,
         typename V,
         typename C,
         std::enable_if_t<yy_is_vector_v<std::remove_cv_t<std::remove_reference_t<T>>>, bool> = true>
auto Find(T & container, const V & value, C && comp)
{
  auto iter = std::lower_bound( container.begin(),
                                container.end(),
                                value,
                                comp);
  bool found = (iter != container.end()) && (*iter == value);
  return std::make_tuple(iter, found);
}

} // namespace yafiyogi
#endif // yy_vector_util_h
