/*

MIT License

Copyright (c) 2019 Yafiyogi

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

#ifndef yy_func_traits_h
# define yy_func_traits_h

namespace yafiyogi {

template<typename T>
struct yy_func_traits:
            public yy_func_traits<decltype(&T::operator())>
{
};

template<typename C, typename R, typename... Args>
struct yy_func_traits<R(C::*)(Args...) const>
{
    using result_type = R;

    template<size_t N>
    struct arg
    {
        using type = typename std::tuple_element<N, std::tuple<Args...>>::type;
    };
};

template<typename C, typename... Args>
struct yy_func_traits<void(C::*)(Args...) const>
{
    using result_type = void;

    template<size_t N>
    struct arg
    {
        using type = typename std::tuple_element<N, std::tuple<Args...>>::type;
    };
};

} // namespace yafiyogi
#endif // yy_func_traits_h
