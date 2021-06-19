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

#ifndef yy_utility_h
# define yy_utility_h

namespace yafiyogi {

template<typename I>
class Range
{
public:
  Range( I && b,
         I && e):
    m_begin( std::forward<I>( b)),
    m_end( std::forward<I>( e)) {}

  auto & begin() const
  {
    return m_begin;
  }

  auto & end() const
  {
    return m_end;
  }

private:
  I m_begin;
  I m_end;
};

template<typename I>
auto make_range( I && b,
                 I && e)
{
  return Range<I>{ std::forward<I>( b),
    std::forward<I>( e)};
}

} // namespace yafiyogi

#endif // yy_utility_h
