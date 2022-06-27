//
// Created by lwh on 22-6-18.
//

#ifndef LIST_COARSELOCKLIST_H
#define LIST_COARSELOCKLIST_H

#include <mutex>
#include "List.h"

template<typename K, typename V>
class CoarseLockList : public List<K, V> {
public:
    class Node {
    public:
        K key_;
        V value_;
        Node *next_;
        Node(K key, V value) : key_(key), value_(value), next_(nullptr) {}
    };
    Node *head_;
    Node *tail_;
    std::mutex mutex_lock;

    CoarseLockList(const K &min_key, const K &max_key) {
        head_ = new Node(min_key, V());
        tail_ = new Node(max_key, V());
        head_->next_ = tail_;
    }

    int Get(const K &key, V &value) {
        mutex_lock.lock();
        int iret = 0;
        Node *prev = head_;
        Node *curr = head_->next_;
        while (key > curr->key_) {
            prev = curr;
            curr = curr->next_;
        }
        if (key == curr->key_) {
            value = curr->value_;
        } else {
            iret = -1;
        }
        mutex_lock.unlock();
        return iret;
    }

    int Put(const K &key, const V &value) {
        mutex_lock.lock();
        int iret = 0;
        Node *prev = head_;
        Node *curr = head_->next_;
        while (key > curr->key_) {
            prev = curr;
            curr = curr->next_;
        }
        if (key == curr->key_) {
            iret = -1;
        } else {
            Node *tmp = new Node(key, value);
            tmp->next_ = curr;
            prev->next_ = tmp;
        }
        mutex_lock.unlock();
        return iret;
    }

    int Del(const K &key) {
        mutex_lock.lock();
        int iret = 0;
        Node *prev = head_;
        Node *curr = head_->next_;
        while (key > curr->key_) {
            prev = curr;
            curr = curr->next_;
        }
        if (key == curr->key_) {
            Node *tmp = prev->next_;
            prev->next_ = curr->next_;
            free(tmp);
        } else {
            iret = -1;
        }
        mutex_lock.unlock();
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

#endif //LIST_COARSELOCKLIST_H
