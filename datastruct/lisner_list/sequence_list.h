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

class SeqLinkList {
private:
    std::unique_ptr<ElemType[]> data;
    int length;
public:
    SeqLinkList():data(std::make_unique<ElemType[]>(MAXSIZE)),length(0) {}
    ~SeqLinkList() = default;
    
    void init() {
        length = 0;
    }

    bool insert(int pos, ElemType elem);
    int append(ElemType elem);
    bool remove(int pos);
    void print();
};