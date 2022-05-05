#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pthread.h"
#include <time.h>
#include <unistd.h>
#include <limits.h>
#include "Martian.h"
#include "Martian_node.h"

#define EDF 1
#define RM 2

#define FCFS 1
#define Prioridad 2
#define SRTN 3

#define RTOS 1
#define INTERACTIVE 2


pthread_mutex_t mutex;
pthread_mutex_t mutexMain;
int counter = 0;
int currentMartian=-1;
int mode=RM;
int scheduler=FCFS;
int systemType = INTERACTIVE;
int lastTime=0;
int countMartians=0;
void updateMartiansToReady(clock_t initialTime, clock_t finalTIme);
int getNextCurrentMartian();
Martian* createMartian(int executionTime, int period, int priority, int arrivalTime);
void setPriority(Martian* martian);

void updateMartiansToReady(clock_t initialTime, clock_t finalTIme) {
    int tiempoTotal;
    for (int i = 0; i < get_size(); i++) {
        Martian *martian = find(i);
        tiempoTotal = (int) (finalTIme - initialTime) / CLOCKS_PER_SEC;
        if (tiempoTotal % martian->period == 0) {
            if (martian->ready==1 && lastTime!=tiempoTotal){
                printf("Error no se pudo calendarizar a marciano #%d correctamente, tiempo: %d segundos\n",martian->id, tiempoTotal);
            }
            martian->ready = 1;
        }
    }
}

int getNextCurrentMartian(){
    int listSize= get_size();
    int martianid=-1;
    int currentPriority=INT_MAX;
    for (int i=0; i<listSize; i++){
        Martian* martian=find(i);
        if (martian->ready==1 && martian->priority<=currentPriority && (systemType!=INTERACTIVE || martian->arrivalTime<=lastTime)){
            martianid=martian->id;
            currentPriority=martian->priority;
        }
    }
    return martianid;
}

_Noreturn void *mainThread(void *arg){
    clock_t  tiempoInicial,tiempoFinal, executedTime;
    tiempoInicial = clock();
    int tiempoTotal;

    while(1){
        pthread_mutex_lock(&mutexMain);
        tiempoFinal=clock();
        if (systemType==RTOS) {
            updateMartiansToReady(tiempoInicial,tiempoFinal);
        }
        tiempoTotal= (int)  (tiempoFinal-tiempoInicial)/CLOCKS_PER_SEC;
        lastTime=tiempoTotal;
        if (currentMartian==-1){
            int martianid=getNextCurrentMartian();
            if (martianid!=-1) {
                int time= (int)  (tiempoFinal-tiempoInicial)/CLOCKS_PER_SEC;
                printf("entra marciano #%d a los %d segundos\n", martianid, time);
                currentMartian = martianid;
                executedTime = clock();
            }
        }
        else {
            Martian* martian = findMartianByID(currentMartian);
            if (martian!=NULL){
                int time= (int)  (tiempoFinal-executedTime)/CLOCKS_PER_SEC;
                int martianExecutedTime=martian->executedtime+time;
                if (martianExecutedTime>= martian->executiontime){
                    tiempoTotal = (int) (tiempoFinal-tiempoInicial)/CLOCKS_PER_SEC;
                    printf("marciano #%d ejecutado %d segundos\n", currentMartian, martianExecutedTime);
                    printf("sale marciano #%d a los %d segundos\n", currentMartian, tiempoTotal);
                    martian->executedtime=0;
                    martian->ready=0;
                    currentMartian=-1;
                    int martianid=getNextCurrentMartian();
                    if (martianid!=-1) {
                        int time= (int)  (tiempoFinal-tiempoInicial)/CLOCKS_PER_SEC;
                        printf("entra marciano #%d a los %d segundos\n", martianid, time);
                        currentMartian = martianid;
                        executedTime = clock();
                    }
                }
                if (((systemType==RTOS && mode==RM) || systemType==INTERACTIVE)  && currentMartian!=-1){
                    int martianid=getNextCurrentMartian();
                    if (martianid!=-1 && martianid!=currentMartian) {
                        time= (int)  (tiempoFinal-executedTime)/CLOCKS_PER_SEC;
                        martian->executedtime+=time;
                        printf("marciano #%d ejecutado %d segundos para un total de %d segundos\n", currentMartian, time ,martian->executedtime);
                        tiempoTotal = (int) (tiempoFinal-tiempoInicial)/CLOCKS_PER_SEC;
                        printf("interrupcion de marciano #%d a el marciano activo #%d a los %d segundos\n", martianid, currentMartian ,tiempoTotal);
                        currentMartian = martianid;
                        executedTime = clock();
                    }
                }

            }
        }
        pthread_mutex_unlock(&mutexMain);
    }
}



void *martian_start(void *arg){
    int i=0;
    Martian* martian = (Martian*) arg;

    while(1){
        pthread_mutex_lock(&mutex);
        if (currentMartian==martian->id){
           // printf("camina el marciano con valor de: %d\n", martian->id);
            usleep(1000000);
        }
        pthread_mutex_unlock(&mutex);
    }
    printf("termina el thread #%ld a sumar al contador con valor de: %d\n",(long)arg,counter);

}

int main() {
    printf("Hello, World!\n");
    pthread_mutex_init (&mutex, NULL);
    pthread_mutex_init (&mutexMain, NULL);
    Martian* martian0=createMartian(4,5,2,0);
    Martian* martian1=createMartian(3,8,1,1);
    Martian* martian2=createMartian(3,9,3,5);
    Martian* martian3=createMartian(2,12,3,6);
    pthread_t mThread;
    pthread_create(&mThread, NULL, mainThread, (void *) 1);

    pthread_exit(NULL);
    pthread_mutex_destroy(&mutex);
    pthread_mutex_destroy(&mutexMain);
    return 0;
}
Martian* createMartian(int executionTime, int period, int priority, int arrivalTime){
    Martian* martian=(Martian*) malloc(sizeof(Martian));
    martian->id=countMartians;
    countMartians++;
    martian->executiontime=executionTime;
    martian->period=period;
    martian->ready=1;
    martian->executedtime=0;
    martian->priority=priority;
    martian->arrivalTime=arrivalTime;
    pthread_mutex_lock(&mutexMain);
    insert(martian);
    setPriority(martian);
    pthread_mutex_unlock(&mutexMain);
    pthread_t martianThread;
    pthread_create(&martianThread,NULL,martian_start,(void*)martian);
    return martian;
}

void setPriority(Martian* martian){
    if (systemType==INTERACTIVE) {
        if (scheduler == FCFS) {
            martian->priority = countMartians;
        } else if (scheduler == SRTN) {
            SRTNPriority();
        }
    } else if (systemType==RTOS){
        RTOSPriority();
    }
}
