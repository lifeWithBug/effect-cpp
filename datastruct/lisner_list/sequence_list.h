#include <memory>

const int MAXSIZE = 100;
typedef int ElemType;

class SeqList {
private:
    ElemType data[MAXSIZE];
    int length;
public:
    SeqList():length(0){}

    void init() {
        length = 0;
    }

    bool insert(int pos, ElemType elem);
    int append(ElemType elem);
    bool remove(int pos);
    void print();
};

class SeqListDy {
private:
    std::unique_ptr<ElemType[]> data;
    int length;         //当前元素个数
    int capacity;       //当前容量
public:
    SeqListDy():data(nullptr),length(0),capacity(0) {}
    explicit SeqListDy(int cap):data(std::make_unique<ElemType[]>(cap)),length(0),capacity(cap) {}
    ~SeqListDy() = default;
    
    void init() {
        data.release();
        length = 0;
        capacity=0;
    }

    bool insert(int pos, ElemType elem);
    int append(ElemType elem);
    bool remove(int pos);
    bool resize(int new_capacity);
    void print();
};