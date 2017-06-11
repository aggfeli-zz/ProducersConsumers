#ifndef SEMAPHORE_H
#define	SEMAPHORE_H
#include <string>
#include <iostream>
#include <stdio.h>
#include <cstdlib>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>

using namespace std;

union semun {
    int val; 
    struct semid_ds *buf;		
    unsigned short *array;	
    struct seminfo *__buf;	
};

int sem_create(int NumberOfSems);
void sem_initialise(int semno, int val, int semid);
void wait(int semno, int semid);
void signal(int semno, int semid);


#endif	/* SEMAPHORE_H */

