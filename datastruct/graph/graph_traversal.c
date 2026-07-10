/*
 * 图的两种遍历：DFS 与 BFS
 *
 * 演示图（邻接表表示）：
 *    0 --- 1 --- 2
 *    |     |     |
 *    |     |     |
 *    3 --- 4 --- 5
 *          |
 *          6
 *
 * DFS(0): 0 1 2 5 4 6 3 （取决于邻接表顺序）
 * BFS(0): 0 1 3 2 4 5 6
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAXV 100

/* ========================== 邻接表 ========================== */

typedef struct EdgeNode {
    int adjvex;
    struct EdgeNode *next;
} EdgeNode;

typedef struct {
    EdgeNode *first;
    int data;
} VertexNode;

typedef struct {
    VertexNode vertices[MAXV];
    int n, e;
} Graph;

void initGraph(Graph *g, int n) {
    g->n = n;
    g->e = 0;
    for (int i = 0; i < n; i++)
        g->vertices[i].first = NULL;
}

void addEdge(Graph *g, int u, int v) {
    EdgeNode *e = (EdgeNode *)malloc(sizeof(EdgeNode));
    e->adjvex = v;
    e->next = g->vertices[u].first;
    g->vertices[u].first = e;
    /* 无向图对称 */
    e = (EdgeNode *)malloc(sizeof(EdgeNode));
    e->adjvex = u;
    e->next = g->vertices[v].first;
    g->vertices[v].first = e;
    g->e++;
}

/* ========================== DFS ========================== */

void dfsRecur(Graph *g, int v, int visited[]) {
    visited[v] = 1;
    printf("%d ", v);

    for (EdgeNode *p = g->vertices[v].first; p != NULL; p = p->next)
        if (!visited[p->adjvex])
            dfsRecur(g, p->adjvex, visited);
}

void DFS(Graph *g, int start) {
    int visited[MAXV] = {0};
    printf("DFS(%d): ", start);
    dfsRecur(g, start, visited);
    printf("\n");
}

/* ========================== BFS ========================== */

typedef struct { int data[MAXV]; int front, rear; } Queue;
void initQueue(Queue *q) { q->front = q->rear = 0; }
void enQueue(Queue *q, int v) { q->data[q->rear++] = v; }
int deQueue(Queue *q) { return q->data[q->front++]; }
int queueEmpty(Queue *q) { return q->front == q->rear; }

void BFS(Graph *g, int start) {
    int visited[MAXV] = {0};
    Queue q;
    initQueue(&q);

    visited[start] = 1;
    enQueue(&q, start);

    printf("BFS(%d): ", start);
    while (!queueEmpty(&q)) {
        int v = deQueue(&q);
        printf("%d ", v);

        for (EdgeNode *p = g->vertices[v].first; p != NULL; p = p->next) {
            if (!visited[p->adjvex]) {
                visited[p->adjvex] = 1;
                enQueue(&q, p->adjvex);
            }
        }
    }
    printf("\n");
}

/* ====================== 连通分量 ====================== */

int countComponents(Graph *g) {
    int visited[MAXV] = {0};
    int count = 0;

    for (int i = 0; i < g->n; i++) {
        if (!visited[i]) {
            count++;
            printf("  连通分量 %d: ", count);
            dfsRecur(g, i, visited);
            printf("\n");
        }
    }
    return count;
}

/* ====================== 环检测 ====================== */

/* 用 DFS 检测无向图是否有环 */
int hasCycleDFS(Graph *g, int v, int parent, int visited[]) {
    visited[v] = 1;
    for (EdgeNode *p = g->vertices[v].first; p != NULL; p = p->next) {
        if (!visited[p->adjvex]) {
            if (hasCycleDFS(g, p->adjvex, v, visited))
                return 1;
        } else if (p->adjvex != parent) {
            return 1;  /* 发现了"回边" */
        }
    }
    return 0;
}

int hasCycle(Graph *g) {
    int visited[MAXV] = {0};
    for (int i = 0; i < g->n; i++)
        if (!visited[i])
            if (hasCycleDFS(g, i, -1, visited))
                return 1;
    return 0;
}

/* ========================== 测试 ========================== */

int main() {
    Graph g;
    initGraph(&g, 7);

    int edges[][2] = {
        {0,1}, {0,3}, {1,2}, {1,4},
        {2,5}, {3,4}, {4,5}, {4,6}
    };
    int ecnt = 8;
    for (int i = 0; i < ecnt; i++)
        addEdge(&g, edges[i][0], edges[i][1]);

    DFS(&g, 0);
    BFS(&g, 0);

    printf("\n有环: %s\n", hasCycle(&g) ? "是" : "否");

    /* ---------- 测试连通分量 ---------- */
    Graph g2;
    initGraph(&g2, 6);
    addEdge(&g2, 0, 1); addEdge(&g2, 2, 3); addEdge(&g2, 4, 5);
    printf("\n非连通图的分量:\n");
    int cc = countComponents(&g2);
    printf("  共 %d 个连通分量\n", cc);

    /* ---------- 测试环检测（树） ---------- */
    Graph tree;
    initGraph(&tree, 4);
    addEdge(&tree, 0, 1); addEdge(&tree, 1, 2); addEdge(&tree, 2, 3);
    printf("\n树有环: %s\n", hasCycle(&tree) ? "是" : "否");

    /* ---------- 测试环检测（带环） ---------- */
    Graph cycleG;
    initGraph(&cycleG, 4);
    addEdge(&cycleG, 0, 1); addEdge(&cycleG, 1, 2);
    addEdge(&cycleG, 2, 3); addEdge(&cycleG, 3, 1);
    printf("带环图有环: %s\n", hasCycle(&cycleG) ? "是" : "否");

    return 0;
}
