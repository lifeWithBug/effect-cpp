#include "sequence_list.h"
#include <iostream>

bool SeqList::insert(int pos, ElemType elem)
{
    if (pos < 1 || pos > length + 1 || length >= MAXSIZE)
        return false;
    for (int i = length; i >= pos; i--)
    {
        data[i] = data[i - 1];
    }
    data[pos - 1] = elem;
    length++;
    return true;
}

int SeqList::append(ElemType elem)
{
    if (length >= MAXSIZE)
        return -1;
    data[length++] = elem;
    return length;
}

bool SeqList::remove(int pos)
{
    if (pos < 1 || pos > length)
        return false;
    for (int i = pos; i < length; i++)
    {
        data[i - 1] = data[i];
    }
    length--;
    return true;
}
void SeqList::print()
{
    std::cout << "SeqList: ";
    for (int i = 0; i < length; i++)
    {
        std::cout << data[i] << " ";
    }
    std::cout << "(length=" << length << ")\n";
}

bool SeqLinkList::insert(int pos, ElemType elem)
{
    if(pos<1||pos>=MAXSIZE||length>=MAXSIZE)
        return false;
    for(int i=length;i>=pos;i--)
    {
        data[i]=data[i-1];
    }
    data[pos-1]=elem;
    length++;
    return true;
}

int main()
{
    SeqList list;
    list.init();
    list.insert(1, 10);
    list.insert(2, 20);
    list.insert(3, 30);
    list.print();

    list.remove(2);
    list.print();

    list.append(40);
    list.print();

    return 0;
}