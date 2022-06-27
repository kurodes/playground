//
// Created by lwh on 22-6-21.
//

#ifndef LIST_LOCKSINGLELIST_H
#define LIST_LOCKSINGLELIST_H

#include <mutex>
#include "List.h"

template<typename K, typename V>
class LockSingleList: public List<K, V> {
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

    LockSingleList(const K &min_key, const K &max_key) {
        head_ = new Node(min_key, V());
        tail_ = new Node(max_key, V());
        head_->next_ = tail_;
    }

    int Get(const K &key, V &value) {
        int iret = 0;
        Node *prev = head_;
        Node *curr = prev->next_;
        curr->mutex_lock.lock();
        while (key > curr->key_) {
            prev = curr;
            curr = curr->next_;
            prev->mutex_lock.unlock();
            curr->mutex_lock.lock();
        }
        if (key == curr->key_) {
            value = curr->value_;
        } else {
            iret = -1;
        }
        curr->mutex_lock.unlock();
        return iret;
    }

    int Put(const K &key, const V &value) {
        int iret = 0;
        Node *prev = head_;
        Node *curr = prev->next_;
        curr->mutex_lock.lock();
        while (key > curr->key_) {
            prev = curr;
            curr = curr->next_;
            prev->mutex_lock.unlock();
            curr->mutex_lock.lock();
        }
        if (key == curr->key_) {
            curr->mutex_lock.unlock();
            iret = -1;
        } else {
            curr->mutex_lock.unlock();
            prev->mutex_lock.lock();
            Node *tmp = new Node(key, value);
            tmp->next_ = curr;
            prev->next_ = tmp;
            prev->mutex_lock.unlock();
        }
        return iret;
    }

    int Del(const K &key) {
        int iret = 0;
        Node *prev = head_;
        Node *curr = prev->next_;
        curr->mutex_lock.lock();
        while (key > curr->key_) {
            prev = curr;
            curr = curr->next_;
            prev->mutex_lock.unlock();
            curr->mutex_lock.lock();
        }
        if (key == curr->key_) {
            curr->mutex_lock.unlock();
            prev->mutex_lock.lock();
            Node *ptr = curr;
            prev->next_ = curr->next_;
            prev->mutex_lock.unlock();
            /* do not free the node,
             * or the concurrent del test will cause segment fault */
            //free(ptr);
        } else {
            curr->mutex_lock.unlock();
            iret = -1;
        }
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

#endif //LIST_LOCKSINGLELIST_H
