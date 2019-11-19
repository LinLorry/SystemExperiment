#include <sys/mman.h>
#include <sys/types.h>
#include <linux/sem.h>
//系统读写安全相关函数
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
//该头文件内包含了通过错误码来回报错误资讯的宏
#include <errno.h>
#include <time.h>
#define MAXSEM 5
//声明三个信号灯ID
int fullid;
int emptyid;
int mutxid;

int main()
{
    /*在sembuf结构中，sem_num是相对应的信号量集中的某一个资源，所以其值是一个从0到相应的信号量集的资源总数（ipc_perm.sem_nsems）之间的整数。sem_op指明所要执行的操作，sem_flg说明函数semop的行为。sem_op的值是一个整数.释放相应的资源数，将sem_op的值加到信号量的值上.
    */
    struct sembuf P,V;
    union semun arg;
    //声明共享主存
    int *array;
    int *sum;
    int *set;
    int *get;
    //映射共享主存
    /*mmap是一种内存映射文件的方法，即将一个文件或者其它对象映射到进程的地址空间
    实现文件磁盘地址和进程虚拟地址空间中一段虚拟地址的一一对映关系。
    实现这样的映射关系后，进程就可以采用指针的方式读写操作这一段内存
    而系统会自动回写脏页面到对应的文件磁盘上
    即完成了对文件的操作而不必再调用read,write等系统调用函数。
    相反，内核空间对这段区域的修改也直接反映用户空间
    从而可以实现不同进程间的文件共享。
    */
    array = (int *)mmap(NULL , sizeof( int )* MAXSEM,
        PROT_READ|PROT_WRITE,MAP_SHARED|MAP_ANONYMOUS,-1,0);
    sum = (int *)mmap(NULL , sizeof( int),
        PROT_READ|PROT_WRITE,MAP_SHARED|MAP_ANONYMOUS,-1,0);
    get = (int *)mmap(NULL , sizeof( int),
        PROT_READ|PROT_WRITE,MAP_SHARED|MAP_ANONYMOUS,-1,0);
    set = (int *)mmap(NULL , sizeof( int),
        PROT_READ|PROT_WRITE,MAP_SHARED|MAP_ANONYMOUS,-1,0);
    *sum = 0;
    *get = 0;
    *set = 0; 

    //创建信号量、生成信号灯
    fullid= semget(IPC_PRIVATE,1,IPC_CREAT|00666);  
    emptyid=semget(IPC_PRIVATE,1,IPC_CREAT|00666);
    mutxid=semget(IPC_PRIVATE,1,IPC_CREAT|00666); 

    //为信号灯赋值
    arg.val = 0;
    if(semctl(fullid , 0 , SETVAL , arg) == -1) perror("semctl setval error");
    arg.val = MAXSEM;
    if(semctl(emptyid , 0 ,SETVAL , arg) == -1) perror("semctl setval error");
    arg.val = 1;
    if(semctl(mutxid , 0 ,SETVAL , arg) == -1) perror("setctl setval error"); 
    //初始化P,V操作
    V.sem_num=0;
    V.sem_op =1;
    V.sem_flg=SEM_UNDO;
    P.sem_num=0;
    P.sem_op =-1;
    P.sem_flg=SEM_UNDO;

  //生产者进程
    if(fork() == 0 )  
    { 
        int i = 0;
        while( i < 100)
        {
            //semop(信号量，资源，数目)
            semop(emptyid , &P ,1 );
            //mutex实现临界资源的互斥使用
            semop(mutxid , &P , 1);

            array[(*set)%MAXSEM] = i + 1;

            printf("Producer %d\n", array[(*set)%MAXSEM]);
            //生产产品的标号+1
            (*set)++;
            semop(mutxid , &V , 1);
            semop(fullid , &V , 1);
            i++;  
        }
        sleep(10);
        printf("Producer is over");
        exit(0);
    } 
    else 
    {
        //ConsumerA  进程
        if(fork()==0) 
        {
            while(1)
            {
                semop(fullid , &P , 1);
                semop(mutxid , &P , 1);
                //判断是否所有产品都被消费了
                if(*get == 100) 
                    break;
                
                *sum += array[(*get)%MAXSEM];
    
                printf("The ComsumerA Get Number %d\n", array[(*get)%MAXSEM] );
                (*get)++;
                //判断这次消费是否为最后一次消费
                if( *get ==100)
                    printf("The sum is %d \n ", *sum);
                semop(mutxid , &V , 1);
                semop(emptyid , &V ,1 );
                sleep(1);
            }
            printf("ConsumerA is over");
            exit(0);
        }
        else 
        {
            //Consumer B进程
            if(fork()==0) 
            {
                while(1)
                {
                    semop(fullid , &P , 1);
                    semop(mutxid , &P , 1);
                    if(*get == 100)
                        break;
                    
                    *sum += array[(*get)%MAXSEM];
                    printf("The ComsumerB Get Number %d\n", array[(*get)%MAXSEM] );   
                    (*get)++;

                    if( *get ==100)
                        printf("The sum is %d \n ", *sum);
                    semop(mutxid , &V , 1);
                    semop(emptyid , &V ,1 );
                    sleep(1);
                }
                printf("ConsumerB is over");
                exit(0);
            }
        }
    }
    // sleep(20);
    return 0;
}