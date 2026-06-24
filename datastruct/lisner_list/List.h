#include <utility>

template <typename Object>
class List
{
private:
    struct Node
    {
        Object data;
        Node *prev;
        Node *next;

        Node(const Object &d = Object{}, Node *p = nullptr, Node *n = nullptr)
            : data{d}, prev{p}, next{n} {}
        Node(Object &&d, Node *p = nullptr, Node *n = nullptr)
            : data{std::move(d)}, prev{p}, next{n} {}
    };

public:
    class const_iterator
    {
    public:
        const_iterator() : current{nullptr} {}

        const Object &operator*() const { return retrieve(); }
        const Object *operator->() const { return &retrieve(); }

        const_iterator &operator++()
        {
            current = current->next;
            return *this;
        }
        const_iterator operator++(int)
        {
            const_iterator old = *this;
            ++(*this);
            return old;
        }
        const_iterator &operator--()
        {
            current = current->prev;
            return *this;
        }
        const_iterator operator--(int)
        {
            const_iterator old = *this;
            --(*this);
            return old;
        }

        bool operator==(const const_iterator &rhs) const { return current == rhs.current; }
        bool operator!=(const const_iterator &rhs) const { return !(*this == rhs); }

    protected:
        Node *current;

        Object &retrieve() const { return current->data; }
        const_iterator(Node *p) : current{p} {}

        friend class List<Object>;
    };

    class iterator : public const_iterator
    {
    public:
        iterator() {}

        Object &operator*() { return const_iterator::retrieve(); }
        const Object &operator*() const { return const_iterator::operator*(); }
        Object *operator->() { return &const_iterator::retrieve(); }
        const Object *operator->() const { return const_iterator::operator->(); }

        iterator &operator++()
        {
            this->current = this->current->next;
            return *this;
        }
        iterator operator++(int)
        {
            iterator old = *this;
            ++(*this);
            return old;
        }
        iterator &operator--()
        {
            this->current = this->current->prev;
            return *this;
        }
        iterator operator--(int)
        {
            iterator old = *this;
            --(*this);
            return old;
        }

    protected:
        iterator(Node *p) : const_iterator{p} {}
        friend class List<Object>;
    };

public:
    List() { init(); }

    List(const List &rhs)
    {
        init();
        for (const_iterator itr = rhs.begin(); itr != rhs.end(); ++itr)
            push_back(*itr);
    }

    ~List()
    {
        clear();
        delete head;
        delete tail;
    }

    List &operator=(const List &rhs)
    {
        if (this != &rhs)
        {
            List temp(rhs);
            swap(temp);
        }
        return *this;
    }

    List(List &&rhs) noexcept
        : theSize{rhs.theSize}, head{rhs.head}, tail{rhs.tail}
    {
        rhs.theSize = 0;
        rhs.init();
    }

    List &operator=(List &&rhs) noexcept
    {
        if (this != &rhs)
        {
            clear();
            delete head;
            delete tail;

            theSize = rhs.theSize;
            head = rhs.head;
            tail = rhs.tail;

            rhs.theSize = 0;
            rhs.init();
        }
        return *this;
    }

    void swap(List &rhs)
    {
        std::swap(theSize, rhs.theSize);
        std::swap(head, rhs.head);
        std::swap(tail, rhs.tail);
    }

    iterator begin() { return iterator{head->next}; }
    const_iterator begin() const { return const_iterator{head->next}; }
    iterator end() { return iterator{tail}; }
    const_iterator end() const { return const_iterator{tail}; }

    int size() const { return theSize; }
    bool empty() const { return theSize == 0; }

    Object &front() { return *begin(); }
    const Object &front() const { return *begin(); }
    Object &back() { return *(--end()); }
    const Object &back() const { return *(--end()); }

    void push_front(const Object &x) { insert(begin(), x); }
    void push_front(Object &&x) { insert(begin(), std::move(x)); }
    void push_back(const Object &x) { insert(end(), x); }
    void push_back(Object &&x) { insert(end(), std::move(x)); }

    void pop_front() { erase(begin()); }
    void pop_back() { erase(--end()); }

    // 在 pos 前插入，返回指向新元素的迭代器
    iterator insert(iterator pos, const Object &x)
    {
        Node *p = pos.current;
        ++theSize;
        Node *newNode = new Node{x, p->prev, p};
        p->prev->next = newNode;
        p->prev = newNode;
        return iterator{newNode};
    }

    iterator insert(iterator pos, Object &&x)
    {
        Node *p = pos.current;
        ++theSize;
        Node *newNode = new Node{std::move(x), p->prev, p};
        p->prev->next = newNode;
        p->prev = newNode;
        return iterator{newNode};
    }

    // 删除 pos 指向的元素，返回下一个元素的迭代器
    iterator erase(iterator pos)
    {
        Node *p = pos.current;
        iterator retVal{p->next};
        p->prev->next = p->next;
        p->next->prev = p->prev;
        delete p;
        --theSize;
        return retVal;
    }

    // 删除 [from, to) 范围内的元素
    iterator erase(iterator from, iterator to)
    {
        while (from != to)
            from = erase(from);
        return to;
    }

    void clear()
    {
        while (!empty())
            pop_front();
    }

private:
    int theSize;
    Node *head;
    Node *tail;

    void init()
    {
        theSize = 0;
        head = new Node;
        tail = new Node;
        head->next = tail;
        tail->prev = head;
    }
};
