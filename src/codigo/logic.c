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

#define UP 0
#define DOWN 1
#define LEFT 2
#define RIGHT 3

int laberinto[21][22]={
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,0,1,1,1,0},
        {0,0,0,0,0,0,1,0,1,0,0,0,0,0,0,0,1,0,1,0,1,0},
        {0,0,1,1,1,1,1,0,1,1,1,0,1,0,1,1,1,0,1,0,1,0},
        {0,0,1,0,0,0,0,0,0,0,1,0,1,0,1,0,0,0,1,0,1,0},
        {0,1,1,1,1,0,1,1,1,1,1,0,1,1,1,0,1,1,1,0,1,0},
        {0,0,0,0,1,0,1,0,1,0,0,0,1,0,0,0,1,0,1,0,0,0},
        {0,0,1,1,1,0,1,0,1,0,1,1,1,1,1,0,1,0,1,1,1,0},
        {0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0},
        {0,0,1,1,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,0},
        {0,0,0,0,0,0,1,0,0,0,1,0,1,0,0,0,1,0,1,0,1,0},
        {0,1,1,0,1,1,1,0,1,1,1,0,1,1,1,1,1,0,1,0,1,0},
        {0,0,1,0,1,0,0,0,1,0,0,0,0,0,0,0,0,0,1,0,1,0},
        {0,0,1,0,1,0,1,1,1,0,1,1,1,1,1,1,1,0,1,0,1,0},
        {0,0,1,0,1,0,0,0,1,0,1,0,0,0,1,0,0,0,1,0,1,0},
        {0,0,1,0,1,1,1,1,1,0,1,0,1,0,1,1,1,1,1,0,1,0},
        {0,0,1,0,0,0,0,0,0,0,1,0,1,0,0,0,0,0,0,0,1,0},
        {0,0,1,1,1,1,1,1,1,0,1,0,1,1,1,1,1,1,1,1,1,0},
        {0,0,1,0,0,0,0,0,1,0,1,0,0,0,0,0,0,0,1,0,1,0},
        {0,0,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,0,1,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
};
pthread_mutex_t mutex;
pthread_mutex_t mutexMain;
int counter = 0;
int currentMartian=-1;
int mode=RM;
int scheduler=FCFS;
int systemType = RTOS;
int lastTime=0;
int countMartians=0;
void updateMartiansToReady(clock_t initialTime, clock_t finalTIme);
int getNextCurrentMartian();
Martian* addMartian(int x, int y, int type, int totalEnergy, int period, int arrivalTime);
void setPriority(Martian* martian);
void moveMartian(int direction, Martian *martian);

void updateMartiansToReady(clock_t initialTime, clock_t finalTIme) {
    int tiempoTotal;
    for (int i = 0; i < get_size(); i++) {
        Martian *martian = find(i);
        tiempoTotal = (int) (finalTIme - initialTime) / CLOCKS_PER_SEC;
        if (tiempoTotal % martian->period == 0) {
            if (martian->ready==1 && lastTime!=tiempoTotal){
                printf("Error no se pudo calendarizar a marciano #%d correctamente, tiempo: %d segundos\n",martian->id, tiempoTotal);
            }
            martian->energy=martian->executiontime;
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
                martian->energy=martian->executiontime-martianExecutedTime;
                if (martianExecutedTime>= martian->executiontime){
                    tiempoTotal = (int) (tiempoFinal-tiempoInicial)/CLOCKS_PER_SEC;
                    printf("marciano #%d ejecutado %d segundos\n", currentMartian, martianExecutedTime);
                    printf("sale marciano #%d a los %d segundos\n", currentMartian, tiempoTotal);
                    martian->executedtime=0;
                    martian->energy=0;
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
                        martian->energy=martian->executiontime-martian->executedtime;
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

int countWays(int row, int col){
    return laberinto[row+1][col]+laberinto[row-1][col]+laberinto[row][col+1]+
            laberinto[row][col-1];
}
int existWay(int direction,int row,int col){
    switch (direction) {
        case UP:
            return laberinto[row-1][col];
        case DOWN:
            return laberinto[row+1][col];
        case LEFT:
            return laberinto[row][col-1];
        case RIGHT:
            return laberinto[row][col+1];
        default:
            return 0;
    }
}

int getOppositeDirection(int direction){
    switch (direction) {
        case UP:
            return DOWN;
        case DOWN:
            return UP;
        case LEFT:
            return RIGHT;
        case RIGHT:
            return LEFT;
        default:
            return 0;
    }
}

int drift(int direction,int row,int col){
    if (direction==UP || direction==DOWN){
        if (laberinto[row][col+1]==1){
            return RIGHT;
        } else {
            return LEFT;
        }
    } else{
        if (laberinto[row+1][col]==1){
            return DOWN;
        } else {
            return UP;
        }
    }
}
int selectRandomdirection(int randomvalue,int direction,int row,int col){
    int value=randomvalue;
    printf("valor de random es %d\n",randomvalue);
    for (int i=0;i<=3;i++){
        if (existWay(i,row,col) && i!= getOppositeDirection(direction)){
            value--;
            printf("reduce random %d por direccion %d\n",value,i);
            if (value==0){
                return i;
            }
        }
    }
    return 0;
}

int selectNextMove(Martian* martian){
    int col=martian->col+1;
    int row=martian->row;
    int ways= countWays(row,col);
    if (ways<=2 && existWay(martian->currentDirection,row,col)==1) {
        return martian->currentDirection;
    }else if (ways==2){
        return drift(martian->currentDirection,row,col);
    }
    else if (ways==1){
        return getOppositeDirection(martian->currentDirection);
    } else{
        int r=rand()%(ways-1)+1;
        return selectRandomdirection(r,martian->currentDirection,row,col);
    }
}

void *martian_start(void *arg){
    Martian* martian = (Martian*) arg;
    while(1){
        pthread_mutex_lock(&mutex);
        if (currentMartian==martian->id){
            int nextMove=selectNextMove(martian);
            printf("el siguiente movimiento es %d\n",nextMove);
            martian->currentDirection=nextMove;
            moveMartian(nextMove,martian);
            usleep(100000);
        }
        pthread_mutex_unlock(&mutex);
    }
    printf("termina el thread #%ld a sumar al contador con valor de: %d\n",(long)arg,counter);

}

/**
 * Moves martian in a specific direction
 * @param direction 0 UP, 1 DOWN, 2 LEFT, 3 RIGHT
 * @param martian Martian object to move
 */
void moveMartian(int direction, Martian *martian) {
    int squareLength = 40;
    switch (direction) {
        case UP:
            // Up
            if (martian->y > 0) {
                martian->destY = martian->y - squareLength;
                martian->movDir = direction;
                martian->row -=1;
            }
            break;
        case DOWN:
            // Down
            if (martian->y < 800) {
                martian->destY = martian->y + squareLength;
                martian->movDir = direction;
                martian->row +=1;
            }
            break;
        case LEFT:
            // Left
            if (martian->x > 0) {
                martian->destX = martian->x - squareLength;
                martian->movDir = direction;
                martian->col -=1;
            }
            break;
        case RIGHT:
            // Right
            if (martian->x < 800) {
                martian->destX = martian->x + squareLength;
                martian->movDir = direction;
                martian->col +=1;
            }
            break;
        default:
            printf("Invalid direction for moving martian: %d\n", direction);
    }
}

int setMartians() {
    printf("Hello, World!\n");

    Martian* martian0=addMartian(0,420,1,2,6,0);
    Martian* martian1=addMartian(0,420,0,3,8,0);
    Martian* martian2=addMartian(0,420,1,2,6,0);
    Martian* martian3=addMartian(0,420,2,2,12,0);

    return 0;
}

void initialize(){
    srand(time(NULL));
    pthread_mutex_init (&mutex, NULL);
    pthread_mutex_init (&mutexMain, NULL);
    pthread_t mThread;
    pthread_create(&mThread, NULL, mainThread, (void *) 1);
}
void freedata(){
    pthread_exit(NULL);
    pthread_mutex_destroy(&mutex);
    pthread_mutex_destroy(&mutexMain);
}

Martian* addMartian(int x, int y, int type, int totalEnergy, int period, int arrivalTime) {
    // Create martian
    Martian *martian = (Martian*)malloc(sizeof(Martian));
    martian->x = x;
    martian->y = y;
    martian->row = 11;
    martian->col = 0;
    martian->id=countMartians;
    countMartians++;
    martian->ready=1;
    martian->currentDirection=3;
    martian->executedtime=0;
    martian->priority=type;
    martian->type = type;
    martian->energy = totalEnergy;
    martian->executiontime = totalEnergy;
    martian->period = period;
    martian->arrivalTime = arrivalTime;
    martian->destX = -1;
    martian->destY = -1;
    martian->movDir = -1;
    martian->pBar=NULL;
    pthread_mutex_lock(&mutexMain);
    insert(martian);
    setPriority(martian);
    pthread_mutex_unlock(&mutexMain);
    pthread_t martianThread;
    pthread_create(&martianThread,NULL,martian_start,(void*)martian);
    pthread_detach(martianThread);
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
