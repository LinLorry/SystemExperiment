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

short change_resources(int request[RESOURCE_NUM], int available[RESOURCE_NUM], 
    int allocation[RESOURCE_NUM], int need[RESOURCE_NUM]);

void output_unchange(int index);

int intput_request_index();

void input_request(int request[RESOURCE_NUM], int need[RESOURCE_NUM]);

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
        
        printf("是否还有Request？\n是输入1，否输入0：\n");
        scanf("%d", &ch);
        if (ch == '0') break;
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
    printf("资源情况  Max        Allocation  Need        Available    \n");
	printf("进程      A  B  C    A  B  C     A  B  C     A  B  C     \n");

    for (size_t i = 0; i < REQUEST_NUM; i++)
    {
        printf("P%d", i);

        for (size_t j = 0; j < RESOURCE_NUM; j++)
        {
            if (j == 0)
                printf("%9d", max[i][0]);
            else
                printf("%3d", max[i][j]);
        }

        for (size_t j = 0; j < RESOURCE_NUM; j++)
        {
            if (j == 0)
                printf("%5d", allocation[i][0]);
            else
                printf("%3d", allocation[i][j]);
        }

        for (size_t j = 0; j < RESOURCE_NUM; j++)
        {
            if (j == 0)
                printf("%6d", need[i][0]);
            else
                printf("%3d", need[i][j]);
        }

		if (i == 0) 
        {
            for (size_t j = 0; j < RESOURCE_NUM; j++)
            {
                if (j == 0)
                    printf("%6d", available[0]);
                else
                    printf("%3d", available[j]);
            }
        }
		printf("\n");
    }
}

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
    printf("请求资源数大于现有资源数，进程%d进入等待。", index);
}

int intput_request_index()
{
    size_t index = 0;
    while (1)
    {
        printf("请输入请求进程号(0-%d)：", REQUEST_NUM - 1);
        scanf("%d", &index);

        if (index < REQUEST_NUM) return index;
        else printf("请求进程号输入错误，请重新输入！\n");
    }
    return index;
}

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
            printf("请求资源数大于该进程需要的数目！请重新输入！\n");
        else break;   
    }
}

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
                for ( k = 0; k < RESOURCE_NUM; k++)
                {
                    works[j][k] = work[k];
                    work[k] += allocation[j][k];
                    finish[j] = 1;
                }
            }

    return index < REQUEST_NUM;
}

void output_safe(int safe[REQUEST_NUM], 
    int works[REQUEST_NUM][RESOURCE_NUM], 
    int allocation[REQUEST_NUM][RESOURCE_NUM], 
    int need[REQUEST_NUM][RESOURCE_NUM])
{
    printf("找到安全序列:");
    for (size_t i = 0; i < REQUEST_NUM; i++)
    {
        if (i == 0) printf("P%d", safe[i]);
        else printf("->P%d", safe[i]);
    }

    printf("\n该序列的资源分配如下：\n");

	printf("资源情况    Work      Need      Allocation   Work+Allocation    Finish\n");
	printf("进程      A  B  C    A  B  C     A  B  C      A  B  C\n");

    for (size_t i = 0; i < REQUEST_NUM; i++)
    {
        const size_t index = safe[i];
        printf("P%d", index);
        for (size_t j = 0; j < RESOURCE_NUM; j++)
        {
            if (j == 0) printf("%9d", works[index][0]);
            else printf("%3d", works[index][j]);
        }

        for (size_t j = 0; j < RESOURCE_NUM; j++)
        {
            if (j == 0) printf("%5d", need[index][0]);
            else printf("%3d", need[index][j]);
        }
        
        for (size_t j = 0; j < RESOURCE_NUM; j++)
        {
            if (j == 0) printf("%5d", allocation[index][0]);
            else printf("%3d", allocation[index][j]);
        }

        for (size_t j = 0; j < RESOURCE_NUM; j++)
        {
            if (j == 0) printf("%6d", works[index][0] + allocation[index][0]);
            else printf("%3d", works[index][j] + allocation[index][j]);
        }
        printf("true\n");
    }
}

void output_unsafe()
{
    printf("找不到安全序列，系统处于不安全状态。\n回滚修改！\n");
}

void roll_to_safe(int request[RESOURCE_NUM], 
    int available[RESOURCE_NUM], int allocation[RESOURCE_NUM])
{
    for (size_t i = 0; i < RESOURCE_NUM; i++)
    {
        allocation[i] -= request[i];
        available[i] += request[i];
    }
}
