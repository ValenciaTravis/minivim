#include<bits/stdc++.h>
using namespace std;
#define MAXN 100005
#define MAXM 500005
#define ll long long
int n, m, col[MAXN], X[MAXN], tt;
unordered_map<int, int> mp;
vector<int> e[MAXN];
int f[MAXN][18], lg[MAXN], dep[MAXN], siz[MAXN], id[MAXN], rev[MAXN], dfn;
void push(int x, int y){
    e[x].push_back(y);
    e[y].push_back(x);
}
void dfs(int x, int fa){
    f[x][0] = fa;
    id[x] = ++dfn;
    dep[x] = dep[fa] + 1;
    siz[x] = 1;
    rev[dfn] = x;
    for(int i=1;i<lg[dep[x]];i++) f[x][i] = f[f[x][i-1]][i-1];
    for(auto v : e[x]){
        if(v == fa) continue;
        dfs(v, x);
        siz[x] += siz[v];
    }
}
int lca(int x, int y){
    if(dep[x] < dep[y]) swap(x, y);
    while(dep[x] != dep[y]) x = f[x][lg[dep[x]-dep[y]]-1];
    if(x == y) return x;
    for(int i=lg[dep[x]]-1;i>=0;i--) if(f[x][i] != f[y][i]) x = f[x][i], y = f[y][i];
    return f[x][0];
}
int kth(int x, int k){
    for(int i=0;i<lg[dep[x]];i++) {
        if((1 << i) & k) x = f[x][i];
    }
    return x;
}
vector<pair<int, int>> v1, v2;
void getV(int x, int rt, vector<pair<int, int>>& v){
    int f = lca(x, rt);
    // printf("f = %d\n", f);
    if(x == rt) {v.push_back({1, n}); return;}
    if(f == rt) {v.push_back({id[x], id[x]+siz[x]-1}); return;}
    if(f != x) {v.push_back({id[x], id[x]+siz[x]-1}); return;}
    int tmp = kth(rt, dep[rt]-dep[x]-1);
    v.push_back({1, id[tmp]-1});
    if(id[tmp] + siz[tmp] <= n) v.push_back({id[tmp]+siz[tmp], n});
}

int bsiz, bcnt, belong[MAXN];
struct Query{
    int l, r, id;
    Query() {l = r = id = 0;}
    Query(int a, int b, int c) {if(a > b) swap(a, b); l = a, r = b, id = c;}
} Q[MAXM<<2];
bool cmp(const Query& a, const Query& b) {
	return (belong[a.l] ^ belong[b.l]) ? belong[a.l] < belong[b.l] : ((belong[a.l] & 1) ? a.r < b.r : a.r > b.r);
}
void init(int n, int m){
    m = max(m, 1);
    bsiz = n / sqrt(m);
    bcnt = ceil(double(n) / bsiz);
    for(int i=1;i<=bcnt;i++) for(int j=(i-1)*bsiz+1;j<=n&&j<=i*bsiz;j++) belong[j] = i;
}
ll tot = 0, pre[MAXN], ans[MAXM], now;
ll cntl[MAXN], cntr[MAXN];
void addQuery(int l1, int r1, int l2, int r2, int id){
    // printf("f(%d, %d, %d, %d)\n", l1, r1, l2, r2);
    if(r1 == n && r2 == n){
        if(l1 > 1 && l2 > 1) Q[++tot] = Query(l1-1, l2-1, id);
        ans[id] += pre[n] - pre[l1-1] - pre[l2-1];
    }else if(r1 == n || r2 == n){
        if(r1 == n) swap(l1, l2), swap(r1, r2);
        ans[id] += pre[r1] - pre[l1-1];
        if(l1 > 1 && l2 > 1) Q[++tot] = Query(l1-1, l2-1, id);
        if(l2 > 1) Q[++tot] = Query(r1, l2-1, -id);
    }else{
        Q[++tot] = Query(r1, r2, id);
        if(l1 > 1) Q[++tot] = Query(l1-1, r2, -id);
        if(l2 > 1) Q[++tot] = Query(r1, l2-1, -id);
        if(l1 > 1 && l2 > 1)Q[++tot] = Query(l1-1, l2-1, id);
    }
}

void addl(int x){
    int c = col[x];
    now += cntr[c];
    ++cntl[c];
}
void dell(int x){
    int c = col[x];
    now -= cntr[c];
    --cntl[c];
}
void addr(int x){
    int c = col[x];
    now += cntl[c];
    ++cntr[c];
}
void delr(int x){
    int c = col[x];
    now -= cntl[c];
    --cntr[c];
}

int rd(){
    int ret = 0;
    char c = getchar();
    while(!isdigit(c)) c = getchar();
    while(isdigit(c)) ret = (ret<<1) + (ret<<3) + c - '0', c = getchar(); 
    return ret;
}

int main(){
    cin>>n>>m;
    init(n, m);
    for(int i=1;i<=n;i++) col[i] = rd(), X[i] = col[i];
    sort(X+1, X+1+n);
    tt = unique(X+1, X+1+n) - X - 1;
    for(int i=1;i<=tt;i++) mp[X[i]] = i;
    for(int i=1;i<=n;i++) col[i] = mp[col[i]];
    for(int i=1;i<=n;i++) lg[i] = lg[i-1] + ((1<<lg[i-1]) == i);
    int x, y;
    for(int i=1;i<n;i++) x = rd(), y = rd(), push(x, y);
    dfs(1, 0);

    for(int i=1;i<=n;i++) addr(rev[i]);
    for(int i=1;i<=n;i++) addl(rev[i]), pre[i] = now;
    for(int i=1;i<=n;i++) dell(rev[i]), delr(rev[i]);

    // printf("now = %lld\n", now);

    int rt = 1, op;
    for(int i=1;i<=m;i++){
        op = rd(), x = rd();
        if(op == 1) {rt = x; ans[i] = -1; continue;}
        y = rd();
        v1.clear(), v2.clear();
        getV(x, rt, v1);
        getV(y, rt, v2);
        for(auto& [l1, r1] : v1) for(auto& [l2, r2] : v2) addQuery(l1, r1, l2, r2, i);
    }
    sort(Q+1, Q+1+tot, cmp);
    int l = 0, r = 0;
    for(int i = 1; i <= tot; ++i) {
		int ql = Q[i].l, qr = Q[i].r;
		while(l < ql) addl(rev[++l]);
        while(r < qr) addr(rev[++r]);
        while(l > ql) dell(rev[l--]);
        while(r > qr) delr(rev[r--]);
        if(Q[i].id > 0) ans[Q[i].id] += now;
        else ans[-Q[i].id] -= now;
	}
    for(int i=1;i<=m;i++) if(ans[i] != -1) printf("%lld\n", ans[i]);
    return 0;
}

/*
7 2
1 1 1 1 1 1 1
1 2
1 3
2 4
2 5
3 6
3 7
1 4
2 1 2
*/