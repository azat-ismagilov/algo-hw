#include <vector>
#include <initializer_list>
#include <list>
#include <stdexcept>
#include <iterator>

template<class KeyType, class ValueType, class Hash = std::hash<KeyType> >
class HashMap {
    using MyPair = typename std::pair<const KeyType, ValueType>;

  private:
    Hash hasher;

    std::vector<std::list<MyPair>> data;
    size_t keyCount = 0;

    const double MaxLoadFactor = 1.618033988; // Golden ratio
    const double MinLoadFactor = MaxLoadFactor * MaxLoadFactor;

    void rehash(const size_t bucketSize) {
        std::vector<std::list<MyPair>> old_data(std::move(data));
        keyCount = 0;
        data.resize(bucketSize);
        for (const auto &bucket : old_data) {
            for (const auto &element : bucket) {
                insert(element);
            }
        }
    }

    size_t bucketIndex(const KeyType &key) const {
        return hasher(key) % data.size();
    }

  public:
    explicit HashMap(Hash _hasher = Hash()) : hasher(_hasher) {
        clear();
    }

    template<typename iter>
    HashMap(iter begin, iter end, Hash _hasher = Hash()) : hasher(_hasher) {
        clear();
        rehash(std::distance(begin, end) * MaxLoadFactor + 1);
        for (auto it = begin; it != end; ++it) {
            insert(*it);
        }
    }

    HashMap(const std::initializer_list<MyPair> &list,
            Hash _hasher = Hash()) : hasher(_hasher) {
        clear();
        rehash(std::distance(list.begin(), list.end()) * MaxLoadFactor + 1);
        for (auto it = list.begin(); it != list.end(); ++it) {
            insert(*it);
        }
    }

    HashMap(const HashMap &other) {
        hasher = other.hasher;
        clear();
        rehash(other.data.size());
        for (const auto &it : other) {
            insert(it);
        }
    }

    HashMap& operator=(const HashMap &other) {
        if (&other != this) {
            hasher = other.hasher;
            clear();
            rehash(other.data.size());
            for (const auto &it : other) {
                insert(it);
            }
        }
        return *this;
    }

    Hash hash_function() const {
        return hasher;
    }

    size_t size() const {
        return keyCount;
    }

    bool empty() const {
        return (size() == 0);
    }

    void insert(const MyPair &v) {
        auto &bucket = data[bucketIndex(v.first)];

        for (const auto &element : bucket) {
            if (element.first == v.first) {
                return;
            }
        }
        bucket.push_back(v);
        ++keyCount;
        if (keyCount >= data.size()) {
            rehash(static_cast<size_t>(keyCount * MaxLoadFactor + 1));
        }
    }

    void erase(const KeyType& key) {
        auto &bucket = data[bucketIndex(key)];

        auto it = bucket.begin();
        while (it != bucket.end() && it->first != key) {
            ++it;
        }
        if (it == bucket.end()) {
            return;
        }

        bucket.erase(it);
        --keyCount;
        if (keyCount * MinLoadFactor < bucket.size()) {
            //if the number of blocks needs to be recalculated
            rehash(static_cast<size_t>(keyCount * MaxLoadFactor + 1));
        }
    }

    ValueType& operator[] (const KeyType& key) {
        auto it = find(key);
        if (it == end()) {
            insert({key, ValueType()});
            it = find(key);
        }
        return it->second;
    }

    const ValueType& at(const KeyType& key) const {
        auto &bucket = data[bucketIndex(key)];

        for (const auto &element : bucket) {
            if (element.first == key) {
                return element.second;
            }
        }

        throw std::out_of_range("There is no such key");
    }

    void clear() {
        data.clear();
        data.resize(1);
        keyCount = 0;
    }

    class iterator : public std::iterator
        <std::forward_iterator_tag, MyPair> {
        using BucketIterator =
            typename std::vector<std::list<MyPair>>::iterator;
        using ElementIterator =
            typename std::list<MyPair>::iterator;
      private:
        BucketIterator bucketIt;
        ElementIterator elementIt;
        HashMap* map;

      public:
        iterator(const BucketIterator _bucketIt, const ElementIterator _elementIt,
                 HashMap* _map) :
            bucketIt(_bucketIt), elementIt(_elementIt), map(_map) {}

        explicit iterator(HashMap* _map) : bucketIt(nullptr), elementIt(nullptr),
            map(_map) {}

        iterator() : bucketIt(nullptr), elementIt(nullptr), map(nullptr) {}

        iterator& operator++() {
            if (bucketIt == map->data.end()) {
                return *this;
            }
            ++elementIt;

            while (bucketIt != map->data.end() && elementIt == bucketIt->end()) {
                ++bucketIt;
                if (bucketIt == map->data.end()) {
                    elementIt = {};
                } else {
                    elementIt = bucketIt->begin();
                }
            }

            return *this;
        }

        iterator operator++() {
            iterator it(*this);
            ++(*this);
            return it;
        }

        const MyPair& operator*() {
            return *elementIt;
        }

        ElementIterator operator->() {
            return elementIt;
        }

        bool operator==(const iterator &it) const {
            return (this->bucketIt == it.bucketIt) && (this->elementIt == it.elementIt);
        }

        bool operator!=(const iterator &it) const {
            return !(*this == it);
        }
    };

    class const_iterator : public std::iterator
        <std::forward_iterator_tag, const MyPair> {
        using BucketIterator =
            typename std::vector<std::list<MyPair>>::const_iterator;
        using ElementIterator =
            typename std::list<MyPair>::const_iterator;
      private:
        BucketIterator bucketIt;
        ElementIterator elementIt;
        const HashMap* map;

      public:
        const_iterator(const BucketIterator _bucketIt, const ElementIterator _elementIt,
                       const HashMap* _map) :
            bucketIt(_bucketIt), elementIt(_elementIt), map(_map) {}

        explicit const_iterator(const HashMap* _map) : bucketIt(nullptr),
            elementIt(nullptr),
            map(_map) {}

        const_iterator() : bucketIt(nullptr), elementIt(nullptr), map(nullptr) {}

        const_iterator& operator++() {
            if (bucketIt == map->data.end()) {
                return *this;
            }
            ++elementIt;

            while (bucketIt != map->data.end() && elementIt == bucketIt->end()) {
                ++bucketIt;
                if (bucketIt == map->data.end()) {
                    elementIt = {};
                } else {
                    elementIt = bucketIt->begin();
                }
            }

            return *this;
        }

        const_iterator operator++(int) {
            const_iterator it(*this);
            ++(*this);
            return it;
        }

        const MyPair& operator*() {
            return *elementIt;
        }

        ElementIterator operator->() {
            return elementIt;
        }

        bool operator==(const const_iterator &it) const {
            return (this->bucketIt == it.bucketIt) && (this->elementIt == it.elementIt);
        }

        bool operator!=(const const_iterator &it) const {
            return (this->bucketIt != it.bucketIt) || (this->elementIt != it.elementIt);
        }
    };

    iterator begin() {
        auto it = data.begin();
        while (it != data.end() && it->begin() == it->end()) {
            ++it;
        }
        if (it == data.end()) {
            return end();
        }
        return iterator(it, it->begin(), this);
    }

    iterator end() {
        return iterator(data.end(), {}, this);
    }

    const_iterator begin() const {
        auto it = data.begin();
        while (it != data.end() && it->begin() == it->end()) {
            ++it;
        }
        if (it == data.end()) {
            return end();
        }
        return const_iterator(it, it->begin(), this);
    }

    const_iterator end() const {
        return const_iterator(data.end(), {}, this);
    }

    iterator find(const KeyType &key) {
        auto &bucket = data[bucketIndex(key)];

        auto it = bucket.begin();
        while (it != bucket.end()) {
            if (it->first == key) {
                break;
            }
            ++it;
        }
        if (it == bucket.end()) {
            return end();
        }

        return iterator(data.begin() + bucketIndex(key), it, this);
    }

    const_iterator find(const KeyType &key) const {
        auto &bucket = data[bucketIndex(key)];

        auto it = bucket.begin();
        while (it != bucket.end()) {
            if (it->first == key) {
                break;
            }
            ++it;
        }
        if (it == bucket.end()) {
            return end();
        }

        return const_iterator(data.begin() + bucketIndex(key), it, this);
    }
};