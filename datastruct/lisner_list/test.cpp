#include<iostream>
#include<memory>
#include<utility>
using namespace std;
int main()
{
    unique_ptr<int> p1(new int(10));
    unique_ptr<int> p2 = make_unique<int>(20);

    shared_ptr<int> sp1 = make_shared<int>(30);
    shared_ptr<int> sp2 = sp1;
    sp1=exchange(p1,nullptr);

    p2=exchange(p1,nullptr);
}