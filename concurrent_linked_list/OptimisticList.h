//
// Created by lwh on 22-6-21.
//

#ifndef LIST_OPTIMISTICLIST_H
#define LIST_OPTIMISTICLIST_H

#include "List.h"
#include <mutex>

template<typename K, typename V>
class OptimisticList : public List<K, V> {
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

    OptimisticList(const K &min_key, const K &max_key) {
        head_ = new Node(min_key, V());
        tail_ = new Node(max_key, V());
        head_->next_ = tail_;
    }

    bool Validate(Node *prev, Node *curr) {
        Node *v_ptr = head_;
        while (v_ptr->key_ < prev->key_) v_ptr = v_ptr->next_;
        if (v_ptr == prev && prev->next_ == curr) return true;
        return false;
    }

    int Get(const K &key, V &value) {
        int iret = 0;
        Node *prev = head_;
        Node *curr = prev->next_;
        while (key > curr->key_) {
            prev = curr;
            curr = curr->next_;
        }
        prev->mutex_lock.lock();
        curr->mutex_lock.lock();
        if (Validate(prev, curr))
        {
            if (key == curr->key_) {
                value = curr->value_;
            } else {
                iret = -1;
            }
            prev->mutex_lock.unlock();
            curr->mutex_lock.unlock();
            return iret;
        } else {
            prev->mutex_lock.unlock();
            curr->mutex_lock.unlock();
            return Get(key, value);
        }
//        curr->mutex_lock.lock();
//        if (key == curr->key_) {
//            value = curr->value_;
//        } else {
//            iret = -1;
//        }
//        curr->mutex_lock.unlock();
//        return iret;
    }

    int Put(const K &key, const V &value) {
        int iret = 0;
        Node *prev = head_;
        Node *curr = prev->next_;
        while (key > curr->key_) {
            prev = curr;
            curr = curr->next_;
        }
        prev->mutex_lock.lock();
        curr->mutex_lock.lock();
        if (Validate(prev, curr)) {
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
        } else {
            prev->mutex_lock.unlock();
            curr->mutex_lock.unlock();
            return Put(key, value);
        }
    }

    int Del(const K &key) {
        int iret = 0;
        Node *prev = head_;
        Node *curr = prev->next_;
        while (key > curr->key_) {
            prev = curr;
            curr = curr->next_;
        }
        prev->mutex_lock.lock();
        curr->mutex_lock.lock();
        if (Validate(prev, curr)) {
            if (key == curr->key_) {
//                Node *ptr = curr;
                prev->next_ = curr->next_;
                /* cannot free the node immediately,
                 * shall wait for all traversing leave */
                //free(ptr);
            } else {
                iret = -1;
            }
            prev->mutex_lock.unlock();
            curr->mutex_lock.unlock();
            return iret;
        } else {
            prev->mutex_lock.unlock();
            curr->mutex_lock.unlock();
            return Del(key);
        }
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


#endif //LIST_OPTIMISTICLIST_H
