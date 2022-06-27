//
// Created by lwh on 22-6-20.
//

#include <iostream> 

#ifndef LIST_LIST_H
#define LIST_LIST_H

template<typename K, typename V>
class List {
public:
    virtual int Get(const K &key, V &value)=0;
    virtual int Put(const K &key, const V &value)=0;
    virtual int Del(const K &key)=0;
    virtual void ShowTenThreadUnsafe()=0;
    virtual int CountThreadUnsafe()=0;
};

#endif //LIST_LIST_H
