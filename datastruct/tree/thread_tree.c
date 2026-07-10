#include <stdio.h>
#include <stdlib.h>

typedef struct ThreadNode {
    int data;
    struct ThreadNode *left;
    struct ThreadNode *right;
    int ltag;
    int rtag;
} ThreadNode;

ThreadNode *newNode(int data) {
    ThreadNode *node = (ThreadNode *)malloc(sizeof(ThreadNode));
    node->data  = data;
    node->left  = NULL;
    node->right = NULL;
    node->ltag  = 0;
    node->rtag  = 0;
    return node;
}

void inThread(ThreadNode *cur, ThreadNode **pre) {
    if (cur == NULL) return;
    inThread(cur->left, pre);
    if (cur->left == NULL) { cur->left = *pre; cur->ltag = 1; }
    if (*pre != NULL && (*pre)->right == NULL) { (*pre)->right = cur; (*pre)->rtag = 1; }
    *pre = cur;
    inThread(cur->right, pre);
}

ThreadNode *firstNode(ThreadNode *node) {
    if (node == NULL) return NULL;
    while (node->ltag == 0) node = node->left;
    return node;
}

ThreadNode *lastNode(ThreadNode *node) {
    if (node == NULL) return NULL;
    while (node->rtag == 0) node = node->right;
    return node;
}

ThreadNode *nextNode(ThreadNode *node) {
    if (node->rtag == 1) return node->right;
    return firstNode(node->right);
}

ThreadNode *prevNode(ThreadNode *node) {
    if (node->ltag == 1) return node->left;
    ThreadNode *p = node->left;
    while (p->rtag == 0) p = p->right;
    return p;
}

int main() {
    ThreadNode *root = newNode(1);
    root->left  = newNode(2);
    root->right = newNode(3);
    root->left->left   = newNode(4);
    root->left->right  = newNode(5);
    root->right->right = newNode(6);

    ThreadNode *pre = NULL;
    inThread(root, &pre);

    printf("forward: ");
    ThreadNode *p;
    for (p = firstNode(root); p != NULL; p = nextNode(p))
        printf("%d ", p->data);
    printf("\n");

    printf("reverse: ");
    for (p = lastNode(root); p != NULL; p = prevNode(p))
        printf("%d ", p->data);
    printf("\n");

    printf("succ of 2: %d\n", nextNode(root->left)->data);
    printf("pred of 3: %d\n", prevNode(root->right)->data);

    return 0;
}
