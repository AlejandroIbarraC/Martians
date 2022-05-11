

#ifndef Martian_NODE_H
#define Martian_NODE_H


#include "Martian.h"
#include <stdlib.h>


static struct Martian_node* Head = NULL;

struct Martian_node{
    Martian* martian;
    struct Martian_node* next;
};

void insert(Martian* Martian);
Martian* find(int i);
int get_size ();
Martian* findMartianByID(int id);
void RTOSPriority();
void SRTNPriority();
void FCFSPriority();
void removeMartian(Martian *martian);

#endif //Martian_NODE_H
