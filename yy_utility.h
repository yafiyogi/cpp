/*

  MIT License

  Copyright (c) 2021-2024 Yafiyogi

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
#define yy_utility_h

#include <array>
#include <utility>
#include <memory>

namespace yafiyogi::yy_util {

template<typename Iterator>
class Range final
{
  public:
    using iterator = std::remove_reference_t<std::remove_volatile_t<Iterator>>;
    constexpr explicit Range(const iterator & p_begin,
                             const iterator & p_end) noexcept:
      m_begin(p_begin),
      m_end(p_end)
    {
    }

    constexpr explicit Range(iterator && p_begin,
                             iterator && p_end) noexcept:
      m_begin(std::move(p_begin)),
      m_end(std::move(p_end))
    {
    }

    constexpr Range() = delete;
    constexpr Range(const Range &) noexcept = default;
    constexpr Range(Range &&) noexcept = default;
    constexpr ~Range() noexcept = default;

    constexpr Range & operator=(const Range &) noexcept = default;
    constexpr Range & operator=(Range &&) noexcept = default;

    [[nodiscard]]
    constexpr iterator begin() const noexcept
    {
      return m_begin;
    }

    [[nodiscard]]
    constexpr iterator end() const noexcept
    {
      return m_end;
    }

  private:
    iterator m_begin;
    iterator m_end;
};

template<typename Iterator>
constexpr auto make_range(const Iterator & begin, const Iterator & end) noexcept
{
  return Range<Iterator>{begin, end};
}

template<typename Iterator>
constexpr auto make_range(Iterator && begin, Iterator && end) noexcept
{
  return Range<Iterator>{std::forward<Iterator>(begin), std::forward<Iterator>(end)};
}

template<typename T>
struct ArraySize
{
    static constexpr size_t size = 0;
};

template<typename T, size_t Size>
struct ArraySize<T[Size]>
{
    static constexpr size_t size = Size;
};

template<typename T, size_t Size>
struct ArraySize<std::array<T, Size>>
{
    static constexpr size_t size = Size;
};

template<typename Return,
         typename T>
constexpr std::unique_ptr<Return> static_unique_cast(T && ptr)
{
  static_assert(yy_traits::is_unique_ptr_v<T>, "static_unique_cast(): parameter must be a std::unique_ptr<T>.");

  return std::unique_ptr<Return>(static_cast<Return *>(ptr.release()));
}

} // namespace yafiyogi::yy_util

#endif // yy_utility_h
