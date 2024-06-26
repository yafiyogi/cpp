/*

  MIT License

  Copyright (c) 2024 Yafiyogi

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

#include "fmt/format.h"

#include "bench_yy_cpp.h"

namespace yafiyogi::benchmark {

BENCHMARK_F(TrieFixtureType, flat_radix_lookup)(::benchmark::State & state)
{
  auto automaton = flat_radix_trie.create_automaton();
  assert(!automaton.empty());

  size_t idx = 0;
  std::size_t count = 0;

  while(state.KeepRunning())
  {
    bool found = automaton.find(TrieFixtureType::query(idx));
    ::benchmark::DoNotOptimize(found);
    if(found)
    {
      automaton.visit([&count](auto &) {
        ::benchmark::DoNotOptimize(++count);
      });
    }

    ++idx;
    idx = (idx % TrieFixtureType::query_size());
  }
}

} // namespace yafiyogi::benchmark
