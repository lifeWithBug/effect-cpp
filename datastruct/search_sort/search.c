#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int seqSearch(int arr[], int n, int key) {
    for (int i = 0; i < n; i++)
        if (arr[i] == key) return i;
    return -1;
}

int binarySearch(int arr[], int n, int key) {
    int left = 0, right = n - 1;
    while (left <= right) {
        int mid = left + (right - left) / 2;
        if      (arr[mid] == key) return mid;
        else if (arr[mid] > key)  right = mid - 1;
        else                      left  = mid + 1;
    }
    return -1;
}

int binarySearchFirst(int arr[], int n, int key) {
    int left = 0, right = n - 1, result = -1;
    while (left <= right) {
        int mid = left + (right - left) / 2;
        if (arr[mid] == key) { result = mid; right = mid - 1; }
        else if (arr[mid] > key) right = mid - 1;
        else left = mid + 1;
    }
    return result;
}

int binarySearchLast(int arr[], int n, int key) {
    int left = 0, right = n - 1, result = -1;
    while (left <= right) {
        int mid = left + (right - left) / 2;
        if (arr[mid] == key) { result = mid; left = mid + 1; }
        else if (arr[mid] > key) right = mid - 1;
        else left = mid + 1;
    }
    return result;
}

int cmp(const void *a, const void *b) { return *(int*)a - *(int*)b; }

int main() {
    int arr[] = {2,3,5,7,11,13,17,19};
    int n = sizeof(arr)/sizeof(arr[0]);
    printf("数据: "); for(int i=0;i<n;i++) printf("%d ",arr[i]); printf("\n\n");
    printf("顺序查找 7: %d\n", seqSearch(arr,n,7));
    printf("二分查找 7: %d\n", binarySearch(arr,n,7));
    printf("查找 6: %d (不存在)\n", binarySearch(arr,n,6));

    int dup[] = {1,2,2,2,3,4,4,5};
    int m = sizeof(dup)/sizeof(dup[0]);
    printf("\n重复数据: "); for(int i=0;i<m;i++) printf("%d ",dup[i]); printf("\n");
    printf("第一个 2: %d, 最后一个 2: %d\n",
        binarySearchFirst(dup,m,2), binarySearchLast(dup,m,2));

    srand(time(NULL));
    int big[100000];
    for(int i=0;i<100000;i++) big[i]=rand()%1000000;
    qsort(big,100000,sizeof(int),cmp);
    int target = big[54321];

    clock_t a=clock(); seqSearch(big,100000,target); clock_t b=clock();
    printf("\n顺序查找 10万: %.3f ms\n", (double)(b-a)*1000/CLOCKS_PER_SEC);

    a=clock(); binarySearch(big,100000,target); b=clock();
    printf("二分查找 10万: %.3f ms\n", (double)(b-a)*1000/CLOCKS_PER_SEC);
    return 0;
}
