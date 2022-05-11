#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pthread.h"
#include <time.h>
#include <unistd.h>
#include <limits.h>
#include <semaphore.h>
#include "Martian.h"
#include "Martian_node.h"
#include<fcntl.h>

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


#define EDF 1
#define RM 2

#define FCFS 1
#define PRIORITY 2
#define SRTN 3

#define RTOS 1
#define INTERACTIVE 2

#define UP 0
#define DOWN 1
#define LEFT 2
#define RIGHT 3

#define STARTSEMAPHORE "startsemaphore"

sem_t* startSemaphore;
pthread_mutex_t mutex;
pthread_mutex_t mutexMain;

int currentMartian=-1;
int mode=EDF;
int scheduler=FCFS;
int systemType = INTERACTIVE;
int lastTime=0;
int countMartians=0;
int finish=0;
void initializeFile(char* path);
void updateMartiansToReady(clock_t initialTime, clock_t finalTIme);
int getNextCurrentMartian();
Martian* addMartian(int x, int y, int type, int totalEnergy, int period, int arrivalTime);
void setPriority(Martian* martian);
void moveMartian(int lastdirection,int direction, Martian *martian);
void writeinFile(char* info,char*path);
void freedata();

#define ARCHIVOMARTIANS "marcianos.txt"
#define ARCHIVOTIME "timeline.txt"
#define STR_LEN 300

void initializeFile(char* path){
    FILE *file;
    file = fopen(path, "w");
    if (file == NULL) {
        return;
    }
    fprintf(file, "");
    fclose(file);
}

void writeinFile(char* info,char*path){
    FILE *file;
    file = fopen(path, "a+");
    if (file == NULL) {
        return;
    }
    fprintf(file, info);
    fclose(file);
    printf("%s",info);
}


void updateMartiansToReady(clock_t initialTime, clock_t finalTIme) {
    int tiempoTotal;
    for (int i = 0; i < get_size(); i++) {
        Martian *martian = find(i);
        tiempoTotal = (int) (finalTIme - initialTime) / CLOCKS_PER_SEC;
        if ((tiempoTotal-martian->timeCreated) % martian->period == 0 && martian->finish==0) {
            if (martian->ready==1 && lastTime!=tiempoTotal){
                printf("Error no se pudo calendarizar a marciano #%d correctamente, tiempo: %d segundos\n",martian->id, tiempoTotal);
                finish=1;
            }
            martian->energy=martian->executiontime;
            martian->ready = 1;
        }
    }
}

int getNextCurrentMartian(){
    int martianid=-1;
    int currentPriority=INT_MAX;
    for (int i=0; i<get_size(); i++){
        Martian* martian=find(i);
        if (martian!=NULL) {
            if (martian->ready == 1 && martian->finish == 0 && martian->priority <= currentPriority &&
                (systemType != INTERACTIVE || martian->arrivalTime <= lastTime)) {
                martianid = martian->id;
                currentPriority = martian->priority;
            }
        }
    }
    return martianid;
}

void *mainThread(void *arg){
    sem_wait(startSemaphore);
    clock_t  tiempoInicial,tiempoFinal;
    int executedTime=0;
    tiempoInicial = clock();
    int tiempoTotal;
    while(finish==0){
        pthread_mutex_lock(&mutexMain);
        tiempoFinal=clock();

        tiempoTotal= (int)  (tiempoFinal-tiempoInicial)/CLOCKS_PER_SEC;

        if (currentMartian==-1){
            if (systemType==RTOS) {
                updateMartiansToReady(tiempoInicial,tiempoFinal);
                lastTime=tiempoTotal;
            }
            int martianid=getNextCurrentMartian();
            if (martianid!=-1) {
                int time= (int)  (tiempoFinal-tiempoInicial)/CLOCKS_PER_SEC;
                char data[STR_LEN];
                sprintf(data, "%d,%d,", martianid,time);
                writeinFile(data,ARCHIVOTIME);
                printf("entra marciano #%d a los %d segundos\n", martianid, time);
                currentMartian = martianid;
                executedTime = tiempoTotal;
            }
        }
        else {
            Martian* martian = findMartianByID(currentMartian);
            if (martian!=NULL){
                int time= tiempoTotal-executedTime;
                int martianExecutedTime=martian->executedtime+time;
                martian->energy=martian->executiontime-martianExecutedTime;
                if (martianExecutedTime>= martian->executiontime || martian->finish==1){
                    tiempoTotal = (int) (tiempoFinal-tiempoInicial)/CLOCKS_PER_SEC;
                    char data[STR_LEN];
                    sprintf(data, "%d\n",tiempoTotal);
                    writeinFile(data,ARCHIVOTIME);
                    printf("marciano #%d ejecutado %d segundos\n", currentMartian, martianExecutedTime);
                    printf("sale marciano #%d a los %d segundos\n", currentMartian, tiempoTotal);
                    currentMartian=-1;
                    martian->executedtime=0;
                    martian->energy=0;
                    martian->ready=0;

                    if (systemType==RTOS) {
                        updateMartiansToReady(tiempoInicial,tiempoFinal);
                        lastTime=tiempoTotal;
                    }
                    int martianid=getNextCurrentMartian();
                    if (martianid!=-1) {
                        int time= (int)  (tiempoFinal-tiempoInicial)/CLOCKS_PER_SEC;
                        sprintf(data, "%d,%d,", martianid,time);
                        writeinFile(data,ARCHIVOTIME);
                        printf("entra marciano #%d a los %d segundos\n", martianid, time);
                        currentMartian = martianid;
                        executedTime = tiempoTotal;
                    }
                }
                if (((systemType==RTOS && mode==RM) || systemType==INTERACTIVE)  && currentMartian!=-1){
                    if (systemType==RTOS) {
                        updateMartiansToReady(tiempoInicial,tiempoFinal);
                        lastTime=tiempoTotal;
                    }
                    int martianid=getNextCurrentMartian();
                    if (martianid!=-1 && martianid!=currentMartian) {
                        tiempoTotal = (int) (tiempoFinal-tiempoInicial)/CLOCKS_PER_SEC;
                        time= tiempoTotal-executedTime;
                        martian->executedtime+=time;
                        martian->energy=martian->executiontime-martian->executedtime;
                        char data[STR_LEN];
                        sprintf(data, "%d\n",tiempoTotal);
                        writeinFile(data,ARCHIVOTIME);
                        printf("marciano #%d ejecutado %d segundos para un total de %d segundos\n", currentMartian, time ,martian->executedtime);
                        char data2[STR_LEN];
                        sprintf(data2, "%d,%d,", currentMartian,tiempoTotal);
                        writeinFile(data2,ARCHIVOTIME);
                        printf("interrupcion de marciano #%d a el marciano activo #%d a los %d segundos\n", martianid, currentMartian ,tiempoTotal);
                        currentMartian = martianid;
                        executedTime = tiempoTotal;
                    }
                }
            }
        }
        if (systemType==RTOS) {
            updateMartiansToReady(tiempoInicial,tiempoFinal);
        }
        lastTime=tiempoTotal;
        pthread_mutex_unlock(&mutexMain);
    }
    char data[STR_LEN];
    sprintf(data, "%d,%d,%d\n", systemType,scheduler,mode);
    writeinFile(data,ARCHIVOMARTIANS);
    if (currentMartian!=-1){
        char data2[STR_LEN];
        sprintf(data2, "%d\n", lastTime);
        writeinFile(data2,ARCHIVOTIME);
    }
    freedata();
    system("python3 ../report/report.py");
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
    for (int i=0;i<=3;i++){
        if (existWay(i,row,col) && i!= getOppositeDirection(direction)){
            value--;
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
int wayIsBlocked(int nextMove, Martian* martian){

    for (int i=0; i<get_size();i++){
        Martian * martiani= find(i);
        int row=martian->row;
        int col=martian->col;
        if (martian->id!=martiani->id){
            if (nextMove==UP){
                row--;
            } else if(nextMove==DOWN)
                row++;
            else if(nextMove==LEFT){
                col--;
            } else{
                col++;
            }

            if (martiani->row==row && martiani->col==col){
                if (((martiani->currentDirection==UP || martiani->currentDirection==RIGHT) && (nextMove==UP || nextMove==RIGHT)) ||
                        ((martiani->currentDirection==DOWN || martiani->currentDirection==LEFT) && (nextMove==DOWN || nextMove==LEFT)) ) {
                    return 1;
                }
            }
        }
    }
    return 0;
}

void *martian_start(void *arg){
    Martian* martian = (Martian*) arg;
    while(finish==0){
        if (systemType==INTERACTIVE && martian->ready==0){
            martian->finish=1;
            break;
        }
        pthread_mutex_lock(&mutex);
        if (currentMartian==martian->id){
            int nextMove=selectNextMove(martian);
            int lastDirection=martian->currentDirection;
            if(wayIsBlocked(nextMove,martian)==0) {
                martian->currentDirection = nextMove;
                moveMartian(lastDirection, nextMove, martian);
            }
            usleep(500000);
        }
        pthread_mutex_unlock(&mutex);
        if (martian->col==0 && martian->row==5){
            martian->finish=1;
            break;
        }
    }

}



/**
 * Moves martian in a specific direction
 * @param direction 0 UP, 1 DOWN, 2 LEFT, 3 RIGHT
 * @param martian Martian object to move
 */

void moveMartian(int lastdirection,int direction, Martian *martian) {
    int squareLength = 40;
    if (lastdirection!=direction) {
        if (direction==RIGHT){
            martian->changex=0-martian->currentChangex;
            martian->changey=0-martian->currentChangey;
        }
        else if (direction==LEFT){
            martian->changex=squareLength/2-martian->currentChangex;
            martian->changey=squareLength/2-martian->currentChangey;
        }
        else if (direction==DOWN){
            martian->changex=squareLength/2-martian->currentChangex;
            martian->changey=squareLength/2-martian->currentChangey;
        }
        else{
            martian->changex=0-martian->currentChangex;
            martian->changey=0-martian->currentChangey;
        }
    }
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

void initialize(){
    sem_unlink(STARTSEMAPHORE);
    srand(time(NULL));
    startSemaphore = sem_open(STARTSEMAPHORE, O_CREAT, 0644, 0);
    pthread_mutex_init (&mutex, NULL);
    pthread_mutex_init (&mutexMain, NULL);
    pthread_t mThread;
    pthread_create(&mThread, NULL, mainThread, (void *) 1);
    pthread_detach(mThread);
}
void freedata(){
    sem_close(startSemaphore);
    sem_unlink(STARTSEMAPHORE);
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
    martian->finish=0;
    martian->currentDirection=3;
    martian->executedtime=0;
    martian->priority=type;
    martian->type = type;
    martian->energy = totalEnergy;
    martian->executiontime = totalEnergy;
    martian->period = period;
    martian->timeCreated=lastTime;
    martian->arrivalTime = arrivalTime;
    martian->destX = -1;
    martian->destY = -1;
    martian->movDir = -1;
    martian->changex=0;
    martian->changey=0;
    martian->currentChangex=20;
    martian->currentChangey=20;
    martian->pBar=NULL;
    char data[STR_LEN];
    sprintf(data, "%d,%d,%d,%d,%d\n", martian->id,martian->energy,martian->arrivalTime,martian->period,martian->timeCreated);
    writeinFile(data,ARCHIVOMARTIANS);
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
            FCFSPriority();
        } else if (scheduler == SRTN) {
            SRTNPriority();
        }
    } else if (systemType==RTOS){
        RTOSPriority();
    }
}
