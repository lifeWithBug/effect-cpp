#include <stdio.h>
#include <limits.h>

#define MAXV 20
#define INF INT_MAX/2

typedef struct { int u, v, w; } Edge;

int bellmanFord(Edge edges[], int e, int n, int src, int dist[]) {
    for (int i = 0; i < n; i++) dist[i] = INF;
    dist[src] = 0;

    for (int i = 0; i < n - 1; i++) {
        int updated = 0;
        for (int j = 0; j < e; j++) {
            int u = edges[j].u, v = edges[j].v, w = edges[j].w;
            if (dist[u] != INF && dist[u] + w < dist[v]) {
                dist[v] = dist[u] + w;
                updated = 1;
            }
        }
        if (!updated) break;
    }

    for (int j = 0; j < e; j++) {
        int u = edges[j].u, v = edges[j].v, w = edges[j].w;
        if (dist[u] != INF && dist[u] + w < dist[v])
            return 1;
    }
    return 0;
}

int main() {
    printf("--- 无负环 ---\n");
    Edge e1[] = {{0,1,5},{0,2,-2},{1,2,3}};
    int d1[MAXV];
    int neg = bellmanFord(e1,3,3,0,d1);
    printf("0->1=%d 0->2=%d 负环?%s\n", d1[1], d1[2], neg?"是":"否");

    printf("\n--- 有负环 ---\n");
    Edge e2[] = {{0,1,4},{1,2,3},{2,3,-6},{3,1,2}};
    int d2[MAXV];
    neg = bellmanFord(e2,4,4,0,d2);
    printf("0->1=%d 负环?%s\n", d2[1], neg?"是":"否");

    return 0;
}
