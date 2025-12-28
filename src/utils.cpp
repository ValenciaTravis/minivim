#include <utils.h>
#include <cmath>

/* Translate a int into string. x represents the number,
   len represents the minimum length of the number, show_zero
   represents whether to put a '0' at the end of the string
   when x = 0. */
std::string utils::int2str(int x, int len, bool show_zero){
    std::string str;
    bool flag = false;
    int i = pow(10, len);
    while(i <= x / 10) i *= 10;
    for(;i;i/=10){
        int ch = x / i;
        x %= i;
        flag = flag || (ch > 0);
        if(ch != 0) {str += '0' + ch; continue;}
        if(flag || (i == 1 && show_zero)) str += '0';
        else str += ' ';
    }
    return str;
}

/* Return whether this char is a separate char. */
bool utils::notSeparate(char c){
    return isalpha(c) || isdigit(c) || c == '_';
}

/* Return a pair<int, int> represents the start position
   and the length of the current word. */
std::pair<int, int> utils::findCurrentWord(const std::string &str, int pos){
    if(pos < 0 || pos == str.size() || !notSeparate(str[pos])) return std::make_pair(pos, 0);
    int p1 = pos, p2 = pos;
    while(p1 > 0 && notSeparate(str[p1-1])) --p1;
    while(p2+1 < str.size() && notSeparate(str[p2+1])) ++p2;
    return std::make_pair(p1, p2-p1+1);
}

/* Return a pair<int, int> represents the the start position
   and the length of the next word after the pos(included). */
std::pair<int, int> utils::findNxtWord(const std::string &str, int pos){
    if(pos < 0 || pos > str.size()) return std::make_pair(pos, 0);
    for(int i=pos;i<str.size();i++) if(notSeparate(str[i])) return findCurrentWord(str, i);
    return std::make_pair(pos, 0);
}

/* Return a pair<int, int> represents the the start position
   and the length of the previous word before the pos(included). */
std::pair<int, int> utils::findPreWord(const std::string &str, int pos){
    if(pos < 0 || pos > str.size()) return std::make_pair(pos, 0);
    for(int i=pos;i>=0;i--) if(notSeparate(str[i])) return findCurrentWord(str, i);
    return std::make_pair(pos, 0);
}