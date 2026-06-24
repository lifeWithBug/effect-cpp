#include "sequence_list.h"
#include <iostream>
#include <utility>

bool SeqList::insert(int pos, ElemType elem)
{
    if (pos < 1 || pos > length || length >= MAXSIZE)
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

int SeqList::find(ElemType elem)
{
    for (size_t i = 0; i < length; i++)
    {
        if (elem == data[i])
            return i + 1;
    }
    return 0;
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

// SeqListDy implementation
bool SeqListDy::resize(int new_capacity)
{
    if (new_capacity <= capacity)
        return false;
    std::unique_ptr<ElemType[]> new_data = std::make_unique<ElemType[]>(new_capacity);
    data = std::exchange(new_data, nullptr);
    capacity = new_capacity;
    return true;
}

bool SeqListDy::insert(int pos, ElemType elem)
{
    if (pos < 1 || pos >= capacity || length >= capacity)
        return false;
    for (int i = capacity - 1; i >= pos; i--)
    {
        data[i] = data[i - 1];
    }
    data[pos - 1] = elem;
    length++;
    return true;
}

int SeqListDy::append(ElemType elem)
{
    if (length >= capacity)
    {
        if (!resize(capacity == 0 ? 1 : capacity * 2))
            return -1;
    }
    data[length++] = elem;
    return length;
}

bool SeqListDy::remove(int pos)
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