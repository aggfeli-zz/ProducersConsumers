#include "Semaphore.h"



int sem_create(int NumberOfSems) 
{ 
    int  id;
    key_t key = 12345;
    int semflg = IPC_CREAT | 0666;
    id = semget(key, NumberOfSems, semflg);
    if(id < 0)
    {
        perror("semget:");
	exit (1);
    }
    return id;
}

void sem_initialise(int semno, int val, int semid) 
{
    union semun un;
    un.val = val;
    if(semctl(semid, semno, SETVAL, un) < 0)
    {
	//	cout << semno << endl;
        perror("semctl:");
	exit(2);
    }
}

void wait(int semno, int semid) 
{
    struct sembuf buf;
    buf.sem_num = semno;
    buf.sem_op = -1;
    buf.sem_flg = 0;
    if(semop(semid, &buf, 1) < 0) 
    {
        perror("semop:");
        exit(2);
    }
}

void signal(int semno, int semid) 
{
    struct sembuf buf;
    buf.sem_num = semno;
    buf.sem_op = 1;
    buf.sem_flg = 0;
    if(semop(semid, &buf, 1) < 0)
    {
        perror("semop:");
        exit(2);
    }
}
