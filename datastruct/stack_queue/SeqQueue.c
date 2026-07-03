#include <stdio.h>
#include <stdlib.h>

#define MAXQSIZE 100   // 最大队列长度
#define OK 1
#define ERROR 0
#define OVERFLOW -1

typedef int Status;
typedef int QElemType;

// ---------- 循环顺序队列结构定义 ----------
typedef struct {
    QElemType* base;    // 动态分配存储空间基址
    int front;          // 队头指针，指向队头元素
    int rear;           // 队尾指针，指向队尾元素的下一个位置
} SqQueue;

// ---------- 初始化 ----------
Status InitQueue(SqQueue* Q) {
    Q->base = (QElemType*)malloc(MAXQSIZE * sizeof(QElemType));
    if (!Q->base) exit(OVERFLOW);
    Q->front = Q->rear = 0;
    return OK;
}

// ---------- 销毁 ----------
Status DestroyQueue(SqQueue* Q) {
    free(Q->base);
    Q->base = NULL;
    Q->front = Q->rear = 0;
    return OK;
}

// ---------- 清空 ----------
Status ClearQueue(SqQueue* Q) {
    Q->front = Q->rear = 0;
    return OK;
}

// ---------- 判空 ----------
Status QueueEmpty(SqQueue Q) {
    return Q.front == Q.rear ? OK : ERROR;
}

// ---------- 获取长度 ----------
int QueueLength(SqQueue Q) {
    return (Q.rear - Q.front + MAXQSIZE) % MAXQSIZE;
}

// ---------- 获取队头 ----------
Status GetHead(SqQueue Q, QElemType* e) {
    if (Q.front == Q.rear) return ERROR;
    *e = Q.base[Q.front];
    return OK;
}

// ---------- 入队 ----------
Status EnQueue(SqQueue* Q, QElemType e) {
    // 判满：(rear + 1) % MAXQSIZE == front （牺牲一个单元）
    if ((Q->rear + 1) % MAXQSIZE == Q->front) {
        return ERROR;   // 队列满
    }
    Q->base[Q->rear] = e;
    Q->rear = (Q->rear + 1) % MAXQSIZE;  // 循环后移
    return OK;
}

// ---------- 出队 ----------
Status DeQueue(SqQueue* Q, QElemType* e) {
    if (Q->front == Q->rear) return ERROR;  // 队列空
    *e = Q->base[Q->front];
    Q->front = (Q->front + 1) % MAXQSIZE;   // 循环后移
    return OK;
}