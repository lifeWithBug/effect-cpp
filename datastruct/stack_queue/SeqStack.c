#include <stdio.h>
#include <stdlib.h>

#define STACK_INIT_SIZE 100   // 初始分配量
#define STACK_INCREMENT 10    // 扩容增量
#define OK 1
#define ERROR 0
#define OVERFLOW -1

typedef int Status;
typedef int SElemType;

// ---------- 顺序栈结构定义 ----------
typedef struct {
    SElemType* base;    // 栈底指针，构造前和销毁后为 NULL
    SElemType* top;     // 栈顶指针，指向栈顶元素的下一个位置
    int stacksize;      // 当前已分配的存储容量（以元素为单位）
} SqStack;

// ---------- 初始化 ----------
Status InitStack(SqStack* S) {
    S->base = (SElemType*)malloc(STACK_INIT_SIZE * sizeof(SElemType));
    if (!S->base) exit(OVERFLOW);
    S->top = S->base;
    S->stacksize = STACK_INIT_SIZE;
    return OK;
}

// ---------- 销毁 ----------
Status DestroyStack(SqStack* S) {
    free(S->base);
    S->base = NULL;
    S->top = NULL;
    S->stacksize = 0;
    return OK;
}

// ---------- 清空 ----------
Status ClearStack(SqStack* S) {
    S->top = S->base;
    return OK;
}

// ---------- 判空 ----------
Status StackEmpty(SqStack S) {
    return S.top == S.base ? OK : ERROR;
}

// ---------- 获取长度 ----------
int StackLength(SqStack S) {
    return (int)(S.top - S.base);
}

// ---------- 获取栈顶 ----------
Status GetTop(SqStack S, SElemType* e) {
    if (S.top == S.base) return ERROR;
    *e = *(S.top - 1);
    return OK;
}

// ---------- 入栈 ----------
Status Push(SqStack* S, SElemType e) {
    // 栈满则扩容
    if (S->top - S->base >= S->stacksize) {
        S->base = (SElemType*)realloc(S->base, 
            (S->stacksize + STACK_INCREMENT) * sizeof(SElemType));
        if (!S->base) exit(OVERFLOW);
        S->top = S->base + S->stacksize;
        S->stacksize += STACK_INCREMENT;
    }
    *(S->top) = e;
    S->top++;
    return OK;
}

// ---------- 出栈 ----------
Status Pop(SqStack* S, SElemType* e) {
    if (S->top == S->base) return ERROR;
    S->top--;
    *e = *(S->top);
    return OK;
}