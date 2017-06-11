#include <iostream>
#include <cstdlib>
#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <unistd.h>
#include <sys/wait.h>
#include <cstring>
#include <fstream>
#include <ctime>
#include <ctype.h>
#include "Semaphore.h"
#define BUFSIZE 10

#define MUTEX 0
#define FULL 1
#define EMPTY 2
#define SHMSIZE 50   // maximum number of bytes in a message

using namespace std;

/*
 * 
 */
struct shm{
    char message[SHMSIZE];
    int id, pid_match = 0;
};

int shm_id; //ID of shared memory
void cleanup (int shm_id, struct shm *temp); //Cleanup procedure
void consumer(int semid, struct shm *temp, char *message, int &id, string temp1);
void producer(int semid, struct shm *temp, char *message, int id);
int pid_match;


int main(int argc, char** argv) 
{
    int N = 2, status, NumOfMessages = 2, id ;
    string line, temp;
    char message[SHMSIZE];
    if (argv[1] != NULL)    N = atoi(argv[1]);
    
    cout << "How many messages a P process will send to C?" << endl;
    cin >> NumOfMessages;
    cout << "Please enter file in order to proceed" << endl;
    cin >> temp;

    pid_t pid;
    
    if ((shm_id = shmget ((key_t) getpid(), 2*sizeof(shm), 0666 | IPC_CREAT)) == -1 ) //Shared memory
    {
        perror("shmget");
        exit (2);
    } 

    struct shm * in_ds = (struct shm *) malloc (sizeof (struct shm));   //For in_ds messages
    in_ds = (struct shm *) shmat (shm_id, NULL, 0);
    struct shm * out_ds = (struct shm *) malloc (sizeof (struct shm));  //For out_ds messages
    out_ds = (struct shm *) shmat (shm_id, NULL, 0);
    
    ifstream file(temp);
    unsigned int time_ui = static_cast<unsigned int>( time(NULL) % 1000 );
     
    int semid = sem_create(3);
    sem_initialise(MUTEX, 1, semid);
    sem_initialise(FULL, 0, semid);
    sem_initialise(EMPTY, 10, semid);
    
    for (int i = 0; i < N; i++)     //Create N-1 children
    {
        srand(time(NULL)- i*2); 
        switch(pid = fork())
        {
            case 0:
                //cout << "Child " << getpid() << endl;               
                if (file.is_open())
                {
                    int counter = 0;
                    file.clear() ;                                    //Start reading file again
                    file.seekg(0, ios::beg) ;
                    while ( getline (file, line) ) counter++;      //Count num of lines into given file
                
                    for (int i = 0; i < NumOfMessages; i++)
                    {
                        int rand_num = (rand() / (RAND_MAX + 1.0))*counter; //Get a random line from file

                        file.clear() ;                                    //Start reading file again
                        file.seekg(0, ios::beg) ;

                        for (int j = 0; j < rand_num; j++) getline (file, line); //Read random line   

                        for (int j = 0; j < SHMSIZE; j++)
                        {
                            message [j] = line[j];
                            message [SHMSIZE] = '\0';
                        }

                        producer ( semid, in_ds, message, getpid());  //Write message to in_ds                

                        wait (&status); 

                        consumer ( semid, out_ds, message, id, ""); //Read out_ds message from parent
                        cout << "Child - message from Parent: " << out_ds->message << endl;
                        cout << "Child pid= "<< getpid() << "   and id coming from Parent=  "<< out_ds->id << endl;

                        if(out_ds->id == getpid()) out_ds->pid_match++;
                        pid_match = out_ds->pid_match;
                    }
                }
                else cout << "Unable to open file";
                file.close();
                producer ( semid, in_ds, message, pid_match);   //Send pid_match to Parent 
                exit(1);
            case -1:
            // take care of possible fork failure 
                perror("fork failure");
                break;
            default:             
                //wait(NULL);
               // cout << "Parent " << getpid() << endl;

                for (int i = 0; i < NumOfMessages; i++)
                {
                    consumer ( semid, in_ds, message, id, ""); //Get in_ds message from child
 
                    producer ( semid, out_ds, message, id); //Send out_ds message to child
                }
                consumer ( semid, out_ds, message, id, "final step"); //Get pid_match from child

                cout << "Number of child processes  " << N << endl;
                cout << "pid_match =    " << out_ds->pid_match << endl;

                waitpid(-1, NULL, 0);
        } 
    }
    
    cleanup (shm_id, in_ds);
    cleanup (shm_id, out_ds);
    semctl(semid, 0, IPC_RMID,0);
    
    return 0;
}


void cleanup (int shm_id, struct shm *temp) //Remove shared memory segment
{
    shmdt (temp);
    shmctl (shm_id, IPC_RMID, 0);
} 

void producer (int semid, struct shm *temp, char *message, int id)
{

    wait(EMPTY, semid);
    wait(MUTEX, semid);

    temp->id = id;
    strcpy(temp->message, message);

    signal(MUTEX, semid);
    signal(FULL, semid);
}

void consumer(int semid, struct shm *temp, char *message, int &id, string temp1)
{
    wait(FULL, semid);
    wait(MUTEX, semid);
            
    if (memcpy (temp, temp, sizeof(struct shm)) == NULL)
    {
        puts ("Error in memcpy");
        cleanup (shm_id, temp);
        exit (5);
    } // end if error in shared memory get
    else
    {
        if (temp1 != "final step")   //If you have a message to convert
        { 
            id = temp->id;
            strcpy(message, temp->message);
            for (int j = 0; j < SHMSIZE; j++)   //Change message from lowercase to uppercase
            {
                message [j] = toupper(message [j]);
                message [SHMSIZE] = '\0';
            }

            strcpy (temp->message, message);
        }
    }                    
    signal(MUTEX, semid);
    signal(EMPTY, semid);
}
