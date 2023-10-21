//
// Created by Stepan Usatiuk on 21.10.2023.
//

#ifndef OS2_LISTQUEUE_HPP
#define OS2_LISTQUEUE_HPP


template<typename T>
class ListQueue {
public:
    ListQueue() {
    }

private:
    struct Node {
        T val;
        Node *next;
    };
    Node *head;
    Node *tail;
};


#endif//OS2_LISTQUEUE_HPP
