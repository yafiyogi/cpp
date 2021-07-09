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

#ifndef yy_type_traits_h
# define yy_type_traits_h

#include <string>
#include <string_view>
#include <type_traits>

namespace yafiyogi {

template<typename T>
struct yy_is_vector:
    public std::false_type
{
};

template<typename T>
inline constexpr bool yy_is_vector_v = yy_is_vector<T>::value;

template<typename T>
using yy_is_vector_t = typename yy_is_vector<T>::type;

template<typename T>
struct yy_is_string:
    std::false_type
{
};

template<>
struct yy_is_string<char *>:
    std::true_type
{
};

template<std::size_t N>
struct yy_is_string<char[N]>:
    std::true_type
{
};

template<>
struct yy_is_string<std::string>:
    std::true_type
{
};

template<>
struct yy_is_string<std::string_view>:
    std::true_type
{
};

template<typename T>
inline constexpr bool yy_is_string_v = yy_is_string<T>::value;

template<typename T>
using yy_is_string_t = typename yy_is_string<T>::type;

template<typename T>
struct yy_is_container:
    std::false_type
{
};

template<>
struct yy_is_container<std::string>:
    std::true_type
{
};

template<typename T>
inline constexpr bool yy_is_container_v = yy_is_container<T>::value;

template<typename T>
using yy_is_container_t = typename yy_is_container<T>::type;


template<typename T>
struct yy_container_type
{
  using type = void;
};

template<typename T>
struct yy_container_type<std::basic_string<T>>
{
  using type = T;
};

template<typename T>
struct yy_container_type<std::basic_string_view<T>>
{
  using type = T;
};

template<>
struct yy_container_type<const char *>
{
  using type = char;
};

template<>
struct yy_container_type<char *>
{
  using type = char;
};

template<std::size_t N>
struct yy_container_type<char[N]>
{
  using type = char;
};

template<typename T>
using yy_container_type_t = typename yy_container_type<T>::type;

template<typename T>
struct is_smart_ptr:
    std::false_type
{
};

template<typename T>
struct is_smart_ptr<std::shared_ptr<T>>:
    std::true_type
{
};

template<typename T>
struct is_smart_ptr<std::unique_ptr<T>>:
    std::true_type
{
};

template<typename T>
inline constexpr bool yy_is_smart_ptr_v = yy_is_smart_ptr<T>::value;

template<typename T>
using yy_is_smart_ptr_t = typename yy_is_smart_ptr<T>::type;

template<typename T>
struct is_optional:
    std::false_type
{
};

template<typename T>
struct is_optional<std::optional<T>>:
    std::true_type
{
};

template<typename T>
inline constexpr bool yy_optional_v = yy_optional<T>::value;

template<typename T>
using yy_optional_t = typename yy_optional<T>::type;

} // namespace yafiyogi
#endif // yy_type_traits_h
