/*
 * notes.cpp
 *
 *  Created on: Oct 21, 2008
 *      Author: weng
 */

#include <iostream>
#include <fstream>
#include <string>

#include <algorithm>

#include <vector>
#include <list>
#include <map>
#include <set>

#include <cassert>

using namespace std;

// function object for printing
template<class T> struct PRINT {
        void operator()(T t) {
            cout << t << " ";
        }
};

void algorithms_demo() {
    static PRINT<int> print_int; // constructed exactly once
    vector<int> integers;

    // fill vector with integers
    for (int i = 0; i < 10; ++i)
        integers.push_back(i);
}

void vector_demo() {
    static PRINT<int> print_int; // constructed exactly once
    vector<int> integers;

    // fill vector with integers
    for (int i = 0; i < 10; ++i)
        integers.push_back(i);
    for_each(integers.begin(), integers.end(), print_int);
    cout << endl;
    integers[5] = 20; // random access; change value
    for_each(integers.begin(), integers.end(), print_int);
    cout << endl;
}

void list_demo() {
    static PRINT<string> print_string;
    list<string> strings;

    // fill vector with strings
    strings.push_back("one");
    strings.push_back("two");
    strings.push_back("three");
    strings.push_back("four");
    strings.push_back("five");

    for_each(strings.begin(), strings.end(), print_string);
    cout << endl;
    // find the string "three" and then change it
    list<string>::iterator it = find(strings.begin(), strings.end(),
            "three");
    *it = "eight";
    for_each(strings.begin(), strings.end(), print_string);
    cout << endl;
}

void map_demo() {
    map<string, int> phonebook;

    // insert name, telephone numbers
    phonebook["Weng"] = 5935439;
    phonebook["Alice"] = 5543399;
    phonebook["Bob"] = 4561234;
    phonebook["Charlie"] = 8289933;
    phonebook["Damien"] = 7789270;
    phonebook["Brian"] = 8988332;

    // retrieve telephone numbers
    cout << "Charlie" << " " << phonebook["Charlie"] << endl;
    cout << "Alice" << " " << phonebook["Alice"] << endl;
    cout << "Brian" << " " << phonebook["Brian"] << endl;
    cout << "Weng" << " " << phonebook["Weng"] << endl;
    cout << "Damien" << " " << phonebook["Damien"] << endl;
    cout << "Bob" << " " << phonebook["Bob"] << endl;

    // erase Brian
    phonebook.erase("Brian");

    // retrieve with non-existent name (should get default number)
    if (int num = phonebook["Brian"] == 0) // default number
    cout << "Brian is not on the phonebook" << endl;
    else
    cout << "Brian" << " " << num << endl;

    // find Charlie and change his number
    map<string, int>::iterator it = phonebook.find("Charlie");
    cout << "Charlie's old number: " << it->second << endl;
    it->second = 7289933;
    cout << "Charlie's new number: " << it->second << endl;
}

void set_demo() {
    static PRINT<int> print;
    set<int, less<int> > integers;

    // fill vector with integers
    for (int i = 0; i < 10; ++i)
        integers.insert(i);

    for_each(integers.begin(), integers.end(), print);
    cout << endl;

    // insert duplicate integer
    integers.insert(5);

    for_each(integers.begin(), integers.end(), print);
    cout << endl;

    set<int, less<int> > left_integers;
    for (int i = 0; i < 7; ++i)
        left_integers.insert(i);
    cout << "left: ";
    for_each(left_integers.begin(), left_integers.end(), print);
    cout << endl;

    set<int, less<int> > right_integers;
    for (int i = 3; i < 10; ++i)
        right_integers.insert(i);
    cout << "right: ";
    for_each(right_integers.begin(), right_integers.end(), print);
    cout << endl;

    // take union
    set<int, less<int> > union_integers;
    set_union(left_integers.begin(), left_integers.end(),
            right_integers.begin(), right_integers.end(), inserter(
                    union_integers, union_integers.begin()), less<int>());
    cout << "union: ";
    for_each(union_integers.begin(), union_integers.end(), print);
    cout << endl;

    // same interface for intersection, difference and symmetric difference
}

class BASE {
    public:
        BASE() {
            cout << "Constructing base class." << endl;
        }
        virtual void print() {
            cout << "This is the base class." << endl;
        }
};

class DERIVED1 : public BASE {
    public:
        DERIVED1() {
            cout << "Constructing first derived class." << endl;
        }
        void print() {
            cout << "This is the first derived class." << endl;
        }
};

class DERIVED2 : public BASE {
    public:
        DERIVED2() {
            cout << "Constructing second derived class." << endl;
        }
        void print() {
            cout << "This is the second derived class." << endl;
        }
};

void class_demo() {
    DERIVED1 d1;
    DERIVED2 d2;
    d1.print();
    d2.print();
    DERIVED1 d11, d12, d13;
    DERIVED2 d21, d22, d23;
    vector<BASE *> vec;
    vec.push_back(&d11);
    vec.push_back(&d12);
    vec.push_back(&d23);
    vec.push_back(&d22);
    vec.push_back(&d13);
    vec.push_back(&d21);
    for_each(vec.begin(), vec.end(), mem_fun(&BASE::print));
}

template <class T> struct LINKED_LIST {
        LINKED_LIST * next;
        LINKED_LIST * prev;
        T info;
};

void linked_list_demo() {
    // create a linked list of integers
    // first node needs special handling
    LINKED_LIST<int> * integers = new LINKED_LIST<int>;
    LINKED_LIST<int> * head = integers; // keep head
    integers->prev = 0;
    integers->info = 0;
    for (int i = 1; i < 10; ++i) {
        integers->next = new LINKED_LIST<int>;
        LINKED_LIST<int> * prev = integers;
        integers = integers->next;
        integers->prev = prev;
        integers->info = i;
    }
    // last node needs special handling
    integers->next = 0;

    // traverse and print linked list
    for (LINKED_LIST<int> * it = head; it != 0; it = it->next)
        cout << it->info << " ";
    cout << endl;

    // reverse linked list
    for (LINKED_LIST<int> * it = head; it != 0; it = it->prev) {
        head = it;
        // swap prev and next pointers
        LINKED_LIST<int> * tmp = it->prev;
        it->prev = it->next;
        it->next = tmp;
    }

    // traverse and print linked list
    for (LINKED_LIST<int> * it = head; it != 0; it = it->next)
        cout << it->info << " ";
    cout << endl;

    // remove node with value 5
    for (LINKED_LIST<int> * it = head; it != 0; it = it->next)
        if (it->info == 5) {
            LINKED_LIST<int> * prev = it->prev;
            LINKED_LIST<int> * next = it->next;
            prev->next = next;
            next->prev = prev;
            delete it;
            break;
        }

    // traverse and print linked list
    for (LINKED_LIST<int> * it = head; it != 0; it = it->next)
        cout << it->info << " ";
    cout << endl;

    // insert node of value 10 after node of value 4
    for (LINKED_LIST<int> * it = head; it != 0; it = it->next)
        if (it->info == 4) {
            LINKED_LIST<int> * prev = it;
            LINKED_LIST<int> * next = it->next;
            LINKED_LIST<int> * new_elt = new LINKED_LIST<int>;
            prev->next = new_elt;
            new_elt->prev = prev;
            new_elt->next = next;
            new_elt->info = 10;
            next->prev = new_elt;
            break;
        }

    // traverse and print linked list
    for (LINKED_LIST<int> * it = head; it != 0; it = it->next)
        cout << it->info << " ";
    cout << endl;
}

class ERROR {
        string const _msg;
    public:
        ERROR(string const msg) :
            _msg(msg) {
        }
        void show() const {
            cout << _msg << endl;
        }
};
void exception_demo() {
    try {
        throw ERROR("The is a test!");
    } catch(ERROR const e) {
        e.show();
    }
}

class CONTAINED {
        string const _s;
    public:
        CONTAINED(string const s) :
            _s(s) {
            cout << "Constructing " << _s << endl;
        }
        ~CONTAINED(){
            cout << "Destroying " << _s << endl;
        }
};

class CONTAINER {
        CONTAINED const member1;
        CONTAINED const member2;
    public:
        CONTAINER() :
            member1("m1"), member2("m2") {
            cout << "Constructing container" << endl;
        }
        ~CONTAINER(){
            cout << "Destroying container" << endl;
        }
};

void class_construction_demo(){
    CONTAINER con;
}
