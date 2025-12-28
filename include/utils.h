#ifndef __MY_UTILS__
#define __MY_UTILS__

#include <string>

namespace utils{
    std::string int2str(int x, int len = 3, bool show_zero = false);

    bool notSeparate(char c);

    std::pair<int, int> findCurrentWord(const std::string &, int);

    std::pair<int, int> findNxtWord(const std::string &, int);

    std::pair<int, int> findPreWord(const std::string &, int);
}

#endif