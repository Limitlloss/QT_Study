#ifndef CU_H
#define CU_H

#include <chrono>
#include <memory>
#include <unordered_map>

inline long long steady_milli() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
}
inline long long system_milli() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}
inline long long steady_micro() {
    return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
}
inline long long system_micro() {
    return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}


template <class T>
struct SharedData {
    T data;
    uint64_t cnt{1};
};
template <class T>
class SharedPtr {
public:
    SharedPtr(SharedData<T> *ptr = 0) : ptr{ptr} {}
    SharedPtr(const SharedPtr &other) : ptr{other.ptr} {
        if(ptr) ptr->cnt++;
    }
    ~SharedPtr() {
        if(ptr==0) return;
        if(ptr->cnt > 1) ptr->cnt--;
        else delete ptr;
    }

    bool isNull() {return ptr==0;}
    SharedPtr &operator=(const SharedPtr &other) {
        this->~SharedPtr();
        new (this) SharedPtr(other);
        return *this;
    }

    T &operator*() {
        if(ptr==0) ptr = new SharedData<T>;
        return ptr->data;
    }
    T *operator->() {
        if(ptr==0) ptr = new SharedData<T>;
        return &ptr->data;
    }
    const T &operator*() const {
        return ptr->data;
    }
    const T *operator->() const {
        return &ptr->data;
    }
    SharedData<T> *ptr{0};
};


template <class V>
class Vector : public SharedPtr<std::vector<V>> {
public:
    using SharedPtr<std::vector<V>>::SharedPtr;

    using iterator       = std::_Vector_iterator<std::_Vector_val<std::_Simple_types<V>>>;
    using const_iterator = std::_Vector_const_iterator<std::_Vector_val<std::_Simple_types<V>>>;

    Vector(std::initializer_list<V> _Ilist) {
        this->ptr = new SharedData<std::vector<V>>{_Ilist, 1};
    }

    bool empty() const noexcept {
        return this->ptr ? this->ptr->data.empty() : true;
    }
    uint64_t size() const noexcept {
        return this->ptr ? this->ptr->data.size() : 0;
    }
    Vector &append(const V &val) {
        (**this).push_back(val);
        return *this;
    }
    V &operator[](const uint64_t pos) noexcept {
        return (**this)[pos];
    }
    const V &operator[](const uint64_t pos) const noexcept {
        return this->ptr ? this->ptr->data[pos] : V();
    }
    const const_iterator begin() const noexcept {
        return this->ptr ? this->ptr->data.begin() : const_iterator();
    }
    const const_iterator end() const noexcept {
        return this->ptr ? this->ptr->data.end() : const_iterator();
    }
};


struct NodeBase {
    NodeBase *next{this};
    NodeBase *prev{this};
};
template <class V>
struct _Node : NodeBase {
    V value;
    ~_Node() {
        if(next) delete (_Node<V>*) next;
    }
};

template <class V>
class LinkedMapIterator {
public:
    LinkedMapIterator(_Node<V> *node) : node(node) {}
    bool operator==(const LinkedMapIterator& that) const {
        return node == that.node;
    }
    bool operator!=(const LinkedMapIterator& that) const {
        return node != that.node;
    }
    LinkedMapIterator& operator++() {
        node = (_Node<V>*) node->next;
        return *this;
    }
    LinkedMapIterator& operator--() {
        node = (_Node<V>*) node->prev;
        return *this;
    }
    V &operator*() const {
        return node->value;
    }
    V *operator->() const {
        return &node->value;
    }
    _Node<V> *node{0};
};

template <class K, class V>
struct LinkedMapPri : NodeBase {
    std::unordered_map<K, _Node<std::pair<K, V>>*> map;
    uint64_t cnt{1};
    ~LinkedMapPri() {
        if(prev) prev->next = 0;
        if(next) delete (_Node<std::pair<K, V>>*) next;
    }
};
template <class K, class V>
class LinkedMap {
public:
    using Node = _Node<std::pair<K, V>>;

    using iterator       = LinkedMapIterator<std::pair<K, V>>;
    using const_iterator = LinkedMapIterator<std::pair<K, V>>;

    LinkedMap() {}
    LinkedMap(std::initializer_list<std::pair<K, V>> pairs) : _pri{new LinkedMapPri<K, V>} {
        for(auto pair : pairs) insert(pair.first, pair.second);
    }
    LinkedMap(std::unordered_map<K, Node*> &&map) : _pri{new LinkedMapPri<K, V>{0, 0, map}} {
        _pri->next = _pri->prev = _pri;
    }
    LinkedMap(const LinkedMap &other) : _pri{other._pri} {
        if(_pri) _pri->cnt++;
    }
    ~LinkedMap() {
        if(_pri==0) return;
        if(_pri->cnt > 1) _pri->cnt--;
        else delete _pri;
    }

    LinkedMap &operator=(const LinkedMap &other) {
        this->~LinkedMap();
        new (this) LinkedMap(other);
        return *this;
    }
    const V operator[](const K &k) const {
        if(_pri==0) return V();
        auto it = _pri->map.find(k);
        if(it==_pri->map.end()) return V();
        return it->second->value.second;
    }
    LinkedMap &insert(const K &k, const V &v) {
        if(_pri==0) _pri = new LinkedMapPri<K, V>;
        auto pair = _pri->map.emplace(k, nullptr);
        if(pair.second) {
            auto node = new Node{_pri, _pri->prev, {k, v}};
            _pri->prev->next = node;
            _pri->prev = node;
            pair.first->second = node;
        } else pair.first->second->value.second = v;
        return *this;
    }
    void erase(const K& k) {
        if(_pri==0) return;
        auto it = _pri->map.find(k);
        if(it==_pri->map.end()) return;
        auto node = it->second;
        _pri->map.erase(it);
        node->prev->next = node->next;
        node->next->prev = node->prev;
        node->next = 0;
        node->prev = 0;
        delete node;
    }
    inline bool empty() const {
        return _pri==0 || _pri->map.empty();
    }

    const_iterator begin() const {
        return const_iterator((Node*) (_pri ? _pri->next : 0));
    }
    const_iterator end() const {
        return const_iterator((Node*) _pri);
    }
    LinkedMapPri<K, V> *_pri{0};
};

#endif // CU_H
