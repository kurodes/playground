//
// Created by lwh on 22-6-21.
//

#ifndef LIST_LOCKCOUPLINGLIST_H
#define LIST_LOCKCOUPLINGLIST_H

#include "List.h"
#include <mutex>

template<typename K, typename V>
class LockCouplingList: public List<K, V> {
public:
    class Node {
    public:
        K key_;
        V value_;
        std::mutex mutex_lock;
        Node *next_;
        Node(K key, V value) : key_(key), value_(value), next_(nullptr) {}
    };
    Node *head_;
    Node *tail_;
    std::mutex mutex_lock;

    LockCouplingList(const K &min_key, const K &max_key) {
        head_ = new Node(min_key, V());
        tail_ = new Node(max_key, V());
        head_->next_ = tail_;
    }

    int Get(const K &key, V &value) {
        int iret = 0;
        Node *prev = head_;
        prev->mutex_lock.lock();
        Node *curr = prev->next_;
        curr->mutex_lock.lock();
        while (key > curr->key_) {
            prev->mutex_lock.unlock();
            prev = curr;
            curr = curr->next_;
            curr->mutex_lock.lock();
        }
        if (key == curr->key_) {
            value = curr->value_;
        } else {
            iret = -1;
        }
        prev->mutex_lock.unlock();
        curr->mutex_lock.unlock();
        return iret;
    }

    int Put(const K &key, const V &value) {
        int iret = 0;
        Node *prev = head_;
        prev->mutex_lock.lock();
        Node *curr = prev->next_;
        curr->mutex_lock.lock();
        while (key > curr->key_) {
            prev->mutex_lock.unlock();
            prev = curr;
            curr = curr->next_;
            curr->mutex_lock.lock();
        }
        if (key == curr->key_) {
            iret = -1;
        } else {
            Node *tmp = new Node(key, value);
            tmp->next_ = curr;
            prev->next_ = tmp;
        }
        prev->mutex_lock.unlock();
        curr->mutex_lock.unlock();
        return iret;
    }

    int Del(const K &key) {
        int iret = 0;
        Node *prev = head_;
        prev->mutex_lock.lock();
        Node *curr = prev->next_;
        curr->mutex_lock.lock();
        while (key > curr->key_) {
            prev->mutex_lock.unlock();
            prev = curr;
            curr = curr->next_;
            curr->mutex_lock.lock();
        }
        if (key == curr->key_) {
            Node *ptr = curr;
            prev->next_ = curr->next_;
            free(ptr);
        } else {
            iret = -1;
        }
        prev->mutex_lock.unlock();
        curr->mutex_lock.unlock();
        return iret;
    }

    void ShowTenThreadUnsafe() {
        int count = 10;
        for (Node *n = head_->next_; n != tail_ && count > 0; n = n->next_) {
            std::cout << n->key_ << " : " << n->value_ << std::endl;
            --count;
        }
    }

    int CountThreadUnsafe() {
        int count = 0;
        Node *n;
        for (n = head_->next_; n != tail_; n = n->next_) {
            ++count;
        }
        return count;
    }
};

#endif //LIST_LOCKCOUPLINGLIST_H
