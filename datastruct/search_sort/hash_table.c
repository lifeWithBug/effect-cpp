#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ================= 链地址法（Separate Chaining） ================= */
#define TABLE_SIZE 10

typedef struct Entry {
    int key;
    int value;
    struct Entry *next;
} Entry;

typedef struct {
    Entry **buckets;  /* 指针数组，每个元素是链表头 */
    int size;
} HashChain;

HashChain *createChain(int size) {
    HashChain *h = (HashChain *)malloc(sizeof(HashChain));
    h->size = size;
    h->buckets = (Entry **)calloc(size, sizeof(Entry *));
    return h;
}

int hash(int key, int size) { return abs(key) % size; }

void putChain(HashChain *h, int key, int val) {
    int idx = hash(key, h->size);
    /* 查找是否已存在 */
    for (Entry *p = h->buckets[idx]; p; p = p->next)
        if (p->key == key) { p->value = val; return; }
    /* 不存在 → 头插新结点 */
    Entry *e = (Entry *)malloc(sizeof(Entry));
    e->key = key; e->value = val;
    e->next = h->buckets[idx];
    h->buckets[idx] = e;
}

int getChain(HashChain *h, int key, int *found) {
    int idx = hash(key, h->size);
    for (Entry *p = h->buckets[idx]; p; p = p->next)
        if (p->key == key) { *found = 1; return p->value; }
    *found = 0; return -1;
}

void delChain(HashChain *h, int key) {
    int idx = hash(key, h->size);
    Entry *cur = h->buckets[idx], *prev = NULL;
    while (cur) {
        if (cur->key == key) {
            if (prev) prev->next = cur->next;
            else h->buckets[idx] = cur->next;
            free(cur);
            return;
        }
        prev = cur;
        cur = cur->next;
    }
}

void printChain(HashChain *h) {
    printf("链地址法:\n");
    for (int i = 0; i < h->size; i++) {
        printf("  [%d]:", i);
        for (Entry *p = h->buckets[i]; p; p = p->next)
            printf(" (%d,%d)", p->key, p->value);
        printf("\n");
    }
}

/* ================= 开放地址法（线性探测） ================= */

typedef struct {
    int *keys;
    int *values;
    int *used;     /* 1=已占用, 0=空 */
    int size;
    int count;
} HashOpen;

HashOpen *createOpen(int size) {
    HashOpen *h = (HashOpen *)malloc(sizeof(HashOpen));
    h->size = size;
    h->count = 0;
    h->keys   = (int *)calloc(size, sizeof(int));
    h->values = (int *)calloc(size, sizeof(int));
    h->used   = (int *)calloc(size, sizeof(int));
    return h;
}

void putOpen(HashOpen *h, int key, int val) {
    int idx = hash(key, h->size);
    int start = idx;
    /* 线性探测：遇到空位就插入，遇到相同 key 就更新 */
    while (h->used[idx]) {
        if (h->keys[idx] == key) { h->values[idx] = val; return; }
        idx = (idx + 1) % h->size;
        if (idx == start) return; /* 表满了 */
    }
    h->keys[idx] = key;
    h->values[idx] = val;
    h->used[idx] = 1;
    h->count++;
}

int getOpen(HashOpen *h, int key, int *found) {
    int idx = hash(key, h->size);
    int start = idx;
    while (h->used[idx]) {
        if (h->keys[idx] == key) { *found = 1; return h->values[idx]; }
        idx = (idx + 1) % h->size;
        if (idx == start) break;
    }
    *found = 0; return -1;
}

void printOpen(HashOpen *h) {
    printf("线性探测法:\n");
    for (int i = 0; i < h->size; i++) {
        if (h->used[i])
            printf("  [%d]: key=%d val=%d\n", i, h->keys[i], h->values[i]);
        else
            printf("  [%d]: (空)\n", i);
    }
    printf("  负载因子: %.2f (%d/%d)\n\n",
        (float)h->count / h->size, h->count, h->size);
}

/* ========================== 测试 ========================== */

int main() {
    printf("====== 链地址法 ======\n");
    HashChain *hc = createChain(TABLE_SIZE);
    putChain(hc, 15, 100);  /* hash=15%10=5 */
    putChain(hc, 25, 200);  /* hash=25%10=5 */
    putChain(hc, 35, 300);  /* hash=35%10=5 - 三个 key 冲突在同一个桶 */
    putChain(hc, 7, 400);
    putChain(hc, 3, 500);
    printChain(hc);

    int found, val;
    val = getChain(hc, 25, &found);
    printf("key=25 → %d (found=%d)\n", val, found);
    val = getChain(hc, 99, &found);
    printf("key=99 → %d (found=%d)\n\n", val, found);

    printf("删除 key=25\n");
    delChain(hc, 25);
    printChain(hc);

    printf("\n====== 开放地址法 ======\n");
    HashOpen *ho = createOpen(TABLE_SIZE);
    putOpen(ho, 15, 100);  /* idx=5 */
    putOpen(ho, 25, 200);  /* idx=5 → 冲突 → idx=6 */
    putOpen(ho, 35, 300);  /* idx=5 → 冲突 → idx=6 → idx=7 */
    putOpen(ho, 7, 400);
    putOpen(ho, 3, 500);
    printOpen(ho);

    val = getOpen(ho, 25, &found);
    printf("key=25 → %d (found=%d)\n", val, found);
    val = getOpen(ho, 99, &found);
    printf("key=99 → %d (found=%d)\n", val, found);

    return 0;
}
