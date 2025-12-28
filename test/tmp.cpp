











































#include<bits/stdc++.h>
using namespace std;
int main(){
    int n;
    cin>>n;
    int a[n+5];
    for(int i=1;i<=n;i++) scanf("%d", &a[i]);
    sort(a+1, a+1+n);
    for(int i=1;i<=n;i++) printf("%d ", a[i]);
    return 0;
}





































