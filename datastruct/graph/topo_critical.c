#include <stdio.h>
#include <string.h>
#include <limits.h>

#define MAXV 30
#define INF INT_MAX/2

void topoSort(int g[MAXV][MAXV], int n) {
    int indeg[MAXV] = {0};
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++)
            if (g[i][j] != INF && i != j) indeg[j]++;
    int q[MAXV], f = 0, r = 0;
    for (int i = 0; i < n; i++) if (indeg[i] == 0) q[r++] = i;
    int res[MAXV], cnt = 0;
    while (f < r) {
        int u = q[f++]; res[cnt++] = u;
        for (int v = 0; v < n; v++)
            if (g[u][v] != INF && u != v && --indeg[v] == 0) q[r++] = v;
    }
    if (cnt < n) { printf("  -> 图中有环!\n"); return; }
    printf("  ");
    for (int i = 0; i < cnt; i++) printf("%d ", res[i]);
    printf("\n");
}

void criticalPath(int g[MAXV][MAXV], int n) {
    int indeg[MAXV] = {0}, topo[MAXV], cnt = 0;
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++) if (g[i][j] != INF) indeg[j]++;
    int q[MAXV], f = 0, r = 0;
    for (int i = 0; i < n; i++) if (indeg[i] == 0) q[r++] = i;
    while (f < r) {
        int u = q[f++]; topo[cnt++] = u;
        for (int v = 0; v < n; v++)
            if (g[u][v] != INF && --indeg[v] == 0) q[r++] = v;
    }
    if (cnt < n) { printf("有环\n"); return; }
    int ve[MAXV] = {0};
    for (int i = 0; i < n; i++) {
        int u = topo[i];
        for (int v = 0; v < n; v++)
            if (g[u][v] != INF && ve[u] + g[u][v] > ve[v])
                ve[v] = ve[u] + g[u][v];
    }
    int vl[MAXV], end = topo[n-1];
    for (int i = 0; i < n; i++) vl[i] = ve[end];
    for (int i = n-1; i >= 0; i--) {
        int u = topo[i];
        for (int v = 0; v < n; v++)
            if (g[u][v] != INF && vl[v] - g[u][v] < vl[u])
                vl[u] = vl[v] - g[u][v];
    }
    printf("  总工期: %d\n\n", ve[end]);
    printf("  事件:   "); for(int i=0;i<n;i++) printf("%3d ", i); printf("\n");
    printf("  最早:   "); for(int i=0;i<n;i++) printf("%3d ", ve[i]); printf("\n");
    printf("  最晚:   "); for(int i=0;i<n;i++) printf("%3d ", vl[i]); printf("\n\n");
    printf("  活动分析:\n");
    int act = 0;
    for (int u = 0; u < n; u++)
        for (int v = 0; v < n; v++)
            if (g[u][v] != INF && u != v) {
                int e = ve[u], l = vl[v] - g[u][v];
                printf("  a%d: %d->%d  e=%d l=%d 余量=%d%s\n",
                    act++, u, v, e, l, l-e, (l==e)?" **关键":"");
            }
    printf("\n  关键路径: 0");
    int cur = 0;
    while (cur != end) {
        for (int v = 0; v < n; v++)
            if (g[cur][v] != INF && vl[v] - g[cur][v] == ve[cur]) {
                printf("->%d", v);
                cur = v;
                break;
            }
    }
    printf("\n");
}

int main() {
    int g[MAXV][MAXV];
    for (int i = 0; i < MAXV; i++)
        for (int j = 0; j < MAXV; j++)
            g[i][j] = INF;

    printf("====== 拓扑排序 ======\n");
    printf("图: 0->1->3, 0->2->3, 2->5, 3->4->5, 3->6\n");
    g[0][1]=1; g[0][2]=1; g[1][3]=1; g[2][3]=1;
    g[2][5]=1; g[3][4]=1; g[3][6]=1; g[4][5]=1;
    topoSort(g, 7);

    printf("\n====== AOE 关键路径 ======\n");
    printf("图: 0-6>1-1>4-9>6-2>8  0-4>2-1>4  0-5>3-2>5-4>7-4>8\n\n");
    for (int i = 0; i < MAXV; i++)
        for (int j = 0; j < MAXV; j++)
            g[i][j] = INF;
    g[0][1]=6; g[0][2]=4; g[0][3]=5;
    g[1][4]=1;
    g[2][4]=1;
    g[3][5]=2;
    g[4][6]=9; g[4][7]=7;
    g[5][7]=4;
    g[6][8]=2;
    g[7][8]=4;
    criticalPath(g, 9);
    return 0;
}
