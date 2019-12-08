#include <stdio.h>

typedef struct
{
    unsigned arrive_time;   // 到达时间
    unsigned service_time;  // 服务时间
    unsigned remain_time;   // 剩余服务时间，用于时间片轮转法
    unsigned finish_time;   // 结束时间
    unsigned turnover_time; // 周转时间 
    unsigned weight;        // 权重
    float wct;              // 带权周转时间
    int tag;                // 标记，如果改进程已经计算则标记为1
} process;

void FCFS();

void RR();

int main(void)
{

    printf("进程调度算法\n");
    printf("1: FCFS算法\n");
    printf("2: 时间片轮转法\n");

    int choose = 0;
    scanf("%d", &choose);
    switch (choose)
    {
        case 1:
            FCFS();
            break;
        case 2:
            RR();
            break;    
        default:
            break;
    }
}

void print_result(process *processes, unsigned number);

void FCFS()
{
    unsigned i, j;
    unsigned index;     // 用于标记最小时间对应的进程号
    unsigned number;    // 进程数目
    unsigned time = 0;  // 标记当前计算的时间
    unsigned tmp;

    printf("输入进程个数\n");
    scanf("%d", &number);
    printf("输入每个进程的到达时间，服务时间\n");

    process processes[number];

    for (i = 0; i < number; i++)
    {
        printf("进程%d的到达时间，服务时间: ", i);
        scanf("%d%d", &processes[i].arrive_time, &processes[i].service_time);
        processes[i].tag = 0;
    }
    
    for (i = 0; i < number; i++)
    {
        tmp = __UINT32_MAX__;
        for (j = 0; j < number; j++)
        {
            if (processes[j].tag == 0 && processes[j].arrive_time < tmp)
            {
                tmp = processes[j].arrive_time;
                index = j;
            }
        }

        if (time > processes[index].arrive_time)
            processes[index].finish_time = time + processes[index].service_time;
        else
            processes[index].finish_time = processes[index].arrive_time + processes[index].service_time;

        time = processes[index].finish_time;

        processes[index].turnover_time = processes[index].finish_time - processes[index].arrive_time;
        processes[index].wct = 1.0f * processes[index].turnover_time / processes[index].service_time;
        processes[index].tag = 1;
    }
    print_result(processes, number);
}

void RR()
{
    unsigned i, j;
    unsigned number;    // 进程数目
    unsigned RR;        // 时间片大小
    unsigned time = 0;  // 当前运行时间
    unsigned offset = 0;     // Wait队列的偏移量
    unsigned count = 0;      // 记录计算完的进程个数
    unsigned N_serve = 0;    // 记录serve队列中的进程数目
    unsigned index = 0;

    printf("输入进程个数\n");
    scanf("%d", &number);
    printf("输入每个进程的到达时间，服务时间\n");

    unsigned wait[number];
    unsigned service[number];
    process processes[number];

    for (i = 0; i < number; i++)
    {
        printf("进程%d的到达时间，服务时间: ", i);
        scanf("%d%d", &processes[i].arrive_time, &processes[i].service_time);

        processes[i].tag = 0;
        processes[i].remain_time = processes[i].service_time;
        wait[i] = i;
    }

    printf("输入时间片大小");
    scanf("%d", &RR);

    for (i = 0; i < number; i++) 
    {
        for (j = 0; j < number - i; j++) 
        {
            if (processes[j].arrive_time > processes[j + 1].arrive_time) 
            {
                wait[i] ^= wait[j];
                wait[j] ^= wait[i];
                wait[i] ^= wait[j];
            }
        }
    }

    while (count < number)
    {
        if (N_serve == 0 && time < processes[wait[offset]].arrive_time)
            time = processes[wait[offset]].arrive_time;

        while (offset != number && time >= processes[wait[offset]].arrive_time)
        {
            service[offset] = wait[offset];
            N_serve++;
            offset++;
            if (offset == number)
                break;
        }

        for ( i = 0; i < offset; i++)
        {
            index = service[i];
            if (processes[index].tag == 0)
            {
                if (processes[index].remain_time > RR)
                {
                    processes[index].remain_time -= RR;
                    time += RR;                       
                }
                else
                {
                    time += processes[index].remain_time;
                    processes[index].remain_time = 0;
                    processes[index].finish_time = time;
                    processes[index].tag = 1;

                    processes[index].turnover_time = processes[index].finish_time - processes[index].arrive_time;
                    processes[index].wct = 1.0f * processes[index].turnover_time / processes[index].service_time;


                    N_serve--;
                    count++;
                }

                while (offset != number && time >= processes[wait[offset]].arrive_time)
                {
                    service[offset] = wait[offset];
                    N_serve++;
                    offset++;
                }
            }
        }
    }
    print_result(processes, number);
}

void print_result(process *processes, unsigned number)
{
    unsigned i;
    unsigned tmp_1 = 0;
    float tmp_2 = 0;
    float turnover_time_average;
    float wct_average;
    printf("进程\t到达时间\t服务时间\t结束时间\t周转时间\t带权周转时间\n");
    for (i = 0; i < number; i++) 
    {
        printf("%4d\t%8d\t%8d\t%8d\t%8d\t%8.2f\n", i, 
            processes[i].arrive_time, 
            processes[i].service_time, 
            processes[i].finish_time, 
            processes[i].turnover_time, 
            processes[i].wct);
        tmp_1 += processes[i].turnover_time;
        tmp_2 += processes[i].wct;
    }

    turnover_time_average = 1.0 * tmp_1 / number;
    wct_average = 1.0 * tmp_2 / number;
    printf("平均周转时间：%g,平均带权周转时间：%g\n", turnover_time_average, wct_average);
}