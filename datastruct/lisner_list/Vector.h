#include <algorithm>
#include <memory>
#include <utility>

template <typename Object>
class Vector
{
public:
    // 显式转换构造
    explicit Vector(int initSize = 0) : theSize{initSize}, theCapacity{initSize + SPARE_CAPACITY}
    {
        objects = std::make_unique<Object[]>(theCapacity);
    }
    // 深拷贝构造
    Vector(const Vector &rhs) : theSize{rhs.theSize},
                                theCapacity{rhs.theCapacity},
                                objects{std::make_unique<Object[]>(rhs.theCapacity)}
    {
        std::copy(rhs.objects.get(), rhs.objects.get() + theSize, objects.get());
    }

    Vector &operator=(const Vector &rhs)
    {
        if (this != &rhs)
        {
            Vector temp(rhs);
            swap(temp);
        }
        return *this;
    }

    // 移动构造
    Vector(Vector &&rhs) noexcept
        : theSize{rhs.theSize}, theCapacity{rhs.theCapacity}, objects{std::move(rhs.objects)}
    {
        rhs.theSize = 0;
        rhs.theCapacity = 0;
    }
    // 移动赋值
    Vector &operator=(Vector &&rhs) noexcept
    {
        Vector temp(rhs);
        swap(temp);
        return *this;
    }

    void swap(Vector &rhs)
    {
        std::swap(theSize, rhs.theSize);
        std::swap(theCapacity, rhs.theCapacity);
        objects.swap(rhs.objects);
    }

    void resize(int newSize)
    {
        if (newSize > theCapacity)
            reserve(newSize * 2);
        theSize = newSize;
    }
    void reserve(int newCapacity)
    {
        if (newCapacity < theSize)
            return;
        auto newArray = std::make_unique<Object[]>(newCapacity);
        std::move(objects.get(), objects.get() + theSize, newArray.get());
        theCapacity = newCapacity;
        objects = std::move(newArray);
    }

    Object &operator[](int index) { return objects[index]; }
    const Object &operator[](int index) const { return objects[index]; }

    bool empty() const { return theSize == 0; }
    int size() const { return theSize; }
    int capacity() const { return theCapacity; }

    void push_back(const Object &x)
    {
        if (theSize == theCapacity)
            reserve(2 * theCapacity + 1);
        objects[theSize++] = x;
    }
    void push_back(Object &&x)
    {
        if (theSize == theCapacity)
            reserve(2 * theCapacity + 1);
        objects[theSize++] = std::move(x);
    }
    void pop_back()
    {
        objects[theSize - 1].~Object();
        --theSize;
    }
    const Object &back() const { return objects[theSize - 1]; }

    typedef Object *iterator;
    typedef const Object *const_iterator;

    iterator begin() { return &objects[0]; }
    const_iterator begin() const { return &objects[0]; }
    iterator end() { return objects.get() + size(); }
    const_iterator end() const { return objects.get() + size(); }

    static const int SPARE_CAPACITY = 16;

private:
    int theSize;
    int theCapacity;
    std::unique_ptr<Object[]> objects;
};