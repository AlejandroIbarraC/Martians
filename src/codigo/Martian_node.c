//
// Created by Gaming on 14/6/2020.
//

#include "Martian_node.h"
#include <stdio.h>
void insert(Martian* Martian){
    struct Martian_node* tmp = (struct Martian_node *) malloc(sizeof(struct Martian_node));
    tmp->martian = Martian;
    tmp->next = Head;
    Head = tmp;
}




// encuentra el puntero a un index de la lista
// entradas:
Martian* find(int index){
    struct Martian_node* tmp = Head;
    if (Head == NULL) {
        return NULL;
    }
    for (int i=0;i < index;i++) {
        if(tmp->next == NULL) {
            return NULL;
        } else {
            tmp = tmp->next;
        }
    }
    return tmp->martian;
}


Martian* findMartianByID(int id){
    struct Martian_node* tmp = Head;
    if (Head == NULL) {
        return NULL;
    }
    while (1){
        if (tmp->martian->id==id){
            return tmp->martian;
        }
        else if(tmp->next == NULL) {
            return NULL;
        }
        else {
            tmp = tmp->next;
        }
    }
}

Martian* deleteMartian(int id){
    struct Martian_node* tmp = Head;
    if (Head == NULL) {
        return NULL;
    }
    while (1){
        if (tmp->next->martian->id==id){
            tmp->martian;
        }
        else if(tmp->next == NULL) {
            return NULL;
        }
        else {
            tmp = tmp->next;
        }
    }
}


int get_size (){
    struct Martian_node* tmp = Head;
    int size=0;
    while (1) {
        if(tmp == NULL) {
            return size;
        } else {
            tmp = tmp->next;
            size++;
        }
    }
}

#define UP 0
#define DOWN 1
#define LEFT 2
#define RIGHT 3


// perform the bubble sort
void RTOSPriority() {
    // loop to access each array element
    int size= get_size();
    int array [size];
    for (int i=0; i<size;i++){
        array[i]=i;
    }
    for (int step = 0; step < size - 1; ++step) {
        for (int i = 0; i < size - step - 1; ++i) {
            if (findMartianByID(array[i])->period >findMartianByID(array[i+1])->period) {
                int temp = array[i];
                array[i] = array[i + 1];
                array[i + 1] = temp;
            }
        }
    }
    printf("orden: \n");
    for (int i=0; i<size;i++){
        findMartianByID(array[i])->priority=i;
        printf("marciano #%d\n", array[i]);
    }
}
void SRTNPriority() {
    // loop to access each array element
    int size= get_size();
    int array [size];
    for (int i=0; i<size;i++){
        array[i]=i;
    }
    for (int step = 0; step < size - 1; ++step) {
        for (int i = 0; i < size - step - 1; ++i) {
            if (findMartianByID(array[i])->executiontime >findMartianByID(array[i+1])->executiontime) {
                int temp = array[i];
                array[i] = array[i + 1];
                array[i + 1] = temp;
            }
        }
    }
    printf("orden: \n");
    for (int i=0; i<size;i++){
        findMartianByID(array[i])->priority=i;
        printf("marciano #%d\n", array[i]);
    }
}
void FCFSPriority() {
    // loop to access each array element
    int size= get_size();
    int array [size];
    for (int i=0; i<size;i++){
        array[i]=i;
    }
    for (int step = 0; step < size - 1; ++step) {
        for (int i = 0; i < size - step - 1; ++i) {
            if (findMartianByID(array[i])->arrivalTime >findMartianByID(array[i+1])->arrivalTime) {
                int temp = array[i];
                array[i] = array[i + 1];
                array[i + 1] = temp;
            }
        }
    }
    printf("orden: \n");
    for (int i=0; i<size;i++){
        findMartianByID(array[i])->priority=i;
        printf("marciano #%d\n", array[i]);
    }
}