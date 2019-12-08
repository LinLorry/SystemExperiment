#include <stdio.h>
#include <stdlib.h>

#define CLEAR() printf("\033[2J")

#define REQUEST_NUM 5
#define RESOURCE_NUM 3

void init_resources(int available[RESOURCE_NUM], 
    int max[REQUEST_NUM][RESOURCE_NUM], 
    int need[REQUEST_NUM][RESOURCE_NUM]);

void output_resources(int max[REQUEST_NUM][RESOURCE_NUM], 
    int allocation[REQUEST_NUM][RESOURCE_NUM], 
    int need[REQUEST_NUM][RESOURCE_NUM], 
    int available[RESOURCE_NUM]);

int intput_request_index();

void input_request(int request[RESOURCE_NUM], int need[RESOURCE_NUM]);

short change_resources(int request[RESOURCE_NUM], int available[RESOURCE_NUM], 
    int allocation[RESOURCE_NUM], int need[RESOURCE_NUM]);

void output_unchange(int index);

short check_safe(int available[RESOURCE_NUM], 
    int allocation[REQUEST_NUM][RESOURCE_NUM], 
    int need[REQUEST_NUM][RESOURCE_NUM], 
    int safe[REQUEST_NUM], 
    int works[REQUEST_NUM][RESOURCE_NUM]);

void output_safe(int safe[REQUEST_NUM], 
    int works[REQUEST_NUM][RESOURCE_NUM], 
    int allocation[REQUEST_NUM][RESOURCE_NUM], 
    int need[REQUEST_NUM][RESOURCE_NUM]);

void output_unsafe();

void roll_to_safe(int request[RESOURCE_NUM], 
    int available[RESOURCE_NUM], 
    int allocation[RESOURCE_NUM]);

int main()
{
    int max[REQUEST_NUM][RESOURCE_NUM] = { 0 };
    int allocation[REQUEST_NUM][RESOURCE_NUM] = { 0 };
    int need[REQUEST_NUM][RESOURCE_NUM] = { 0 };
    int available[RESOURCE_NUM] = { 0 };
    int request[RESOURCE_NUM] = { 0 };

    int works[REQUEST_NUM][RESOURCE_NUM];
    int safe[REQUEST_NUM];

    int request_index = 0;
    int ch;

    CLEAR();
    printf("银行家算法开始：\n");
    
    init_resources(available, max, need);

    while (1)
    {
        output_resources(max, allocation, need, available);

        request_index = intput_request_index();

        input_request(request, need[request_index]);

        if (!change_resources(request, available, 
            allocation[request_index], need[request_index]))
        {
            if (!check_safe(available, allocation, need, safe, works))
            {
                output_safe(safe, works, allocation, need);
            }
            else
            {
                output_unsafe();
                roll_to_safe(request, available, allocation[request_index]);
            }
        }
        else
        {
            output_unchange(request_index);
        }
        while ((ch = getchar()) != EOF && ch != '\n');
        
        printf("是否还有请求？[y/n]：\n");
        scanf("%c", &ch);
        if (ch == 'n' || ch == 'N') break;
    }

    return 1;
}

void init_resources(int available[RESOURCE_NUM], 
    int max[REQUEST_NUM][RESOURCE_NUM], 
    int need[REQUEST_NUM][RESOURCE_NUM])
{
    printf("请输入A、B、C资源的数量：");
    for (int *p = available; p < available + RESOURCE_NUM; p++)
        scanf("%d", p);

    for (size_t i = 0; i < REQUEST_NUM; i++)
    {
        printf("请输入进程%d最大所需要的A、B、C资源数：", i);
        for (size_t j = 0; j < RESOURCE_NUM; j++)
        {
            scanf("%d", max[i] + j);
            need[i][j] = max[i][j];
        }
    }
}

void output_resources(int max[REQUEST_NUM][RESOURCE_NUM], 
    int allocation[REQUEST_NUM][RESOURCE_NUM], 
    int need[REQUEST_NUM][RESOURCE_NUM], 
    int available[RESOURCE_NUM])
{
    CLEAR();
    printf("Resources\t    Max  \t Allocation\t   Need  \t Available\n");
	printf("Process  \t  A  B  C\t  A  B  C  \t  A  B  C\t  A  B 0\n");

    for (size_t i = 0; i < REQUEST_NUM; i++)
    {
        printf("P%d       \t", i);

        for (size_t j = 0; j < RESOURCE_NUM; j++)
        {
            printf("%3d", max[i][j]);
        }

        printf("\t");

        for (size_t j = 0; j < RESOURCE_NUM; j++)
        {
            printf("%3d", allocation[i][j]);
        }

        printf("  \t");

        for (size_t j = 0; j < RESOURCE_NUM; j++)
        {
            printf("%3d", need[i][j]);
        }

		if (i == 0) 
        {
            printf("\t");

            for (size_t j = 0; j < RESOURCE_NUM; j++)
            {
                printf("%3d", available[j]);
            }
        }
		printf("\n");
    }
}

// 获取请求进程号，并返回进程号
int intput_request_index()
{
    if (REQUEST_NUM == 1) return 0;

    size_t index = 0;
    while (1)
    {
        printf("请输入请求进程号(0-%d)：", REQUEST_NUM - 1);
        scanf("%d", &index);

        if (index < REQUEST_NUM) return index;
        else 
        {
            printf("请求进程号输入错误，请重新输入！\n");
            char ch;
            while ((ch = getchar()) != EOF && ch != '\n');
        }
    }
    return index;
}

// 输入请求的资源数，并对资源的请求量进行初步的判断
void input_request(int request[RESOURCE_NUM], 
    int need[RESOURCE_NUM])
{
    size_t i;
    while (1)
    {
        printf("请按照A、B、C的顺序输入请求资源数：");
        for (i = 0; i < RESOURCE_NUM; i++)
        {
            scanf("%d", &request[i]);
            if (request[i] > need[i]) break;
        }

        if (i < RESOURCE_NUM) 
        {
            printf("请求资源数大于该进程需要的数目！请重新输入！\n");
            char ch;
            while ((ch = getchar()) != EOF && ch != '\n');
        }
        else break;   
    }
}

// 修改成功返回0，否则返回1
short change_resources(int request[RESOURCE_NUM], int available[RESOURCE_NUM], 
    int allocation[RESOURCE_NUM], int need[RESOURCE_NUM])
{
    size_t i;
    for (i = 0; i < RESOURCE_NUM; i++)
        if (request[i] > available[i]) break;
    
    if (i < RESOURCE_NUM) return 1;
    else
    {
        for (i = 0; i < RESOURCE_NUM; i++)
        {
            available[i] -= request[i];
            allocation[i] += request[i];
            need[i] -= request[i];
        }
    }
    return 0;
}

void output_unchange(int index)
{
    printf("请求资源数大于现有资源数，进程%d进入等待。\n", index);
}

// 进行银行家算法，对请求进行判断
short check_safe(int available[RESOURCE_NUM], 
    int allocation[REQUEST_NUM][RESOURCE_NUM], 
    int need[REQUEST_NUM][RESOURCE_NUM], 
    int safe[REQUEST_NUM], int works[REQUEST_NUM][RESOURCE_NUM])
{
    int work[RESOURCE_NUM];
    int finish[REQUEST_NUM] = { 0 };

    for (size_t i = 0; i < RESOURCE_NUM; i++)
        work[i] = available[i];

    size_t index = 0;

    for (size_t i = 0; i < REQUEST_NUM; i++)
        for (size_t j = 0; j < REQUEST_NUM; j++)
            if (!finish[j])
            {
                size_t k;
                for (k = 0; k < RESOURCE_NUM; k++)
                    if (need[j][k] > work[k]) break;
                if (k < RESOURCE_NUM) continue;

                safe[index++] = j;
                for (k = 0; k < RESOURCE_NUM; k++)
                {
                    works[j][k] = work[k];
                    work[k] += allocation[j][k];
                    finish[j] = 1;
                }
            }

    return index < REQUEST_NUM;
}

// 经过银行家算法确认安全后将安全序列输出
void output_safe(int safe[REQUEST_NUM], 
    int works[REQUEST_NUM][RESOURCE_NUM], 
    int allocation[REQUEST_NUM][RESOURCE_NUM], 
    int need[REQUEST_NUM][RESOURCE_NUM])
{
    CLEAR();
    printf("找到安全序列: ");
    for (size_t i = 0; i < REQUEST_NUM; i++)
    {
        if (i == 0) printf("P%d", safe[i]);
        else printf("->P%d", safe[i]);
    }

    printf("\n该序列的资源分配如下：\n");

    printf("Resources\t    Work \t   Need  \tAllocation\tWork+Allocation\tFinish\n");
	printf("Process  \t  A  B  C\t  A  B  C\t  A  B  C \t  A  B  C      \t\n");

	// printf("资源情况    Work      Need      Allocation   Work+Allocation    Finish\n");
	// printf("进程      A  B  C    A  B  C     A  B  C      A  B  C\n");

    for (size_t i = 0; i < REQUEST_NUM; i++)
    {
        const size_t index = safe[i];
        printf("P%d       \t", index);

        for (size_t j = 0; j < RESOURCE_NUM; j++)
        {
            printf("%3d", works[index][j]);
        }

        printf("\t");

        for (size_t j = 0; j < RESOURCE_NUM; j++)
        {
            printf("%3d", need[index][j]);
        }

        printf("\t");
        
        for (size_t j = 0; j < RESOURCE_NUM; j++)
        {
            printf("%3d", allocation[index][j]);
        }

        printf(" \t");

        for (size_t j = 0; j < RESOURCE_NUM; j++)
        {
           printf("%3d", works[index][j] + allocation[index][j]);
        }

        printf("      \ttrue\n");
    }
}

// 经过银行家算法确认不安全后将不安全的状态输出
void output_unsafe()
{
    printf("找不到安全序列，系统处于不安全状态。\n回滚修改！\n");
}

// 经过银行家算法确认不安全后状态回滚
void roll_to_safe(int request[RESOURCE_NUM], 
    int available[RESOURCE_NUM], int allocation[RESOURCE_NUM])
{
    for (size_t i = 0; i < RESOURCE_NUM; i++)
    {
        allocation[i] -= request[i];
        available[i] += request[i];
    }
}
