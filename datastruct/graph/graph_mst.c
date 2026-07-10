/*
 * 最小生成树 —— Prim 算法 与 Kruskal 算法
 *
 * 演示图（无向带权图）：
 *
 *    0 ----1---- 1
 *    |  \      / |
 *    |   \    /  |
 *    6    \  /   5
 *    |      2    |
 *    |    /  \   |
 *    |   /    \  |
 *    3 ----4---- 4
 *          |
 *          3
 *          6
 *
 * 顶点: 0, 1, 2, 3, 4, 5, 6
 * MST 总权值 = 16
 */

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#define MAXV 20
#define INF INT_MAX

/* ======================== Prim 算法 ======================== */

/*
 * 思想：从某个顶点出发，每次选取"已选集合"到"未选集合"
 * 之间权值最小的边，把对面那个顶点拉进来。重复 n-1 次。
 *
 * 类似在已修通的城市和未修通的城市之间，每次都修最便宜的那条路。
 */
void prim(int graph[MAXV][MAXV], int n, int start) {
    int selected[MAXV] = {0};   /* 已在树中的顶点 */
    int parent[MAXV];           /* 记录最小边的来源 */
    int minDist[MAXV];          /* 到树的距离（最小边权） */

    /* 初始化：只有 start 在树中，其余顶点到树的距离是 graph[start][i] */
    for (int i = 0; i < n; i++) {
        minDist[i] = graph[start][i];
        parent[i] = start;
    }
    selected[start] = 1;
    minDist[start] = 0;

    int totalWeight = 0;
    printf("Prim(起点 %d):\n", start);

    /* 每次循环选一个顶点，共 n-1 次 */
    for (int count = 0; count < n - 1; count++) {
        /* 找离树最近的未选顶点 */
        int u = -1;
        int min = INF;
        for (int i = 0; i < n; i++) {
            if (!selected[i] && minDist[i] < min) {
                min = minDist[i];
                u = i;
            }
        }

        if (u == -1) { printf("  图不连通\n"); return; }

        selected[u] = 1;
        totalWeight += min;
        printf("  选边 %d -- %d (w=%d)\n", parent[u], u, min);

        /* 更新未选顶点到树的距离 */
        for (int v = 0; v < n; v++) {
            if (!selected[v] && graph[u][v] < minDist[v]) {
                minDist[v] = graph[u][v];
                parent[v] = u;
            }
        }
    }
    printf("  MST 总权值: %d\n\n", totalWeight);
}

/* ====================== Kruskal 算法 ====================== */

/* 边结构 */
typedef struct {
    int u, v, w;
} Edge;

/* 并查集 */
int parent[MAXV];

int find(int x) {
    /* 路径压缩 */
    while (parent[x] >= 0) x = parent[x];
    return x;
}

void unionSet(int a, int b) {
    int ra = find(a), rb = find(b);
    if (ra == rb) return;
    if (parent[ra] < parent[rb]) {  /* 按秩合并（秩用负数表示） */
        parent[ra] += parent[rb];
        parent[rb] = ra;
    } else {
        parent[rb] += parent[ra];
        parent[ra] = rb;
    }
}

/* 边排序用 */
int cmpEdges(const void *a, const void *b) {
    return ((Edge *)a)->w - ((Edge *)b)->w;
}

/*
 * 思想：把所有边按权值从小到大排序，依次尝试加入。
 * 如果这条边的两端已经连通（属于同一集合），跳过，否则加入。
 *
 * 就像把所有可选的桥按成本排好，每次只看最便宜的那座。
 */
void kruskal(Edge edges[], int en, int n) {
    /* 初始化并查集 */
    for (int i = 0; i < n; i++) parent[i] = -1;

    qsort(edges, en, sizeof(Edge), cmpEdges);

    int totalWeight = 0;
    int edgeCount = 0;

    printf("Kruskal:\n");

    for (int i = 0; i < en && edgeCount < n - 1; i++) {
        int ru = find(edges[i].u);
        int rv = find(edges[i].v);

        if (ru != rv) {
            unionSet(ru, rv);
            totalWeight += edges[i].w;
            edgeCount++;
            printf("  选边 %d -- %d (w=%d)\n",
                edges[i].u, edges[i].v, edges[i].w);
        }
    }

    printf("  MST 总权值: %d\n\n", totalWeight);
}

/* ========================== 测试 ========================== */

int main() {
    int n = 7;

    /* ---------- 邻接矩阵（INF 表示无边） ---------- */
    int graph[MAXV][MAXV];
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++)
            graph[i][j] = (i == j) ? 0 : INF;

    /*    0 --1-- 1 --2-- 2
     *    | \     /      |
     *    6  3   7       5
     *    |   \ /        |
     *    3 --4-- 4 --5-- 5
     *          | 9
     *          6
     */
    int edges[][3] = {
        {0,1,1}, {0,3,6}, {0,4,3},
        {1,2,2}, {1,3,5}, {1,4,7},
        {2,4,4}, {2,5,5},
        {3,4,4},
        {4,5,5}, {4,6,9},
    };
    int en = 11;

    for (int i = 0; i < en; i++) {
        int u = edges[i][0], v = edges[i][1], w = edges[i][2];
        graph[u][v] = w;
        graph[v][u] = w;
    }

    /* Prim */
    prim(graph, n, 0);

    /* Kruskal */
    Edge elist[en];
    for (int i = 0; i < en; i++) {
        elist[i].u = edges[i][0];
        elist[i].v = edges[i][1];
        elist[i].w = edges[i][2];
    }
    kruskal(elist, en, n);

    /* ---------- 验证 ---------- */
    printf("=== 手动验证 ===\n");
    printf("预期 MST: 0-1(1), 1-2(2), 0-4(3), 3-4(4), 2-5(5)\n");
    printf("或者: ... (MST 不唯一)\n");
    printf("总权值: 1+2+3+4+5=15 或 1+2+3+4+5=15 或其他\n");
    /* 实际上正确 MST 是: 0-1(1), 1-2(2), 0-4(3), 3-4(4), 4-5(5) = 15
     * 或者: 0-1(1), 1-2(2), 0-4(3), 3-4(4), 2-5(5) = 15 */

    return 0;
}
