#include <stdio.h>
#include <stdlib.h>

#define OK 1
#define ERROR 0

typedef int Status;
typedef int QElemType;

// ---------- 链队列结点定义 ----------
typedef struct QNode {
    QElemType data;
    struct QNode* next;
} QNode, *QueuePtr;

// ---------- 链队列结构定义 ----------
typedef struct {
    QueuePtr front;   // 队头指针（指向头结点）
    QueuePtr rear;    // 队尾指针（指向队尾结点）
} LinkQueue;

// ---------- 初始化 有头结点 ----------
Status InitQueue(LinkQueue* Q) {
    Q->front = Q->rear = (QueuePtr)malloc(sizeof(QNode));
    if (!Q->front) return ERROR;
    Q->front->next = NULL;
    return OK;
}

// ---------- 销毁 ----------
Status DestroyQueue(LinkQueue* Q) {
    QueuePtr p = Q->front;
    QueuePtr temp;
    while (p) {
        temp = p;
        p = p->next;
        free(temp);
    }
    Q->front = Q->rear = NULL;
    return OK;
}

// ---------- 清空 ----------
Status ClearQueue(LinkQueue* Q) {
    QueuePtr p = Q->front->next;  // 第一个数据结点
    QueuePtr temp;
    while (p) {
        temp = p;
        p = p->next;
        free(temp);
    }
    Q->rear = Q->front;           // 重置 rear 指向头结点
    Q->front->next = NULL;
    return OK;
}

// ---------- 判空 ----------
Status QueueEmpty(LinkQueue Q) {
    return Q.front == Q.rear ? OK : ERROR;
}

// ---------- 获取长度 ----------
int QueueLength(LinkQueue Q) {
    int count = 0;
    QueuePtr p = Q.front->next;
    while (p) {
        count++;
        p = p->next;
    }
    return count;
}

// ---------- 获取队头 ----------
Status GetHead(LinkQueue Q, QElemType* e) {
    if (Q.front == Q.rear) return ERROR;
    *e = Q.front->next->data;
    return OK;
}

// ---------- 入队（尾插法） ----------
Status EnQueue(LinkQueue* Q, QElemType e) {
    QueuePtr p = (QueuePtr)malloc(sizeof(QNode));
    if (!p) return ERROR;
    p->data = e;
    p->next = NULL;
    Q->rear->next = p;   // 插入到队尾
    Q->rear = p;         // 更新队尾指针
    return OK;
}

// ---------- 出队（头删法） ----------
Status DeQueue(LinkQueue* Q, QElemType* e) {
    if (Q->front == Q->rear) return ERROR;
    QueuePtr p = Q->front->next;   // 第一个数据结点
    *e = p->data;
    Q->front->next = p->next;      // 头结点指向第二个数据结点
    
    // 若删除的是最后一个元素，rear 指向头结点
    if (Q->rear == p) {
        Q->rear = Q->front;
    }
    free(p);
    return OK;
}