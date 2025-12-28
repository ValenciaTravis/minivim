#include <multiTrie.h>

multiTrie::multiTrie(){
    size = 1;
    a.assign(2, Node());
}

/* Create a new node. If there is any node in the rubbish bin, 
   use that node instead of create a new one. */
int multiTrie::newNode(){
    if(rub.size()) {
        int ret = rub.back();
        rub.pop_back();
        return ret;
    }
    ++size;
    a.push_back(Node());
    return size;
}

/* Delete a node and clear it. Throw it into the rubbish bin. */
void multiTrie::delNode(int x){
    a[x].s = "";
    a[x].trans.clear();
    a[x].siz = a[x].cnt = 0;
    rub.push_back(x);
    return;
}

/* Insert a word into the multiTrie. */
void multiTrie::insert(const std::string &str){
    int now = 1;
    for(int i=0;i<str.size();i++){
        int &to = a[now].trans[str[i]];
        if(!to) to = newNode();
        now = to;
    }
    a[now].cnt++;
    if(a[now].cnt > 1) return; // Return if this word has already existed.
    // Increase siz by 1 from root to now
    a[now].s = str;
    now = 1; a[now].siz++;
    for(int i=0;i<str.size();i++){
        int &to = a[now].trans[str[i]];
        now = to;
        a[now].siz++;
    }
}

void multiTrie::del(const std::string &str){
    int now = 1;
    for(int i=0;i<str.size();i++){
        int &to = a[now].trans[str[i]];
        now = to;
    }
    a[now].cnt--;
    if(a[now].cnt) return; // Return if this word has already existed.
    // Decrease siz by 1 from root to now
    std::vector<int> useless_nodes;
    now = 1; a[now].siz--;
    for(int i=0;i<str.size();i++){
        int &to = a[now].trans[str[i]];
        a[to].siz--;
        if(!a[to].siz) {
            useless_nodes.push_back(to);
            a[now].trans.erase(str[i]);
        }
        now = to;
    }
    // Delete all the useless nodes.
    for(auto x : useless_nodes) delNode(x);
}

/* Return the strings starts with str and ranks from l to r,
   in lexicographical order. */
std::vector<std::string> multiTrie::findPrefix(const std::string &str, int l, int r){
    int now = 1;
    std::vector<std::string> ret;
    for(int i=0;i<str.size();i++){
        if(!a[now].trans.count(str[i])) return ret;
        now = a[now].trans[str[i]];
    }
    if(r == -1) r = a[now].siz-1;
    _dfs(now, 0, l, r, ret);
    return ret;
}

void multiTrie::_dfs(int x, int rk, int l, int r, std::vector<std::string> &v){
    if(a[x].cnt > 0){ // If this node is the end of some word
        if(l <= rk && rk <= r) v.push_back(a[x].s); // Add the word into the vector
        rk++;
    }  
    for(auto [ch, y] : a[x].trans){
        int L = rk, R = rk + a[y].siz - 1;
        // If there is any intersection between [l, r] and [L, R], then dfs to find the words.
        if((L <= l && l <= R) || (L <= l && r <= R) || (L <= r && r <= R) || (l <= L && R <= r)) 
            _dfs(y, rk, l, r, v);
        rk += a[y].siz;
    }
}