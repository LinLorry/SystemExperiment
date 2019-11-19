#include <sys/mman.h>
#include <linux/sem.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#define MAXSEM 5
#define MAX_PRODUCER_NUM 5
#define MAX_CONSUMER_NUM 5
#define MAX_PRODUCTION_NUM 100

int producer_num = 0;
int procution_num = 0;
int consumer_num = 0;

int *array;
int *sum;
int *set;
int *get;

int fullid;
int emptyid;
int mutxid;

struct sembuf P,V;
union semun arg;

void production();

void consumption();

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

    fullid = semget(IPC_PRIVATE,1,IPC_CREAT|00666);  
    emptyid = semget(IPC_PRIVATE,1,IPC_CREAT|00666);
    mutxid = semget(IPC_PRIVATE,1,IPC_CREAT|00666); 

    arg.val = 0;
    if(semctl(fullid , 0 , SETVAL , arg) == -1) perror("semctl setval error");
    arg.val = MAXSEM;    //初始化P,V操作
    if(semctl(emptyid , 0 ,SETVAL , arg) == -1) perror("semctl setval error");
    arg.val = 1;
    if(semctl(mutxid , 0 ,SETVAL , arg) == -1) perror("setctl setval error"); 

    V.sem_num=0;
    V.sem_op =1;
    V.sem_flg=SEM_UNDO;
    P.sem_num=0;
    P.sem_op =-1;
    P.sem_flg=SEM_UNDO;

    for(int i = 0; i < MAX_PRODUCER_NUM; )
    {
        int tmp = fork();
        if (tmp == 0)
        {
            production();
            exit(0);
        }
        else ++i;
        
    }

    for(int i = 0; i < MAX_CONSUMER_NUM; )
    {
        int tmp = fork();
        if (tmp == 0)
        {
            consumption();
            exit(0);
        }
        else ++i;
    }

    waitpid(-1,&status,0);

    return 0;
}

void production()
{
    while (*set < MAX_PRODUCTION_NUM)
    {
        semop(emptyid , &P , 1);
        semop(mutxid , &P , 1);

        array[(*set)%MAXSEM] = *set;

        printf("Producer %d\n", *set);
        
        (*set)++;

        semop(mutxid , &V , 1);
        semop(fullid , &V , 1);
    } 
    printf("Produce is over.\n"); 

}

void consumption()
{
    int id = ++consumer_num;
    while (1)
    {
        semop(fullid , &P , 1);
        semop(mutxid , &P , 1);
        if(*get == MAX_PRODUCTION_NUM) 
            break;
        
        *sum += array[(*get)%MAXSEM];

        printf("The ComsumerA Get Number %d\n", array[(*get)%MAXSEM] );
        (*get)++;
        if(*get == MAX_PRODUCTION_NUM)
            printf("The sum is %d \n ", *sum);
        semop(mutxid , &V , 1);
        semop(emptyid , &V ,1 );
        sleep(1);
    }
    printf("%d comsumer is over.\n", id); 
}