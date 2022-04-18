#pragma once

#include <vector>

template<typename K, typename V, typename Hash = std::hash<K>, typename KeyEqual = std::equal_to<K>>
class HashTable {
public:
    class Iterator {
    public:
        Iterator(HashTable& iterator_owner, size_t ind) : iterator_owner_(iterator_owner), ind_(ind) {
        }

        std::pair<K, V>& operator*() {
            return iterator_owner_.elements_[ind_];
        }

        const std::pair<K, V>& operator*() const {
            return iterator_owner_.elements_[ind_];
        }

        std::pair<K, V>* operator->() {
            return &iterator_owner_.elements_[ind_];
        }

        const std::pair<K, V>* operator->() const {
            return &iterator_owner_.elements_[ind_];
        }

        Iterator& operator++() {
            ind_ = iterator_owner_.next_element_[ind_];
            return *this;
        }

        bool operator!=(const Iterator& it) const {
            return ind_ != it.ind_ || &iterator_owner_ != &it.iterator_owner_;
        }

    private:
        HashTable& iterator_owner_;
        size_t ind_;
    };

    class ConstIterator {
    public:
        ConstIterator(const HashTable& iterator_owner, size_t ind) : iterator_owner_(iterator_owner), ind_(ind) {
        }

        const std::pair<K, V>& operator*() const {
            return iterator_owner_.elements_[ind_];
        }

        const std::pair<K, V>* operator->() const {
            return &iterator_owner_.elements_[ind_];
        }

        ConstIterator& operator++() {
            ind_ = iterator_owner_.next_element_[ind_];
            return *this;
        }

        bool operator!=(const ConstIterator& it) const {
            return ind_ != it.ind_ || &iterator_owner_ != &it.iterator_owner_;
        }

    private:
        const HashTable& iterator_owner_;
        size_t ind_;
    };

    HashTable() : elements_(8), next_element_(8, 9), first_element_(8), last_element_(8), count_elements_inside_(0) {
    }

    template<typename Collection>
    HashTable(const Collection& collection) {
        for (auto &value : collection) {
            insert(value);
        }
    }

    HashTable(const HashTable& table) : elements_(table.elements_), next_element_(table.next_element_),
          first_element_(table.first_element_), last_element_(table.last_element_),
          count_elements_inside_(table.count_elements_inside_) {
    }

    HashTable(HashTable&& table) : first_element_(table.first_element_), last_element_(table.last_element_),
          count_elements_inside_(table.count_elements_inside_) {
        std::swap(elements_, table.elements_);
        std::swap(next_element_, table.next_element_);
    }

    HashTable& operator=(const HashTable& table) {
        elements_ = table.elements_;
        next_element_ = table.next_element_;
        first_element_ = table.first_element_;
        last_element_ = table.last_element_;
        count_elements_inside_ = table.count_elements_inside_;
        return *this;
    }

    HashTable& operator=(HashTable&& table) {
        std::swap(elements_, table.elements_);
        std::swap(next_element_, table.next_element_);
        last_element_ = table.last_element_;
        first_element_ = table.first_element_;
        count_elements_inside_ = table.count_elements_inside_;
        return *this;
    }

    V& operator[](const K& key) {
        for (size_t ind = Hash{}(key) % elements_.size(); ind < elements_.size(); ++ind) {
            if (next_element_[ind] > elements_.size()) {
                if (first_element_ == elements_.size()) {
                    first_element_ = ind;
                } else {
                    next_element_[last_element_] = ind;
                }

                last_element_ = ind;
                next_element_[ind] = elements_.size();
                elements_[ind].first = key;
                ++count_elements_inside_;
                if (count_elements_inside_ > elements_.size() * MAX_LOAD_FACTOR) {
                    Relocate();
                    return find(key)->second;
                }
                return elements_[ind].second;
            }

            if (KeyEqual{}(key, elements_[ind].first)) {
                return elements_[ind].second;
            }
        }

        for (size_t ind = 0; ; ++ind) {
            if (next_element_[ind] > elements_.size()) {
                if (first_element_ == elements_.size()) {
                    first_element_ = ind;
                } else {
                    next_element_[last_element_] = ind;
                }

                last_element_ = ind;
                next_element_[ind] = elements_.size();
                elements_[ind].first = key;
                ++count_elements_inside_;
                if (count_elements_inside_ > elements_.size() * MAX_LOAD_FACTOR) {
                    Relocate();
                    return find(key)->second;
                }
                return elements_[ind].second;
            }

            if (KeyEqual{}(key, elements_[ind].first)) {
                return elements_[ind].second;
            }
        }
    }

    const V& at(const K& key) const {
        for (size_t ind = Hash{}(key) % elements_.size(); ind < elements_.size(); ++ind) {
            if (next_element_[ind] > elements_.size()) {
                throw std::out_of_range("HashTable::at: key not found");
            }

            if (KeyEqual{}(key, elements_[ind].first)) {
                return elements_[ind].second;
            }
        }

        for (size_t ind = 0; ; ++ind) {
            if (next_element_[ind] > elements_.size()) {
                throw std::out_of_range("HashTable::at: key not found");
            }

            if (KeyEqual{}(key, elements_[ind].first)) {
                return elements_[ind].second;
            }
        }
    }

    V& at(const K& key) {
        for (size_t ind = Hash{}(key) % elements_.size(); ind < elements_.size(); ++ind) {
            if (next_element_[ind] > elements_.size()) {
                throw std::out_of_range("HashTable::at: key not found");
            }

            if (KeyEqual{}(key, elements_[ind].first)) {
                return elements_[ind].second;
            }
        }

        for (size_t ind = 0;; ++ind) {
            if (next_element_[ind] > elements_.size()) {
                throw std::out_of_range("HashTable::at: key not found");
            }

            if (KeyEqual{}(key, elements_[ind].first)) {
                return elements_[ind].second;
            }
        }
    }

    std::pair<Iterator, bool> insert(const std::pair<K, V>& value) {
        for (size_t ind = Hash{}(value.first) % elements_.size(); ind < elements_.size(); ++ind) {
            if (next_element_[ind] > elements_.size()) {
                if (first_element_ == elements_.size()) {
                    first_element_ = ind;
                } else {
                    next_element_[last_element_] = ind;
                }

                last_element_ = ind;
                next_element_[ind] = elements_.size();
                elements_[ind] = value;
                ++count_elements_inside_;
                if (count_elements_inside_ > elements_.size() * MAX_LOAD_FACTOR) {
                    Relocate();
                    return std::make_pair(find(value.first), true);
                }
                return std::make_pair(Iterator(*this, ind), true);
            }

            if (KeyEqual{}(value.first, elements_[ind].first)) {
                return std::make_pair(Iterator(*this, ind), false);
            }
        }

        for (size_t ind = 0;; ++ind) {
            if (next_element_[ind] > elements_.size()) {
                if (first_element_ == elements_.size()) {
                    first_element_ = ind;
                } else {
                    next_element_[last_element_] = ind;
                }

                last_element_ = ind;
                next_element_[ind] = elements_.size();
                elements_[ind] = value;
                ++count_elements_inside_;
                if (count_elements_inside_ > elements_.size() * MAX_LOAD_FACTOR) {
                    Relocate();
                    return std::make_pair(find(value.first), true);
                }
                return std::make_pair(Iterator(*this, ind), true);
            }

            if (KeyEqual{}(value.first, elements_[ind].first)) {
                return std::make_pair(Iterator(*this, ind), false);
            }
        }
    }

    template<typename... Args>
    std::pair<Iterator, bool> emplace(const Args&... args) {
        std::pair<K, V> value(args...);
        for (size_t ind = Hash{}(value.first) % elements_.size(); ind < elements_.size(); ++ind) {
            if (next_element_[ind] > elements_.size()) {
                if (first_element_ == elements_.size()) {
                    first_element_ = ind;
                } else {
                    next_element_[last_element_] = ind;
                }

                last_element_ = ind;
                next_element_[ind] = elements_.size();
                elements_[ind] = value;
                ++count_elements_inside_;
                if (count_elements_inside_ > elements_.size() * MAX_LOAD_FACTOR) {
                    Relocate();
                    return std::make_pair(find(value.first), true);
                }
                return std::make_pair(Iterator(*this, ind), true);
            }

            if (KeyEqual{}(value.first, elements_[ind].first)) {
                return std::make_pair(Iterator(*this, ind), false);
            }
        }

        for (size_t ind = 0;; ++ind) {
            if (next_element_[ind] > elements_.size()) {
                if (first_element_ == elements_.size()) {
                    first_element_ = ind;
                } else {
                    next_element_[last_element_] = ind;
                }

                last_element_ = ind;
                next_element_[ind] = elements_.size();
                elements_[ind] = value;
                ++count_elements_inside_;
                if (count_elements_inside_ > elements_.size() * MAX_LOAD_FACTOR) {
                    Relocate();
                    return std::make_pair(find(value.first), true);
                }
                return std::make_pair(Iterator(*this, ind), true);
            }

            if (KeyEqual{}(value.first, elements_[ind].first)) {
                return std::make_pair(Iterator(*this, ind), false);
            }
        }
    }

    Iterator find(const K& key) {
        for (size_t ind = Hash{}(key) % elements_.size(); ind < elements_.size(); ++ind) {
            if (next_element_[ind] > elements_.size()) {
                return end();
            }

            if (KeyEqual{}(key, elements_[ind].first)) {
                return Iterator(*this, ind);
            }
        }

        for (size_t ind = 0; ; ++ind) {
            if (next_element_[ind] > elements_.size()) {
                return end();
            }

            if (KeyEqual{}(key, elements_[ind].first)) {
                return Iterator(*this, ind);
            }
        }
    }

    ConstIterator find(const K& key) const {
        for (size_t ind = Hash{}(key) % elements_.size(); ind < elements_.size(); ++ind) {
            if (next_element_[ind] > elements_.size()) {
                return end();
            }

            if (KeyEqual{}(key, elements_[ind].first)) {
                return ConstIterator(*this, ind);
            }
        }

        for (size_t ind = 0; ; ++ind) {
            if (next_element_[ind] > elements_.size()) {
                return end();
            }

            if (KeyEqual{}(key, elements_[ind].first)) {
                return ConstIterator(*this, ind);
            }
        }
    }

    size_t size() const {
        return count_elements_inside_;
    }

    bool empty() const {
        return size() == 0;
    }

    void clear() {
        elements_.assign(elements_.size(), std::pair<K, V>{});
        next_element_.assign(elements_.size(), elements_.size() + 1);
        first_element_ = last_element_ = elements_.size();
        count_elements_inside_ = 0;
    }

    Iterator begin() {
        return Iterator(*this, first_element_);
    }

    Iterator end() {
        return Iterator(*this, elements_.size());
    }

    ConstIterator begin() const {
        return ConstIterator(*this, first_element_);
    }

    ConstIterator end() const {
        return ConstIterator(*this, elements_.size());
    }

private:
    static HashTable MakeWithCapacity(size_t size) {
        HashTable table;
        table.elements_.resize(size);
        table.next_element_.assign(size, size + 1);
        table.first_element_ = table.last_element_ = size;
        table.count_elements_inside_ = 0;
        return table;
    }

    void Relocate() {
        HashTable new_table = MakeWithCapacity(elements_.size() * 2);
        for (auto& element: *this) {
            new_table.insert(element);
        }
        *this = new_table;
    }

    std::vector<std::pair<K, V>> elements_;
    std::vector<size_t> next_element_;
    size_t first_element_;
    size_t last_element_;
    size_t count_elements_inside_;
    const double MAX_LOAD_FACTOR = 0.25;
};