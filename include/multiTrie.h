#ifndef __MULTI_TRIE__
#define __MULTI_TRIE__

#include <vector>
#include <string>
#include <map>

class multiTrie{
public:

    multiTrie();
    int newNode();
    void delNode(int x);
    void insert(const std::string &str);
    void del(const std::string &str);
    std::vector<std::string> findPrefix(const std::string &str, int l=0, int r=-1);

private:
    void _dfs(int, int, int, int, std::vector<std::string> &);
    struct Node{
        std::string s;
        std::map<char, int> trans;
        int siz, cnt;
        Node() {s = ""; siz = cnt = 0;}
    };
    std::vector<Node> a;
    int size;
    std::vector<int> rub;
};

#endif