/**
 * 二叉树的非递归（迭代）遍历
 *   前序（Pre-order）：根 → 左 → 右
 *   中序（In-order） ：左 → 根 → 右
 *   后序（Post-order）：左 → 右 → 根
 *
 * 非递归实现的核心思路：用手动栈模拟递归时的系统调用栈。
 */

#include <stdio.h>
#include <stdlib.h>

#define OK    1
#define ERROR 0
#define MAX   100

typedef int Status;
typedef int ElemType;

/* ---------- 二叉树结点定义 ---------- */
typedef struct TreeNode {
    ElemType data;
    struct TreeNode *left;
    struct TreeNode *right;
} TreeNode, *BiTree;

/* ---------- 顺序栈（用于非递归遍历）---------- */
typedef struct {
    TreeNode* data[MAX];   // 栈中存的是结点指针
    int top;               // 栈顶下标（-1 表示空）
} Stack;

Status StackInit(Stack *s)   { s->top = -1;               return OK; }
Status StackEmpty(Stack *s)  { return s->top == -1;        }
Status StackPush(Stack *s, TreeNode *p) {
    if (s->top == MAX - 1) return ERROR;
    s->data[++(s->top)] = p;
    return OK;
}
Status StackPop(Stack *s, TreeNode **p) {
    if (s->top == -1) return ERROR;
    *p = s->data[(s->top)--];
    return OK;
}
Status StackTop(Stack *s, TreeNode **p) {
    if (s->top == -1) return ERROR;
    *p = s->data[s->top];
    return OK;
}

/* ================================================================
   辅助：创建新结点、手动构造一棵示例二叉树
   ================================================================ */
TreeNode* NewNode(ElemType x) {
    TreeNode *p = (TreeNode*)malloc(sizeof(TreeNode));
    p->data  = x;
    p->left  = NULL;
    p->right = NULL;
    return p;
}

/**
 * 构造下面的二叉树（方便验证输出）：
 *
 *        1
 *       / \
 *      2   3
 *     / \ / \
 *    4  5 6  7
 *
 * 前序期望：1 2 4 5 3 6 7
 * 中序期望：4 2 5 1 6 3 7
 * 后序期望：4 5 2 6 7 3 1
 */
BiTree BuildDemoTree() {
    TreeNode *r = NewNode(1);
    r->left  = NewNode(2);
    r->right = NewNode(3);
    r->left->left   = NewNode(4);
    r->left->right  = NewNode(5);
    r->right->left  = NewNode(6);
    r->right->right = NewNode(7);
    return r;
}

/*
 * 二叉树的 前序 / 中序 / 后序 遍历
 * 每种遍历分别给出递归和非递归（迭代）实现
 *
 * 演示用树的结构：
 *        1
 *       / \
 *      2   3
 *     / \   \
 *    4   5   6
 *
 * 前序: 1 2 4 5 3 6
 * 中序: 4 2 5 1 3 6
 * 后序: 4 5 2 6 3 1
 */

#include <stdio.h>
#include <stdlib.h>

/* ========================== 结点定义 ========================== */

typedef struct TreeNode {
    int data;
    struct TreeNode *left;
    struct TreeNode *right;
} TreeNode;

/* ========================== 辅助函数 ========================== */

TreeNode *newNode(int data) {
    TreeNode *node = (TreeNode *)malloc(sizeof(TreeNode));
    node->data = data;
    node->left = NULL;
    node->right = NULL;
    return node;
}

void visit(TreeNode *node) {
    printf("%d ", node->data);
}

/* ====================== 递归实现 ====================== */

/* ---------- 前序: 根 -> 左 -> 右 ---------- */
void preOrderRecur(TreeNode *root) {
    if (root == NULL) return;
    visit(root);
    preOrderRecur(root->left);
    preOrderRecur(root->right);
}

/* ---------- 中序: 左 -> 根 -> 右 ---------- */
void inOrderRecur(TreeNode *root) {
    if (root == NULL) return;
    inOrderRecur(root->left);
    visit(root);
    inOrderRecur(root->right);
}

/* ---------- 后序: 左 -> 右 -> 根 ---------- */
void postOrderRecur(TreeNode *root) {
    if (root == NULL) return;
    postOrderRecur(root->left);
    postOrderRecur(root->right);
    visit(root);
}


/* ================================================================
   一、前序遍历（非递归）   根 → 左 → 右
   ----------------------------------------------------------------
   思路：
     1. 根入栈
     2. 弹出栈顶，访问它
     3. 先压右孩子，再压左孩子（这样出栈时左先处理）
     4. 重复直到栈空
   ================================================================ */
void PreOrder(BiTree root) {
    if (root == NULL) return;

    Stack s;
    StackInit(&s);
    StackPush(&s, root);

    printf("前序（非递归）: ");
    while (!StackEmpty(&s)) {
        TreeNode *cur;
        StackPop(&s, &cur);         // 弹出栈顶
        printf("%d ", cur->data);   // 访问

        // 先右后左 —— 保证左孩子先出栈
        if (cur->right) StackPush(&s, cur->right);
        if (cur->left)  StackPush(&s, cur->left);
    }
    printf("\n");
}

/* ================================================================
   二、中序遍历（非递归）   左 → 根 → 右
   ----------------------------------------------------------------
   思路：
     1. 从根开始，一路向左走到底，沿途所有结点入栈
     2. 弹出栈顶，访问它
     3. 转向它的右子树，对右子树重复第 1 步
     4. 重复直到栈空且当前指针为 NULL
   ================================================================ */
void InOrder(BiTree root) {
    Stack s;
    StackInit(&s);

    TreeNode *cur = root;

    printf("中序（非递归）: ");
    while (cur != NULL || !StackEmpty(&s)) {
        // 一路向左，沿途入栈
        while (cur != NULL) {
            StackPush(&s, cur);
            cur = cur->left;
        }
        // 此时 cur == NULL，弹出栈顶（最左结点）
        StackPop(&s, &cur);
        printf("%d ", cur->data);   // 访问
        cur = cur->right;           // 转向右子树
    }
    printf("\n");
}

/* ================================================================
   三、后序遍历（非递归）   左 → 右 → 根
   ----------------------------------------------------------------
   后序是三种非递归中最难的 —— 根必须在左右子树都访问完之后才能访问。

   经典解法："lastVisited 标记法"
     1. 从根开始，一路向左走到底，沿途入栈
     2. 查看栈顶结点：
        - 如果栈顶有右孩子 且 右孩子不是"上一次刚访问过的"结点
          → 转向右子树（不弹出栈顶，因为根还不能访问）
        - 否则（没有右孩子，或者右孩子刚被访问过）
          → 弹出栈顶，访问它，并标记为 lastVisited
     3. 重复直到栈空
   ================================================================ */
void PostOrder(BiTree root) {
    if (root == NULL) return;

    Stack s;
    StackInit(&s);

    TreeNode *cur         = root;
    TreeNode *lastVisited = NULL;   // 记录上一次刚访问过的结点

    printf("后序（非递归）: ");
    while (cur != NULL || !StackEmpty(&s)) {
        // 一路向左，沿途入栈
        while (cur != NULL) {
            StackPush(&s, cur);
            cur = cur->left;
        }

        // 查看栈顶（不弹出）
        TreeNode *top;
        StackTop(&s, &top);

        // 如果栈顶结点有右孩子，且右孩子还没被访问过 → 转向右子树
        if (top->right != NULL && top->right != lastVisited) {
            cur = top->right;
        } else {
            // 右子树不存在或已经访问过 → 可以访问当前栈顶了
            StackPop(&s, &top);
            printf("%d ", top->data);
            lastVisited = top;   // 标记为最近访问
            // cur 保持 NULL，下一轮会继续查看新的栈顶
        }
    }
    printf("\n");
}

/* ================================================================
   四、层序遍历（BFS）—— 顺手写一个，用队列
   ================================================================ */
typedef struct {
    TreeNode* data[MAX];
    int front, rear;
} Queue;

void QueueInit(Queue *q) { q->front = q->rear = 0; }
Status QueueEmpty(Queue *q) { return q->front == q->rear; }
Status EnQueue(Queue *q, TreeNode *p) {
    if ((q->rear + 1) % MAX == q->front) return ERROR;
    q->data[q->rear] = p;
    q->rear = (q->rear + 1) % MAX;
    return OK;
}
Status DeQueue(Queue *q, TreeNode **p) {
    if (q->front == q->rear) return ERROR;
    *p = q->data[q->front];
    q->front = (q->front + 1) % MAX;
    return OK;
}

void LevelOrder(BiTree root) {
    if (root == NULL) return;

    Queue q;
    QueueInit(&q);
    EnQueue(&q, root);

    printf("层序（BFS）  : ");
    while (!QueueEmpty(&q)) {
        TreeNode *cur;
        DeQueue(&q, &cur);
        printf("%d ", cur->data);
        if (cur->left)  EnQueue(&q, cur->left);
        if (cur->right) EnQueue(&q, cur->right);
    }
    printf("\n");
}

/* ================================================================
   主函数 —— 验证四种遍历
   ================================================================ */
int main() {
    BiTree root = BuildDemoTree();

    PreOrder(root);    // 期望：1 2 4 5 3 6 7
    InOrder(root);     // 期望：4 2 5 1 6 3 7
    PostOrder(root);   // 期望：4 5 2 6 7 3 1
    LevelOrder(root);  // 期望：1 2 3 4 5 6 7

    return 0;
}
