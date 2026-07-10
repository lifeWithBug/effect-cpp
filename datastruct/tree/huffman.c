/*
 * 哈夫曼树（最优二叉树）与哈夫曼编码
 *
 * 核心思想：用出现频率为权重，构建一棵带权路径长度（WPL）最小的二叉树。
 * 频率越高的字符，离根越近（路径越短），从而达到数据压缩的目的。
 *
 * 演示数据：
 *   字符:  A   B   C   D   E   F
 *   频率:  45  13  12  16  9   5
 *
 * 这是教材经典数据，合计 100 个字符。
 * 哈夫曼树构建过程就是不断从森林里选两个最小的树合并。
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_CHAR 256          /* ASCII 字符集大小 */
#define MAX_NODES (MAX_CHAR * 2 - 1)  /* 哈夫曼树最大结点数 */

/* ========================== 哈夫曼树结点 ========================== */

typedef struct {
    int weight;               /* 权重（字符频率） */
    int parent;               /* 父结点下标（-1 表示无父，即还在森林中） */
    int left;                 /* 左孩子下标 */
    int right;                /* 右孩子下标 */
    char ch;                  /* 字符（仅叶子结点有意义） */
} HuffNode;

/* ========================== 构建哈夫曼树 ========================== */

/*
 * 从下标 0..n-1 中选出两个权值最小且没有父结点的结点。
 * 选出的下标存入 s1, s2。
 */
void selectMinTwo(HuffNode *nodes, int n, int *s1, int *s2) {
    int min1 = -1, min2 = -1;

    for (int i = 0; i < n; i++) {
        if (nodes[i].parent != -1) continue;  /* 已经被合并 */

        if (min1 == -1 || nodes[i].weight < nodes[min1].weight) {
            min2 = min1;
            min1 = i;
        } else if (min2 == -1 || nodes[i].weight < nodes[min2].weight) {
            min2 = i;
        }
    }

    *s1 = min1;
    *s2 = min2;
}

/*
 * 构建哈夫曼树。
 * leafCount: 叶子结点数（即不同字符的个数）
 * weights: 每个字符的频率数组
 * return: 哈夫曼树的所有结点（数组），返回最后一个结点下标（根）
 */
int buildHuffmanTree(HuffNode *nodes, int leafCount, int *weights) {
    int total = 2 * leafCount - 1;   /* 哈夫曼树总结点数 */

    /* 初始化所有结点 */
    for (int i = 0; i < total; i++) {
        nodes[i].parent = -1;
        nodes[i].left   = -1;
        nodes[i].right  = -1;
        nodes[i].weight = (i < leafCount) ? weights[i] : 0;
    }

    /* 依次合并最小的两棵树，共合并 leafCount-1 次 */
    for (int i = leafCount; i < total; i++) {
        int s1, s2;
        selectMinTwo(nodes, i, &s1, &s2);

        /* 创建新结点 i，作为 s1 和 s2 的父结点 */
        nodes[i].left  = s1;
        nodes[i].right = s2;
        nodes[i].weight = nodes[s1].weight + nodes[s2].weight;
        nodes[s1].parent = i;
        nodes[s2].parent = i;
    }

    return total - 1;  /* 根结点下标 */
}

/* ====================== 哈夫曼编码 ====================== */

/*
 * 从叶子结点出发，沿 parent 走到根，反向得到编码。
 * 编码存放在 codes 数组中，codes[i] 是对应 weights 中第 i 个字符的编码串。
 */
void buildHuffmanCodes(HuffNode *nodes, int leafCount, char **codes) {
    char buf[leafCount];  /* 临时 buffer，长度不会超过 leafCount */

    for (int i = 0; i < leafCount; i++) {
        int cur = i;
        int len = 0;

        /* 从叶子向上走到根，沿途记下方向 */
        while (nodes[cur].parent != -1) {
            int p = nodes[cur].parent;
            buf[len++] = (nodes[p].left == cur) ? '0' : '1';
            cur = p;
        }

        /* 反转：因为是从叶子到根，而编码需要从根到叶子 */
        codes[i] = (char *)malloc((len + 1) * sizeof(char));
        for (int j = 0; j < len; j++)
            codes[i][j] = buf[len - 1 - j];
        codes[i][len] = '\0';
    }
}

/* ====================== 编码与解码 ====================== */

/*
 * 对给定文本进行哈夫曼编码。
 * 需要事先知道字符到索引的映射（这里简化：使用 ASCII 码直接映射）。
 */
void encode(char *text, HuffNode *nodes,
            char **codes, int leafCount, int *charToIdx) {
    printf("编码: ");
    for (int i = 0; text[i] != '\0'; i++) {
        int idx = charToIdx[(unsigned char)text[i]];
        printf("%s", codes[idx]);
    }
    printf("\n");
}

/*
 * 解码：从根出发，读 0 向左走，读 1 向右走，
 * 到达叶子就输出字符，然后回到根。
 */
void decode(char *encoded, HuffNode *nodes, int root) {
    printf("解码: ");
    int cur = root;
    for (int i = 0; encoded[i] != '\0'; i++) {
        cur = (encoded[i] == '0') ? nodes[cur].left : nodes[cur].right;

        /* 到达叶子结点 */
        if (nodes[cur].left == -1 && nodes[cur].right == -1) {
            printf("%c", nodes[cur].ch);
            cur = root;  /* 回到根，继续解码 */
        }
    }
    printf("\n");
}

/* ====================== 计算 WPL ====================== */

/*
 * 计算带权路径长度（WPL）。
 * 每个叶子结点的权重 × 路径长度，求和。
 * = 所有非叶子结点的权重之和（重要性质）。
 */
int calcWPL(HuffNode *nodes, int leafCount, int total) {
    int wpl = 0;
    for (int i = 0; i < total; i++) {
        /* 哈夫曼树中非叶子结点的权重之和 = WPL */
        if (nodes[i].left != -1 || nodes[i].right != -1)
            wpl += nodes[i].weight;
    }
    return wpl;
}

/* 打印编码表 */
void printCodes(char *chars, int *weights, char **codes, int n) {
    printf("字符  频率  编码\n");
    printf("----------------\n");
    for (int i = 0; i < n; i++)
        printf("  %c    %2d    %s\n", chars[i], weights[i], codes[i]);
}

/* ========================== 测试 ========================== */

int main() {
    /* 演示数据 */
    char chars[] = {'A', 'B', 'C', 'D', 'E', 'F'};
    int weights[] = {45, 13, 12, 16, 9, 5};
    int n = sizeof(chars) / sizeof(chars[0]);

    printf("原数据:\n");
    printf("字符: ");
    for (int i = 0; i < n; i++) printf("%c ", chars[i]);
    printf("\n频率: ");
    for (int i = 0; i < n; i++) printf("%d ", weights[i]);
    printf("\n\n");

    /* 构建哈夫曼树 */
    HuffNode nodes[MAX_NODES];
    int root = buildHuffmanTree(nodes, n, weights);

    /* 生成编码 */
    char *codes[n];
    buildHuffmanCodes(nodes, n, codes);

    printCodes(chars, weights, codes, n);

    printf("\nWPL = %d\n", calcWPL(nodes, n, 2 * n - 1));

    /* 构建字符到索引的映射 */
    int charToIdx[MAX_CHAR];
    for (int i = 0; i < MAX_CHAR; i++) charToIdx[i] = -1;
    for (int i = 0; i < n; i++) charToIdx[(unsigned char)chars[i]] = i;
    /* 给叶子结点关联对应字符 */
    for (int i = 0; i < n; i++) nodes[i].ch = chars[i];

    /* 编码 */
    char text[] = "BADFEDCBA";
    printf("\n原文: %s\n", text);
    encode(text, nodes, codes, n, charToIdx);

    /* 解码 */
    char encoded[] = "110110011001110101011011000";
    /* 用换行符结束避免被吞掉 */
    char encoded2[] = "10001111001110100110";
    printf("已知编码: %s\n", encoded2);
    decode(encoded2, nodes, root);

    /* 释放编码内存 */
    for (int i = 0; i < n; i++) free(codes[i]);

    return 0;
}
