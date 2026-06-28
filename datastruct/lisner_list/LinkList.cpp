

template <typename EleType>
class LinkList
{
public:
    LinkList() { init(); }

    // 析构函数：释放所有节点
    ~LinkList()
    {
        clear();
        delete head;
        delete tail;
    }

    bool empty() const { return head->next == tail; }

    // ---------- 核心操作 ----------

    // 在链表头部插入
    void push_front(const T &value)
    {
        insert_after(head, value);
    }

    // 在链表尾部插入
    void push_back(const T &value)
    {
        insert_before(tail, value);
    }

    // 删除头部节点
    void pop_front()
    {
        if (empty())
            return;
        remove_node(head->next);
    }

    // 删除尾部节点
    void pop_back()
    {
        if (empty())
            return;
        remove_node(tail->prev);
    }

    // 在指定节点之后插入（核心插入逻辑）
    void insert_after(Node<T> *pos, const T &value)
    {
        Node<T> *newNode = new Node<T>(value);
        Node<T> *nextNode = pos->next;

        pos->next = newNode;
        newNode->prev = pos;
        newNode->next = nextNode;
        nextNode->prev = newNode;
    }

    // 在指定节点之前插入
    void insert_before(Node<T> *pos, const T &value)
    {
        insert_after(pos->prev, value);
    }

    // 删除指定节点（核心删除逻辑）
    void remove_node(Node<T> *node)
    {
        if (node == head || node == tail || node == nullptr)
            return; // 不能删除哨兵

        node->prev->next = node->next;
        node->next->prev = node->prev;
        delete node;
    }

    void reverse()
    {
        Node<T> *current = head;
        while (current != nullptr)
        {
            std::swap(current->next, current->prev);
            current = current->prev;
        }
        std::swap(head, tail);
    }

    // 清空链表（保留哨兵）
    void clear()
    {
        Node<T> *current = head->next;
        while (current != tail)
        {
            Node<T> *next = current->next;
            delete current;
            current = next;
        }
        head->next = tail;
        tail->prev = head;
    }

    // 打印链表（正向遍历）
    void printForward() const {
        Node<T>* current = head->next;
        while (current != tail) {
            cout << current->data << " <-> ";
            current = current->next;
        }
        cout << "nullptr" << endl;
    }
private:
    struct Node
    {
        EleType data;
        Node *prev;
        Node *next;

        Node(const EleType &d = EleType{}, Node *p = nullptr, Node *n = nullptr)
            : data{d}, prev{p}, next{n} {}
    };

private:
    Node<EleType> *head;
    Node<EleType> *tail;

    void init()
    {
        head = new Node<EleType>(EleType{});
        tail = new Node<EleType>(EleType{});
        head->next = tail;
        tail->prev = head;
    }
};