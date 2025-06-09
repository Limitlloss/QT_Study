#ifndef CPP_H
#define CPP_H

#include <chrono>
#include <unordered_map>

inline int64_t steady_milli() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
}
inline int64_t system_milli() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}
inline int64_t steady_micro() {
    return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
}
inline int64_t system_micro() {
    return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}


template <class T>
struct SharedData {
    size_t cnt{1};
    T data;
};
template <class T>
class SharedPtr {
public:
    SharedPtr(SharedData<T> *ptr = 0) : ptr{ptr} {}
    ~SharedPtr() {
        if(ptr==0) return;
        if(ptr->cnt > 1) ptr->cnt--;
        else delete ptr;
    }

    SharedPtr(const SharedPtr &other) : ptr{other.ptr} {
        if(ptr) ptr->cnt++;
    }
    SharedPtr &operator=(const SharedPtr &other) {
        this->~SharedPtr();
        new (this) SharedPtr(other);
        return *this;
    }
    SharedPtr(SharedPtr &&other) noexcept : ptr(other.ptr) {
        other.ptr = 0;
    }
    SharedPtr &operator=(SharedPtr &&other) noexcept {
        auto aaa = ptr;
        ptr = other.ptr;
        other.ptr = aaa;
        return *this;
    }

    bool isNull() const noexcept {return ptr==0;}
    bool empty() const noexcept {
        return ptr ? ptr->data.empty() : true;
    }
    size_t size() const noexcept {
        return ptr ? ptr->data.size() : 0;
    }

    T &operator*() const {
        if(ptr==0) ptr = new SharedData<T>;
        return ptr->data;
    }
    T *operator->() const {
        if(ptr==0) ptr = new SharedData<T>;
        return &ptr->data;
    }
    bool operator==(const SharedPtr &other) const {
        if(ptr==other.ptr) return true;
        auto siz = size();
        if(siz!=other.size()) return false;
        if(siz==0) return true;
        return ptr->data==other.ptr->data;
    }
    bool operator!=(const SharedPtr &other) const {
        return ! (*this==other);
    }
    mutable SharedData<T> *ptr = 0;
};


template <class V>
class Vector : public SharedPtr<std::vector<V>> {
public:
    using SharedPtr<std::vector<V>>::SharedPtr;

    using iterator       = std::_Vector_iterator<std::_Vector_val<std::_Simple_types<V>>>;
    using const_iterator = std::_Vector_const_iterator<std::_Vector_val<std::_Simple_types<V>>>;

    Vector(std::initializer_list<V> _Ilist) {
        this->ptr = new SharedData<std::vector<V>>{1, _Ilist};
    }

    Vector &append(const V &val) {
        (*this)->push_back(val);
        return *this;
    }
    Vector &append(V&& val) {
        (*this)->push_back(_STD move(val));
        return *this;
    }
    Vector &operator<<(const V &val) {
        (*this)->push_back(val);
        return *this;
    }
    Vector &operator<<(V&& val) {
        (*this)->push_back(_STD move(val));
        return *this;
    }

    V &operator[](const uint64_t pos) noexcept {
        return (**this)[pos];
    }
    const V &operator[](const uint64_t pos) const noexcept {
        return (**this)[pos];
    }

    iterator begin() const noexcept {
        return this->ptr ? this->ptr->data.begin() : iterator();
    }
    iterator end() const noexcept {
        return this->ptr ? this->ptr->data.end() : iterator();
    }
};


struct NodeBase {
    NodeBase *next{this};
    NodeBase *prev{this};
};
template <class P>
struct _Node : NodeBase {
    P value;
    ~_Node() {
        if(next) delete (_Node<P>*) next;
    }
};

template <class P>
class LinkedMapIterator {
public:
    LinkedMapIterator(_Node<P> *node) : node(node) {}
    bool operator==(const LinkedMapIterator& other) const noexcept {
        return node == other.node;
    }
    bool operator!=(const LinkedMapIterator& other) const noexcept {
        return node != other.node;
    }
    LinkedMapIterator& operator++() {
        node = (_Node<P>*) node->next;
        return *this;
    }
    LinkedMapIterator& operator--() {
        node = (_Node<P>*) node->prev;
        return *this;
    }
    const LinkedMapIterator operator++(int) {
        auto rtn = *this;
        node = (_Node<P>*) node->next;
        return rtn;
    }
    const LinkedMapIterator operator--(int) {
        auto rtn = *this;
        node = (_Node<P>*) node->prev;
        return rtn;
    }
    P &operator*() const noexcept {
        return node->value;
    }
    P *operator->() const noexcept {
        return &node->value;
    }
    _Node<P> *node{0};
};

template <class K, class V>
struct LinkedMapPri : NodeBase {
    size_t cnt = 1;
    std::unordered_map<K, _Node<std::pair<K, V>>*> map;
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
        for(auto &pair : pairs) insert(pair.first, pair.second);
    }
    LinkedMap(std::unordered_map<K, Node*> &&map) : _pri{new LinkedMapPri<K, V>{0, 0, map}} {
        _pri->next = _pri->prev = _pri;
    }
    ~LinkedMap() {
        if(_pri==0) return;
        if(_pri->cnt > 1) _pri->cnt--;
        else delete _pri;
    }

    LinkedMap(const LinkedMap &other) : _pri{other._pri} {
        if(_pri) _pri->cnt++;
    }
    LinkedMap &operator=(const LinkedMap &other) {
        this->~LinkedMap();
        new (this) LinkedMap(other);
        return *this;
    }
    LinkedMap(LinkedMap &&other) noexcept : _pri(other._pri) {
        other._pri = 0;
    }
    LinkedMap &operator=(LinkedMap &&other) noexcept {
        auto aaa = _pri;
        _pri = other._pri;
        other._pri = aaa;
        return *this;
    }

    bool empty() const noexcept {
        return _pri==0 || _pri->map.empty();
    }
    size_t size() const noexcept {
        return _pri ? _pri->map.size() : 0;
    }

    iterator find(const K &k) const {
        if(_pri==0) return iterator((Node*) _pri);
        auto it = _pri->map.find(k);
        if(it==_pri->map.end()) return iterator((Node*) _pri);
        return iterator(it->second);
    }
    const V operator()(const K &k) const {
        return (*this)[k];
    }
    const V operator[](const K &k) const {
        if(_pri==0) return V();
        auto it = _pri->map.find(k);
        if(it==_pri->map.end()) return V();
        return it->second->value.second;
    }
    V &operator[](const K &k) {
        if(_pri==0) _pri = new LinkedMapPri<K, V>;
        auto pair = _pri->map.emplace(k, nullptr);
        if(pair.second) {
            auto node = new Node{_pri, _pri->prev, {k, V()}};
            _pri->prev->next = node;
            _pri->prev = node;
            pair.first->second = node;
        }
        return pair.first->second->value.second;
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
    V remove(const K& k) {
        if(_pri==0) return V();
        auto it = _pri->map.find(k);
        if(it==_pri->map.end()) return V();
        auto node = it->second;
        _pri->map.erase(it);
        node->prev->next = node->next;
        node->next->prev = node->prev;
        node->next = 0;
        node->prev = 0;
        auto v = node->value.second;
        delete node;
        return v;
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

    bool operator==(const LinkedMap &other) const {
        if(_pri==other._pri) return true;
        auto siz = size();
        if(siz!=other.size()) return false;
        if(siz==0) return true;
        auto aaa = begin();
        auto bbb = other.begin();
        while(aaa!=end()) {
            if(aaa->first != bbb->first || aaa->second != bbb->second) return false;
            ++aaa;
            ++bbb;
        }
        return true;
    }
    bool operator!=(const LinkedMap &other) const {
        return ! (*this==other);
    }

    iterator begin() const {
        return iterator((Node*) (_pri ? _pri->next : 0));
    }
    iterator end() const {
        return iterator((Node*) _pri);
    }
    LinkedMapPri<K, V> *_pri = 0;
};

#endif // CPP_H
