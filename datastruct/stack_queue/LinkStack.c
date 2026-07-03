#include <stdio.h>
#include <stdlib.h>

#define OK 1
#define ERROR 0

typedef int Status;
typedef int SElemType;

// ---------- 链栈结点定义 ----------
typedef struct StackNode {
    SElemType data;
    struct StackNode* next;
} StackNode, *LinkStackPtr;

// ---------- 链栈结构定义 ----------
typedef struct {
    LinkStackPtr top;   // 栈顶指针
    int count;          // 结点个数
} LinkStack;

// ---------- 初始化 无头结点 ----------
Status InitStack(LinkStack* S) {
    S->top = NULL;
    S->count = 0;
    return OK;
}

// ---------- 销毁 ----------
Status DestroyStack(LinkStack* S) {
    LinkStackPtr p;
    while (S->top) {
        p = S->top;
        S->top = S->top->next;
        free(p);
    }
    S->count = 0;
    return OK;
}

// ---------- 清空 ----------
Status ClearStack(LinkStack* S) {
    return DestroyStack(S);
}

// ---------- 判空 ----------
Status StackEmpty(LinkStack S) {
    return S.top == NULL ? OK : ERROR;
}

// ---------- 获取长度 ----------
int StackLength(LinkStack S) {
    return S.count;
}

// ---------- 获取栈顶 ----------
Status GetTop(LinkStack S, SElemType* e) {
    if (S.top == NULL) return ERROR;
    *e = S.top->data;
    return OK;
}

// ---------- 入栈（头插法） ----------
Status Push(LinkStack* S, SElemType e) {
    LinkStackPtr p = (LinkStackPtr)malloc(sizeof(StackNode));
    if (!p) return ERROR;
    p->data = e;
    p->next = S->top;   // 新结点指向原栈顶
    S->top = p;         // 更新栈顶
    S->count++;
    return OK;
}

// ---------- 出栈（头删法） ----------
Status Pop(LinkStack* S, SElemType* e) {
    if (S->top == NULL) return ERROR;
    LinkStackPtr p = S->top;
    *e = p->data;
    S->top = S->top->next;  // 栈顶下移
    free(p);
    S->count--;
    return OK;
}