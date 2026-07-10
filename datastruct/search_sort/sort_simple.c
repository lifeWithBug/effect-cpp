#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

/* =================== 冒泡排序 =================== */

/* 两两比较，大的往后冒。每轮至少确定一个最大值在末尾。 */
void bubbleSort(int arr[], int n) {
    for (int i = 0; i < n - 1; i++) {
        int swapped = 0;
        for (int j = 0; j < n - 1 - i; j++) {
            if (arr[j] > arr[j + 1]) {
                int t = arr[j]; arr[j] = arr[j + 1]; arr[j + 1] = t;
                swapped = 1;
            }
        }
        if (!swapped) break;  /* 没有交换 → 已经有序 */
    }
}

/* =================== 选择排序 =================== */

/* 每轮找剩余部分的最小值，放到已排序部分的末尾。 */
void selectSort(int arr[], int n) {
    for (int i = 0; i < n - 1; i++) {
        int minIdx = i;
        for (int j = i + 1; j < n; j++)
            if (arr[j] < arr[minIdx]) minIdx = j;
        if (minIdx != i) {
            int t = arr[i]; arr[i] = arr[minIdx]; arr[minIdx] = t;
        }
    }
}

/* =================== 插入排序 =================== */

/* 像打牌时整理手牌：每次把一张牌插入到已排序序列的正确位置。 */
void insertSort(int arr[], int n) {
    for (int i = 1; i < n; i++) {
        int key = arr[i];
        int j = i - 1;
        while (j >= 0 && arr[j] > key) {
            arr[j + 1] = arr[j];
            j--;
        }
        arr[j + 1] = key;
    }
}

/* =================== 打印 + 测试 =================== */

void printArray(int arr[], int n, char *label) {
    printf("%s: ", label);
    for (int i = 0; i < n; i++) printf("%d ", arr[i]);
    printf("\n");
}

void copyArray(int src[], int dst[], int n) {
    memcpy(dst, src, n * sizeof(int));
}

int cmp(const void *a, const void *b) { return *(int*)a - *(int*)b; }

int main() {
    int raw[] = {64, 34, 25, 12, 22, 11, 90, 8};
    int n = sizeof(raw) / sizeof(raw[0]);
    int arr[n];

    printf("====== 三种简单排序演示 ======\n\n");
    printf("原始: "); for (int i = 0; i < n; i++) printf("%d ", raw[i]);
    printf("\n\n");

    copyArray(raw, arr, n); bubbleSort(arr, n);
    printArray(arr, n, "冒泡排序");

    copyArray(raw, arr, n); selectSort(arr, n);
    printArray(arr, n, "选择排序");

    copyArray(raw, arr, n); insertSort(arr, n);
    printArray(arr, n, "插入排序");

    /* ---------- 性能对比 ---------- */
    printf("\n====== 性能对比 (10000 个随机数) ======\n");
    int big[10000], a1[10000], a2[10000], a3[10000];
    srand(time(NULL));
    for (int i = 0; i < 10000; i++) big[i] = rand() % 100000;

    copyArray(big, a1, 10000);
    copyArray(big, a2, 10000);
    copyArray(big, a3, 10000);

    clock_t t;
    t = clock(); bubbleSort(a1, 10000); t = clock() - t;
    printf("冒泡排序: %.2f ms\n", t * 1000.0 / CLOCKS_PER_SEC);

    t = clock(); selectSort(a2, 10000); t = clock() - t;
    printf("选择排序: %.2f ms\n", t * 1000.0 / CLOCKS_PER_SEC);

    t = clock(); insertSort(a3, 10000); t = clock() - t;
    printf("插入排序: %.2f ms\n", t * 1000.0 / CLOCKS_PER_SEC);

    /* 验证有序 */
    qsort(big, 10000, sizeof(int), cmp);
    printf("\n正确性验证: ");
    printf("%s ", memcmp(a1, big, 10000*sizeof(int))==0?"冒泡OK":"冒泡NG");
    printf("%s ", memcmp(a2, big, 10000*sizeof(int))==0?"选择OK":"选择NG");
    printf("%s\n", memcmp(a3, big, 10000*sizeof(int))==0?"插入OK":"插入NG");

    return 0;
}
