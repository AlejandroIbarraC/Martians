

#include "Martian_node.h"
#include <stdio.h>
void insert(Martian* Martian){
    struct Martian_node* tmp = (struct Martian_node *) malloc(sizeof(struct Martian_node));
    if (get_size()==0){
        Head=NULL;
    }
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
        array[i]=find(i)->id;
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
        array[i]=find(i)->id;
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
        array[i]=find(i)->id;
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
    for (int i=0; i<get_size();i++){
        findMartianByID(array[i])->priority=i;
        printf("marciano #%d\n", array[i]);
    }
}

void removeMartian(Martian *martian) {
    printf("Size a %d\n",get_size());
    struct Martian_node* martiaNode = Head;
    for (int i=0;i < get_size();i++) {
        if (martiaNode->martian->id==martian->id){
            break;
        }
        else if(martiaNode->next == NULL) {
            break;
        }
        else {
            martiaNode = martiaNode->next;
        }
    }

    struct Martian_node *martians = Head;
    if (martian->id== martians->martian->id) {
            // List has more than 1 item. Can delete head.
            Head = martians->next;


    } else {
        struct Martian_node *tmp = Head;
        while (tmp->next != NULL) {

            if (martian->id == tmp->next->martian->id) {
                tmp->next = tmp->next->next;
                break;
            }
            tmp = tmp->next;
        }
    }
}