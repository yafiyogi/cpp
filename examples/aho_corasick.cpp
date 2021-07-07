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

#include "yy_aho_corasick.h"
#include <string>
#include <iostream>

using namespace yafiyogi;

int main()
{

  auto trie = yafiyogi::trie<std::string, std::string>{};


  trie.add( "his", "his");
  trie.add( "he", "he");
  trie.add( "she", "she");
  trie.add( "hers", "hers");

  trie.compile();

  std::string msg = "123he456thehisshes0he";

  auto bot = trie.create_automaton();

  for(auto ch: msg)
  {
    bot.next(ch);

    if( !bot.empty())
    {
      bot.visit([](auto value)
      {
        std::cout << value << std::endl;
      });
    }
  }



  return 0;
}
