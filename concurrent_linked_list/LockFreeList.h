//
// Created by lwh on 22-6-22.
//

#ifndef LIST_LOCKFREELIST_H
#define LIST_LOCKFREELIST_H

#include <atomic>

#include "List.h"

#define CAS(ptr, oldval, newval) \
    (__sync_bool_compare_and_swap(ptr, oldval, newval))

template<typename K, typename V>
class LockFreeList : public List<K, V> {
public:
    class Node {
    public:
        K key_;
        V value_;
        uintptr_t tag_next_;

        Node(K key, V value) : key_(key), value_(value), tag_next_(0) {}

        Node *Next() {
            return (Node *) ((tag_next_ << 16) >> 16);
        }

        uint16_t Tag() {
            return (uint16_t) (tag_next_ >> 48);
        }
    };

    Node *head_;
    Node *tail_;

    LockFreeList(const K &min_key, const K &max_key) {
        head_ = new Node(min_key, V());
        tail_ = new Node(max_key, V());
        head_->tag_next_ = TagPtr(tail_, 0);
        tail_->tag_next_ = TagPtr(nullptr, 0);
    }

    inline uintptr_t TagPtr(Node *ptr, uint16_t data) {
        return ((uintptr_t) ptr | (uintptr_t) data << 48);
    }

    inline uintptr_t TagPtr(uintptr_t ptr, uint16_t data) {
        return ((uintptr_t) ptr | (uintptr_t) data << 48);
    }

    int Get(const K &key, V &value) {
        int iret = 0;
        Node *prev = head_;
        Node *curr = prev->Next();
        while (key > curr->key_) {
            prev = curr;
            curr = curr->Next();
        }
        if (key == curr->key_ && curr->Tag() == 0) {
            value = curr->value_;
        } else {
            iret = -1;
        }
        return iret;
    }

    int Put(const K &key, const V &value) {
        retry:
        Node *prev = head_;
        Node *curr = prev->Next();
        while (true) {
            while (curr->Tag() == 1) {
                /* do not dereference pointers, why */
                Node *succ = curr->Next();
                if (!CAS(&(prev->tag_next_), TagPtr(curr, 0), TagPtr(succ, 0))) goto retry;
                curr = succ;
//                if (!CAS(&(prev->tag_next_), TagPtr(curr, 0), TagPtr(curr->Next(), 0))) goto retry;
//                curr = curr->Next();
            }
            if (key <= curr->key_) break;
            prev = curr;
            curr = curr->Next();
        }
        if (key == curr->key_ && curr->Tag() == 0) {
            return -1;
        } else {
            Node *tmp = new Node(key, value);
            tmp->tag_next_ = TagPtr(curr, 0);
            if (!CAS(&(prev->tag_next_), TagPtr(curr, 0), TagPtr(tmp, 0))) {
                free(tmp);
                return Put(key, value);
            }
        }
        return 0;
    }

    int Del(const K &key) {
        Node *prev = head_;
        Node *curr = prev->Next();
        while (key > curr->value_) {
            prev = curr;
            curr = curr->Next();
        }
        if (key == curr->key_) {
            auto tmp = curr->tag_next_;
            if (CAS(&(curr->tag_next_), TagPtr(tmp, 0), TagPtr(tmp, 1))) {  // logically delete
                return 0;
            }
        }
        return -1;
    }

//    int Del(const K &key) {
//        retry:
//        Node *prev = head_;
////        Node *curr = prev->Next();
//        uintptr_t curr_tag = prev->tag_next_;
//        while (true) {
//            while (curr->Tag() == 1) {
//                if (false == CAS(&(prev->tag_next_), TagPtr(curr, 0), TagPtr(curr->tag_next_, 0))) goto retry;
//                curr = prev->Next();
//            }
//            if (key <= curr->key_) break;
//            prev = curr;
//            curr = curr->Next();
//        }
//        if (key == curr->key_) {
//            if (true ==
//                CAS(&(curr->tag_next_), TagPtr(curr->tag_next_, 0), TagPtr(curr->tag_next_, 1))) {  // logically delete
//                /* the remove op has succeeded atomically*/
//                if (true == CAS(&(prev->tag_next_), TagPtr(curr, 0), curr->tag_next_)) {  // physically delete
//                    /* we should not assume the thread will physically remove the deleted node successfully,
//                     * because two CAS are not atomic, and the thread may halt here forever.
//                     * To ensure lock-free (one failed thread will not affect others),
//                     * we use the "help" mechanism which everyone can physically remove the deleted node during traversing,
//                     * so that other thread won't be blocked on the logically deleted nodes
//                     */
//                    /* other thread certainly may read the deleted nodes right now (no mutex),
//                     * so we cannot free space right away
//                     */
////                    free(curr);
//                }
//                return 0;
//            }
//        }
//        return -1;
//    }

    void ShowTenThreadUnsafe() {
        int count = 10;
        for (Node *n = head_->Next(); n != tail_ && count > 0; n = n->Next()) {
            std::cout << n->key_ << " : " << n->value_ << std::endl;
            --count;
        }
    }

    int CountThreadUnsafe() {
        int count = 0;
        Node *n;
        for (n = head_->Next(); n != tail_; n = n->Next()) {
            /* check deleted mark*/
            if (n->Tag() == 0)
                ++count;
        }
        return count;
    }
};

#endif  // LIST_LOCKFREELIST_H
