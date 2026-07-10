/*
 * 图 —— 数据结构入门第一课
 *
 * 包含两种最常用的存储方式：
 *   1. 邻接矩阵（Adjacency Matrix）
 *   2. 邻接表（Adjacency List）
 *
 * 演示图：
 *    0 --- 1
 *    |   / |
 *    |  /  |
 *    | /   |
 *    2 --- 3 --- 4
 *
 * 5 个顶点，6 条边
 */

#include <stdio.h>
#include <stdlib.h>

/* ========================== 邻接矩阵 ========================== */

#define MAXV 100  /* 最大顶点数 */

typedef struct {
    int mat[MAXV][MAXV];  /* 邻接矩阵，mat[i][j]=1 表示有边 */
    int n;                 /* 实际顶点数 */
} GraphMatrix;

void initMatrix(GraphMatrix *g, int n) {
    g->n = n;
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++)
            g->mat[i][j] = 0;   /* 0 表示无边 */
}

void addEdgeMatrix(GraphMatrix *g, int u, int v) {
    g->mat[u][v] = 1;
    g->mat[v][u] = 1;  /* 无向图：对称 */
}

void printMatrix(GraphMatrix *g) {
    printf("邻接矩阵:\n");
    printf("  ");
    for (int i = 0; i < g->n; i++) printf("%d ", i);
    printf("\n");
    for (int i = 0; i < g->n; i++) {
        printf("%d ", i);
        for (int j = 0; j < g->n; j++)
            printf("%d ", g->mat[i][j]);
        printf("\n");
    }

    /* 空间复杂度: O(n²)，适合稠密图 */
    printf("空间: %d x %d = %d 个 int\n\n",
        g->n, g->n, g->n * g->n);
}

/* ========================== 邻接表 ========================== */

/* 边表结点 */
typedef struct EdgeNode {
    int adjvex;              /* 邻接点编号 */
    struct EdgeNode *next;   /* 下一条边 */
} EdgeNode;

/* 顶点结点 */
typedef struct {
    int data;         /* 顶点信息 */
    EdgeNode *first;  /* 第一条边 */
} VertexNode;

typedef struct {
    VertexNode vertices[MAXV];
    int n;             /* 顶点数 */
    int e;             /* 边数 */
} GraphList;

void initList(GraphList *g, int n) {
    g->n = n;
    g->e = 0;
    for (int i = 0; i < n; i++)
        g->vertices[i].first = NULL;
}

void addEdgeList(GraphList *g, int u, int v) {
    /* u -> v */
    EdgeNode *e1 = (EdgeNode *)malloc(sizeof(EdgeNode));
    e1->adjvex = v;
    e1->next = g->vertices[u].first;
    g->vertices[u].first = e1;  /* 头插 */

    /* v -> u（无向图对称） */
    EdgeNode *e2 = (EdgeNode *)malloc(sizeof(EdgeNode));
    e2->adjvex = u;
    e2->next = g->vertices[v].first;
    g->vertices[v].first = e2;

    g->e++;
}

void printList(GraphList *g) {
    printf("邻接表:\n");
    int space = 0;
    for (int i = 0; i < g->n; i++) {
        printf("  %d -> ", i);
        for (EdgeNode *p = g->vertices[i].first; p != NULL; p = p->next) {
            printf("%d ", p->adjvex);
            space++;
        }
        printf("\n");
    }
    /* 空间复杂度: O(n + 2e)，适合稀疏图 */
    printf("空间: %d 个顶点 + %d 个边结点\n\n",
        g->n, 2 * g->e);
}

/* ========================== 测试 ========================== */

int main() {
    /*
     * 构建:
     *    0 --- 1
     *    |   / |
     *    |  /  |
     *    | /   |
     *    2 --- 3 --- 4
     */
    const int edges[][2] = {
        {0,1}, {0,2}, {1,2}, {1,3}, {2,3}, {3,4}
    };
    const int edgeCount = 6;
    const int vertexCount = 5;

    /* ---------- 邻接矩阵 ---------- */
    GraphMatrix gm;
    initMatrix(&gm, vertexCount);
    for (int i = 0; i < edgeCount; i++)
        addEdgeMatrix(&gm, edges[i][0], edges[i][1]);
    printMatrix(&gm);

    /* ---------- 邻接表 ---------- */
    GraphList gl;
    initList(&gl, vertexCount);
    for (int i = 0; i < edgeCount; i++)
        addEdgeList(&gl, edges[i][0], edges[i][1]);
    printList(&gl);

    /* ---------- 对比 ---------- */
    printf("=== 对比 ===\n");
    printf("顶点数 n = %d, 边数 e = %d\n", vertexCount, edgeCount);
    printf("邻接矩阵：判断 u,v 是否有边  O(1)\n");
    printf("邻接表：  找 v 的所有邻居    O(deg(v))\n");

    return 0;
}
