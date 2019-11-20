#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#define MAXSEM 5
#define MAX_PRODUCER_NUM 5
#define MAX_CONSUMER_NUM 5
#define MAX_PRODUCTION_NUM 100

int *array;
int *sum;
int *set;
int *get;

int full_id;
int empty_id;
int mutx_id;

union semun {
    int val; /* value for SETVAL */
    struct semid_ds *buf; /* buffer for IPC_STAT, IPC_SET */
    unsigned short *array; /* array for GETALL, SETALL */
    struct seminfo *__buf; /* buffer for IPC_INFO */
} arg;

struct sembuf P = {0 , -1, SEM_UNDO};
struct sembuf V = {0 , 1, SEM_UNDO};

struct sembuf up_all_producer = {0, MAX_PRODUCER_NUM, SEM_UNDO};
struct sembuf up_aLl_consumer = {0, MAX_CONSUMER_NUM, SEM_UNDO};

void production(size_t id);

void consumption(size_t id);

int main()
{
    int status;

    array = (int *)mmap(NULL, sizeof(int) * MAXSEM, 
        PROT_READ|PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    sum = (int *)mmap(NULL, sizeof(int), 
        PROT_READ|PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    get = (int *)mmap(NULL , sizeof( int),
        PROT_READ|PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS,-1,0);
    set = (int *)mmap(NULL , sizeof( int),
        PROT_READ|PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS,-1,0);

    *sum = 0, *get = 0, *set = 0;

    full_id = semget(IPC_PRIVATE,1,IPC_CREAT|00666);
    empty_id = semget(IPC_PRIVATE,1,IPC_CREAT|00666);
    mutx_id = semget(IPC_PRIVATE,1,IPC_CREAT|00666);

    arg.val = 0;
    if (semctl(full_id, 0, SETVAL, arg) == -1) perror("semctl setval error");
    arg.val = 1;
    if (semctl(mutx_id, 0, SETVAL, arg) == -1) perror("setctl setval error");
    arg.val = MAXSEM;
    if (semctl(empty_id, 0, SETVAL, arg) == -1) perror("semctl setval error");
    
    for(int i = 0; i < MAX_PRODUCER_NUM; )
    {
        int tmp = fork();
        if (tmp == 0)
        {
            production(i);
            if (semctl(full_id, 0, SETVAL, arg) == -1) perror("semctl setval error");
            return 0;
        }
        else ++i;
    }

    for(int i = 0; i < MAX_CONSUMER_NUM; )
    {
        int tmp = fork();
        if (tmp == 0)
        {
            consumption(i); 
            if (semctl(empty_id, 0, SETVAL, arg) == -1) perror("semctl setval error");
            return 0;
        }
        else ++i;
    }

    for (size_t i = 0; i < 10; i++)
        waitpid(-1,&status,0);
    return 0;
}

void production(size_t id)
{
    int production_sum = 0;
    short break_flag = 0;
    while (1)
    {
        semop(empty_id, &P, 1);
        semop(mutx_id, &P, 1);
        
        if (*set != MAX_PRODUCTION_NUM)
        {
            array[(*set)%MAXSEM] = *set;
            production_sum += *set;
            printf("Producer %d produce %d\n", id, *set);
            (*set)++;
            if (*set == MAX_PRODUCTION_NUM && 
                semctl(empty_id, 0, SETVAL, arg) == -1) 
                perror("semctl setval error");
        }

        if (*set == MAX_PRODUCTION_NUM)
            break_flag = 1;

        semop(mutx_id , &V, 1);
        semop(full_id , &V, 1);

        if (break_flag)
            break;
        sleep(1);  
    }

    printf("Producer %d is over. Produce sum : %d\n", id, production_sum);
}

void consumption(size_t id)
{
    short break_flag = 0;
    while (1)
    {
        semop(full_id, &P, 1);
        semop(mutx_id, &P, 1);

        if (*get != MAX_PRODUCTION_NUM)
        {
            printf("Consumer %d get the %d\n", id, array[(*get)%MAXSEM]);
            *sum += array[(*get)%MAXSEM];
            (*get)++;
            if (*get == MAX_PRODUCTION_NUM && 
                semctl(full_id, 0, SETVAL, arg) == -1) 
                perror("semctl setval error");    
        }
        
        if (*get == MAX_PRODUCTION_NUM)
            break_flag = 1;

        semop(mutx_id, &V, 1);
        semop(empty_id, &V, 1);

        if (break_flag)
            break;
    }
    printf("Consumer %d is over\n", id);
}