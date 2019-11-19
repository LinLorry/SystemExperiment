//标准输入输入
#include<stdio.h>
//标准库函数
#include<stdlib.h>
//Unix、Linux相关的系统调用函数
#include<unistd.h>
#include<time.h>
//Unix、Linux系统的基本系统数据类型的头文件，
#include<sys/types.h>
//Unix、Linux的系统等待函数
#include<sys/wait.h>
//信号量操作的相关函数
#include<linux/sem.h>
//子进程数目
#define NUM_PROCS 5
//信号量ID
#define SEM_ID    250
#define FILE_NAME "/tmp/sem_aaa"
#define DELAY  4000000
void update_file(int sem_set_id, char *file_path, int number){
	struct sembuf sem_op;
	//建立一个文件指针
	FILE *file;
	//等待信号量的数值变为非负数，此处设为负值，相当于对信号量进行P操作
	sem_op.sem_num=0;
	sem_op.sem_op=-1;
	sem_op.sem_flg=0;
	/* semop(sem_set_id,&sem_op,1) 操作一组信号；sem_set_id是进程的标识符，sem_op是结构指针。sem_op：如果其值为正数，该值会加到现有的信号内含值中。通常用于释放所控资源的使用权；如果sem_op的值为负数，而其绝对值又大于信号的现值，操作将会阻塞，直到信号值大于或等于sem_op的绝对值。通常用于获取资源的使用权；如果sem_op的值为0，则操作将暂时阻塞，直到信号的值变为0。*/

	semop(sem_set_id,&sem_op,1);
	//写文件，若成功则将当前的进程号写入文件中并输出
	file=fopen(file_path,"w");
	if(file){//临界区
		//将当前的进程号写入文件中并输出
		fprintf(file,"%d\n",number);
		printf("%d\n",number);
		fclose(file);
	}
	//发送信号，把信号量的数值加1，此处相当于对信号量进行V操作
	sem_op.sem_num=0;
	sem_op.sem_op=1;
	sem_op.sem_flg=0;
	semop(sem_set_id,&sem_op,1);
}
//子进程写文件
void do_child_loop(int sem_set_id,char *file_name){
//getpid() 获取当前进程的ID
	pid_t pid=getpid();
	int i,j;
	for(i=0;i<3;i++){
		update_file(sem_set_id,file_name,pid);
		for(j=0;j<4000000;j++);
	}
}

int main(int argc,char **argv)
{
	int sem_set_id;  //信号量集的ID
	union semun sem_val;  //信号量的数值，用于semctl()
	int child_pid;
	int i;
	int rc;
	// 建立信号量集，ID是250，其中只有一个信号量
	sem_set_id=semget(SEM_ID,1,IPC_CREAT|0600);
	//如果调用失败，输出错误类型并退出程序

	if(sem_set_id==-1){
		perror("main: semget");
		exit(1);
	}
	//把第一个信号量的数值设置为1
	sem_val.val=1;
	rc=semctl(sem_set_id,0,SETVAL,sem_val);
	//测试是否调用成功
	if(rc==-1)
	{
		perror("main:semctl");
		exit(1);
	}
	//建立一些子进程，使它们可以同时以竞争的方式访问信号量
	for(i=0;i<NUM_PROCS;i++){
		//创建子进程（通过 fork()函数 ），成功返回0，否则返回-1
		child_pid=fork();
		switch(child_pid){
		case -1:
		       perror("fork");
		case 0:   //子进程写文件
			do_child_loop(sem_set_id,FILE_NAME);
			exit(0);
		default:  //父进程接着运行
			break;
		}
	}
	//等待子进程结束
	for(i=0;i<NUM_PROCS;i++){
		int child_status;
		wait(&child_status);
	}
	printf("main:we're done\n");
	fflush(stdout);
	return 0;
}