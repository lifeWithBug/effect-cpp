/*
 * 平衡二叉树（AVL 树）完整实现
 * 包含：插入、四种旋转（LL/RR/LR/RL）、中序遍历、销毁
 *
 * AVL 树定义：一棵 BST，且任意结点的
 *   | 左子树高度 - 右子树高度 | <= 1
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/* ========================== 结点定义 ========================== */

typedef struct AVLNode {
    int data;
    int height;               // 以该结点为根的子树高度
    struct AVLNode *left;
    struct AVLNode *right;
} AVLNode;

/* ========================== 辅助函数 ========================== */

AVLNode *newNode(int data) {
    AVLNode *node = (AVLNode *)malloc(sizeof(AVLNode));
    node->data  = data;
    node->height = 1;         // 新结点初始高度为 1
    node->left  = NULL;
    node->right = NULL;
    return node;
}

int height(AVLNode *node) {
    return node ? node->height : 0;
}

int max(int a, int b) {
    return a > b ? a : b;
}

/*
 * 平衡因子 = 左子树高度 - 右子树高度
 * 正常范围: -1, 0, 1
 * > 1: 左边偏重，需要右旋或左右双旋
 * < -1: 右边偏重，需要左旋或右左双旋
 */
int balanceFactor(AVLNode *node) {
    return node ? height(node->left) - height(node->right) : 0;
}

void updateHeight(AVLNode *node) {
    node->height = 1 + max(height(node->left), height(node->right));
}

/* ====================== 四种旋转操作 ====================== */

/*
 * LL 型：右旋（Right Rotation）
 *
 * 触发条件：BF(root) = 2 且 BF(root->left) >= 0
 * （即"左子树的左边"插入导致失衡）
 *
 *    root(2)           newRoot
 *    /    \             /    \
 *   A(1)   C    -->    B    root
 *  /   \                    /   \
 * B     D                  D     C
 *
 * 只做一次顺时针旋转。
 */
AVLNode *rotateRight(AVLNode *root) {
    AVLNode *newRoot = root->left;
    AVLNode *T2      = newRoot->right;

    // 旋转
    newRoot->right = root;
    root->left     = T2;

    // 必须先更新 root，再更新 newRoot（因为 newRoot 在 root 上面）
    updateHeight(root);
    updateHeight(newRoot);

    return newRoot;
}

/*
 * RR 型：左旋（Left Rotation）
 *
 * 触发条件：BF(root) = -2 且 BF(root->right) <= 0
 * （即"右子树的右边"插入导致失衡）
 *
 *  root(-2)                newRoot
 *  /    \                  /    \
 * A    C(-1)      -->    root    D
 *      /   \             /   \
 *     B     D           A     B
 *
 * 只做一次逆时针旋转。和右旋完全对称。
 */
AVLNode *rotateLeft(AVLNode *root) {
    AVLNode *newRoot = root->right;
    AVLNode *T2      = newRoot->left;

    newRoot->left = root;
    root->right   = T2;

    updateHeight(root);
    updateHeight(newRoot);

    return newRoot;
}

/*
 * LR 型：左右双旋（Left-Right Rotation）
 *
 * 触发条件：BF(root) = 2 且 BF(root->left) <= -1
 * （即"左子树的右边"插入导致失衡）
 *
 * 步骤：
 *   1. 先对左子树做左旋（变成 LL 型）
 *   2. 再对根做右旋
 *
 *     root                  root                  C
 *     /   \      左旋左子树   /   \      右旋根     /   \
 *    A     C     -------->  B     C    ------->  A    root
 *   / \                   / \                   / \   / \
 *  D   B                 A   E                 D   F E   C
 *     / \               / \
 *    F   E             D   F
 */
AVLNode *rotateLeftRight(AVLNode *root) {
    root->left = rotateLeft(root->left);   // 第一步：左子树左旋
    return rotateRight(root);               // 第二步：根右旋
}

/*
 * RL 型：右左双旋（Right-Left Rotation）
 *
 * 触发条件：BF(root) = -2 且 BF(root->right) >= 1
 * （即"右子树的左边"插入导致失衡）
 *
 * 步骤：
 *   1. 先对右子树做右旋（变成 RR 型）
 *   2. 再对根做左旋
 *
 * 和 LR 完全对称。
 */
AVLNode *rotateRightLeft(AVLNode *root) {
    root->right = rotateRight(root->right);  // 第一步：右子树右旋
    return rotateLeft(root);                  // 第二步：根左旋
}

/* ====================== 插入 ====================== */

/*
 * 插入一个值，返回新的子树根。
 *
 * 总体步骤：
 *   1. 像普通 BST 一样递归插入
 *   2. 回溯时更新高度
 *   3. 计算平衡因子
 *   4. 如果失衡（|BF| > 1），根据 BF 判断旋转类型并执行旋转
 */
AVLNode *insert(AVLNode *root, int data) {
    // 1. 普通 BST 插入
    if (root == NULL)
        return newNode(data);

    if (data < root->data)
        root->left = insert(root->left, data);
    else if (data > root->data)
        root->right = insert(root->right, data);
    else
        return root;   // 重复值不插入

    // 2. 回溯：更新当前结点高度
    updateHeight(root);

    // 3. 计算平衡因子
    int bf = balanceFactor(root);

    // 4. 根据四种情况旋转

    // LL: 左子树的左边插入  -> 右旋
    if (bf > 1 && data < root->left->data)
        return rotateRight(root);

    // RR: 右子树的右边插入  -> 左旋
    if (bf < -1 && data > root->right->data)
        return rotateLeft(root);

    // LR: 左子树的右边插入  -> 左右双旋
    if (bf > 1 && data > root->left->data)
        return rotateLeftRight(root);

    // RL: 右子树的左边插入  -> 右左双旋
    if (bf < -1 && data < root->right->data)
        return rotateRightLeft(root);

    return root;  // 未失衡，直接返回
}

/* ====================== 遍历与验证 ====================== */

void inOrder(AVLNode *root) {
    if (root == NULL) return;
    inOrder(root->left);
    printf("%d ", root->data);
    inOrder(root->right);
}

/* 前序遍历（附带高度和 BF，用于调试观察树结构） */
void preOrderDebug(AVLNode *root) {
    if (root == NULL) return;
    printf("%d(h=%d,bf=%d) ", root->data, root->height, balanceFactor(root));
    preOrderDebug(root->left);
    preOrderDebug(root->right);
}

void destroy(AVLNode *root) {
    if (root == NULL) return;
    destroy(root->left);
    destroy(root->right);
    free(root);
}

/* ====================== 测试 ====================== */

int main() {
    AVLNode *root = NULL;

    /*
     * 测试 LL 旋转：插入 5, 4, 3
     * 插入 3 时，5 的 BF 变成 2（左边重），触发右旋
     */
    printf("=== 测试 LL 旋转: 5, 4, 3 ===\n");
    int ll[] = {5, 4, 3};
    for (int i = 0; i < 3; i++) {
        root = insert(root, ll[i]);
        printf("插入 %d 后: ", ll[i]);
        preOrderDebug(root);
        printf("\n");
    }
    inOrder(root); printf("(中序)\n\n");
    destroy(root);
    root = NULL;

    /*
     * 测试 RR 旋转：插入 1, 2, 3
     * 插入 3 时，1 的 BF 变成 -2（右边重），触发左旋
     */
    printf("=== 测试 RR 旋转: 1, 2, 3 ===\n");
    int rr[] = {1, 2, 3};
    for (int i = 0; i < 3; i++) {
        root = insert(root, rr[i]);
        printf("插入 %d 后: ", rr[i]);
        preOrderDebug(root);
        printf("\n");
    }
    inOrder(root); printf("(中序)\n\n");
    destroy(root);
    root = NULL;

    /*
     * 测试 LR 旋转：插入 6, 2, 4
     * 插入 4 时：6 的 BF=2（左边重），但 4 插在 2 的右边
     * 触发左右双旋
     */
    printf("=== 测试 LR 旋转: 6, 2, 4 ===\n");
    int lr[] = {6, 2, 4};
    for (int i = 0; i < 3; i++) {
        root = insert(root, lr[i]);
        printf("插入 %d 后: ", lr[i]);
        preOrderDebug(root);
        printf("\n");
    }
    inOrder(root); printf("(中序)\n\n");
    destroy(root);
    root = NULL;

    /*
     * 测试 RL 旋转：插入 2, 6, 4
     * 插入 4 时：2 的 BF=-2（右边重），但 4 插在 6 的左边
     * 触发右左双旋
     */
    printf("=== 测试 RL 旋转: 2, 6, 4 ===\n");
    int rl[] = {2, 6, 4};
    for (int i = 0; i < 3; i++) {
        root = insert(root, rl[i]);
        printf("插入 %d 后: ", rl[i]);
        preOrderDebug(root);
        printf("\n");
    }
    inOrder(root); printf("(中序)\n\n");
    destroy(root);
    root = NULL;

    /*
     * 综合测试：随机插入多个值
     * 观察每次插入后树是否保持平衡（所有结点的 BF 在 -1/0/1 范围）
     */
    printf("=== 综合测试: 10, 20, 30, 40, 50, 25 ===\n");
    int all[] = {10, 20, 30, 40, 50, 25};
    for (int i = 0; i < 6; i++) {
        root = insert(root, all[i]);
        printf("插入 %d 后: ", all[i]);
        preOrderDebug(root);
        printf("\n");
    }
    inOrder(root); printf("(中序)\n");

    destroy(root);
    return 0;
}
