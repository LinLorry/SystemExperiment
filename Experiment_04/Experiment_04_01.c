#include "stdio.h"
#define N 5       //进程数目
float average[2]; //平均数
struct process 
{
    int rt;     //到达时间
    int st;     //服务时间
    int stl;    //剩余服务时间，用于时间片轮转法
    int ft;     //结束时间
    int ct;     //周转时间
    int weight; //权重
    float wct;  //带权周转时间
    int tag;    //标记，如果改进程已经计算则标记为1
} process;

struct process p[N]; //定义进程数组
int n;               //输入进程个数
void Menu();
void Start();
void FCFS();
void P_FCFS();
void RR();
void main() 
{
    Menu();
    int choose;
    scanf("%d", &choose);
    switch (choose) {
        case 1:
            FCFS();
            break;
        case 2:
            RR();
            default:
        break;
    }
}

void Menu() {
  printf("进程调度算法\n");
  printf("1: FCFS算法\n");
  printf("2: 时间片轮转法\n");
}

void FCFS() 
{
    int i; 
    for (i = 0; i < n; i++)
    {
        p[i].tag = 0;
    }
    printf("输入进程个数\n");
    scanf("%d", &n);
    printf("输入每个进程的到达时间，服务时间\n");
    for (i = 0; i < n; i++) 
    {
        scanf("%d%d", &p[i].rt, &p[i].st);
    }
    int temp = 65535; //用于比较时间先后
    int time = 0;     //标记当前计算的时间
    int index;        //用于标记最小时间对应的进程号
    for (i = 0; i < n; i++) 
    {
        temp = 65535;
        int j;
        for (j = 0; j < n; j++) 
        {
            if (p[j].tag == 0 && p[j].rt < temp) 
            {
                temp = p[j].rt;
                index = j;
            }
        }
        
        if (time > p[index].rt) 
        {
            p[index].ft = time + p[index].st; //计算该进程结束时间
        } 
        else 
        {
            p[index].ft = p[index].rt + p[index].st;
        }
        time = p[index].ft;                             //更新当前时间
        p[index].ct = p[index].ft - p[index].rt;        //计算进程周转时间
        p[index].wct = 1.0 * p[index].ct / p[index].st; //计算带权周转时间
        p[index].tag = 1; //将标记符设置为0，表示已经计算
    }
    P_FCFS();
}

void P_FCFS() {
    printf("进程     到达时间     服务时间     结束时间    周转时间     "
            "带权周转时间\n");
    int i;
    for (i = 0; i < n; i++) 
    {
        printf("%d        ", i);
        printf("%d            ", p[i].rt);
        printf("%d              ", p[i].st);
        printf("%d            ", p[i].ft);
        printf("%d           ", p[i].ct);
        printf("%g         \n", p[i].wct);
    }
    int temp1 = 0, temp2 = 0;
    for (i = 0; i < n; i++) 
    {
        temp1 += p[i].ct;
        temp2 += p[i].wct;
    }
    average[0] = 1.0 * temp1 / n;
    average[1] = 1.0 * temp2 / n;
    printf("平均周转时间：%g,平均带权周转时间：%g\n", average[0], average[1]);
}

void RR() {
    int i;
    for (i = 0; i < n; i++) 
    {
        p[i].tag = 0;
    }

    printf("输入进程个数\n");
    scanf("%d", &n);
    printf("输入每个进程的到达时间，服务时间\n");

    for (i = 0; i < n; i++) 
    {
        scanf("%d%d", &p[i].rt, &p[i].st);
    }
    printf("输入时间片大小");
    int RR = 0;
    scanf("%d", &RR);
    int Wait[n];
    for (i = 0; i < n; i++) 
    {
        Wait[i] = i;
        p[i].stl = p[i].st;
    }
    int Serve[n];
    int m = n;
    int j;
    for (i = 0; i < n; i++) 
    {
        m--;
        for (j = 0; j < m; j++) 
        {
            if (p[j].rt > p[j + 1].rt) 
            {
                Wait[i] ^= Wait[j];
                Wait[j] ^= Wait[i];
                Wait[i] ^= Wait[j];
            }
        }
    }
    int time = 0;    //当前运行时间
    int offset = 0;  // Wait队列的偏移量
    int count = 0;   //记录计算完的进程个数
    int N_serve = 0; //记录serve队列中的进程数目
    while (count < n) 
    {
        if (N_serve == 0) 
        {
            if (time < p[Wait[offset]].rt)
                time = p[Wait[offset]].rt;
        }

        while(offset != n && time >= p[Wait[offset]].rt) 
        {
            Serve[offset] = Wait[offset];
            N_serve++;
            offset++;
            if (offset == n)
                break;
        }

        for(i = 0 ; i < offset; i++){
            if (Serve[i] != -1) 
            {
                if (p[Serve[i]].tag == 0) 
                {
                    if (p[Serve[i]].stl > RR) 
                    { //剩余的服务时间大于时间片
                        p[Serve[i]].stl -= RR;
                        time += RR; //时间增加
                    } 
                    else 
                    {
                        time += p[Serve[i]].stl;
                        p[Serve[i]].stl = 0; //程序运行完成
                        p[Serve[i]].ft = time;
                        p[Serve[i]].tag = 1; //更改标志为1,表示已经计算完成
                        Serve[i] =
                            -1; //将serve队列中进程出队（由于没用真正队列直接使用标记-1表示已经出队列了
                        N_serve--;
                        count++; //表示进程已经计算完的个数加一
                    }

                    while (offset != n && time >= p[Wait[offset]].rt) {
                        Serve[offset] = Wait[offset];
                        N_serve++;
                        offset++;
                    }
                }
            }
        }
    } //所有进程都已经serve完
  //计算周转时间，带权周转时间
    for (i = 0; i < n; i++) 
    {
        p[i].ct = p[i].ft - p[i].rt;
        p[i].wct = 1.0 * p[i].ct / p[i].st;
    }
    P_FCFS();
}