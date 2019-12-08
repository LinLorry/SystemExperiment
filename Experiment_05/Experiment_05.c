#include <stdio.h>
#include <stdlib.h>
#include <string.h> //1字符串头文件
#include <unistd.h>
#ifndef _UNISTD_H
#define _UNISTD_H
#include <IO.H>      //1io.h主要定义一些和缓冲区相关的读写函数
#include <PROCESS.H> //1process.h 是包含用于和宏指令的作用声明
#endif
#define TRUE 1
#define FALSE 0
#define INVALID -1
#define total_instruction 320
#define total_vp 32
#define clear_period 50

typedef struct //页面结构
{
    int pn,  //页面序号
        pfn, //页面所在内存区的页框号
        counter, time;
} pl_type;

pl_type pl[total_vp]; //页面结构数组

struct pfc_struct {     //页面控制结构
    int pn,             //页面号
        pfn;            //内存区页面的帧号
    struct pfc_struct *next;
};

typedef struct pfc_struct pfc_type;
pfc_type pfc[total_vp], *freepf_head, *busypf_head, *busypf_tail;
int diseffect;
int a[total_instruction];//指令序号数组
int page[total_instruction];
int offset[total_instruction];
int initialize(int);

int FIFO(int);//不同的页面置换算法
int LRU(int);
int OPT(int);
int CLOCK(int);

int main(void) 
{
    int s;
    int i;
    srand(10 *getpid()); // 1每次运行时进程号不同，用来作为初始化随机数队列的"种子

    s = (int)((float)(total_instruction - 1) * (rand() / (RAND_MAX + 1.0)));
    printf("\n------------随机产生指令流------------\n");

    for (i = 0; i < total_instruction; i += 4) //产生指令队列
    {
        a[i] = s;            //任选一指令访问点m
        a[i + 1] = a[i] + 1; //顺序执行一条指令

        a[i + 2] =(int)((float)a[i] * (rand() / (RAND_MAX + 1.0))); //执行前地址指令m'
        a[i + 3] = a[i + 2] + 1; //顺序执行一条指令
        printf("%6d%6d%6d%6d\n", a[i], a[i + 1], a[i + 2], a[i + 3]);
        s = (int)((float)((total_instruction - 1) - a[i + 2]) *(rand() / (RAND_MAX + 1.0))) + a[i + 2];
    }

    printf("--------------------------------------\n");
    for (i = 0; i < total_instruction; i++) //将指令序列变换成页地址流
    {
        page[i] = a[i] / 10; //第几页
        offset[i] = a[i] % 10;
    }

    printf("\n--不同页面工作区各种替换策略的命中率表--\n");
    printf("Page\t FIFO\t LRU\t OPT\t CLOCK\n");
    for (i = 4; i <= 32; i++) //用户内存工作区从个页面到个页面
    {//i作为内存中的页框数
        printf(" %2d \t", i);
        FIFO(i);
        LRU(i);
        OPT(i);
        CLOCK(i);
        printf("\n");
    }
    return 0;
}

//初始化页面结构数组和页面控制结构数组
// total_pf;  用户进程的内存页面数
int initialize(int total_pf) 
{
    int i;
    diseffect = 0;
    for (i = 0; i < total_vp; i++) //对所有的页
    {
        pl[i].pn = i;
        pl[i].pfn = INVALID;
        pl[i].counter = 0;//访问次数置0
        pl[i].time = -1;//存在时间设为-1
    }

    for (i = 0; i < total_pf - 1; i++)  //所有的页块进行链式链接
    {
        pfc[i].next = &pfc[i + 1];
        pfc[i].pfn = i;
    }//默认选择的是前i个页面共存于内存中

    pfc[total_pf - 1].next = NULL;//最后一页不链接其他页框
    pfc[total_pf - 1].pfn = total_pf - 1;//最后一个页框，pfn设为编号值，VALID化
    freepf_head = &pfc[0];//指向第一个页框的指针
    return 0;
}

int LRU(int total_pf) 
{
    int MinT;
    int MinPn; //最小访问时间对应的叶号
    int i, j;
    int CurrentTime;
    initialize(total_pf);//初始化
    CurrentTime = 0;
    diseffect = 0;

    for (i = 0; i < total_instruction; i++) 
    {
        if (pl[page[i]].pfn == INVALID) 
        {
            diseffect++;//产生一次缺页
            if (freepf_head == NULL) 
            {//说明内存中一页都没有空闲
                MinT = 100000;//保证这一次读取到的页能够进入内存

                for (j = 0; j < total_vp; j++) 
                {
                    if (MinT > pl[j].time && pl[j].pfn != INVALID) 
                    {
                        MinT = pl[j].time;//最小访问时间
                        MinPn = j;
                    }//通过便利，找出最小访问时间的夜
                }

                freepf_head = &pfc[pl[MinPn].pfn];
                pl[MinPn].pfn = INVALID;//内存中无该页面，换出l
                pl[MinPn].time = -1;
                freepf_head->next = NULL;
            }

            pl[page[i]].pfn = freepf_head->pfn;//换入，同时修改信息
            pl[page[i]].time = CurrentTime;
            freepf_head = freepf_head->next;
        } 
        else
            pl[page[i]].time = CurrentTime;
        CurrentTime++;
    }
    printf("%6.3f\t", 1 - (float)diseffect / 320);//用1减去缺页率；
    return 0;
}

int OPT(int total_pf) 
{//最理想的结果
    int i, j;
    int MaxD;
    int MaxPn;
    int dis;
    int dist[total_vp];
    initialize(total_pf);
    diseffect = 0;
    for (i = 0; i < total_instruction; i++) 
    {
        if (pl[page[i]].pfn == INVALID) //页面失效
        {
            diseffect++;
            if (freepf_head == NULL) 
            {
                for (j = 0; j < total_vp; j++) 
                {
                    if (pl[j].pfn != INVALID)
                    dist[j] = 100000;
                    else
                    dist[j] = 0;
                }
                dis = 1;
                for (j = i + 1; j < total_instruction; j++) 
                {
                    if (pl[page[j]].pfn != INVALID && pl[page[j]].counter == 0) 
                    {
                        dist[page[j]] = dis;
                        pl[page[j]].counter = 1;
                    }
                    dis++;
                }
                MaxD = -1;
                for (j = 0; j < total_vp; j++) 
                {
                    pl[j].counter = 0;
                    if (MaxD < dist[j]) 
                    {
                        MaxD = dist[j];
                        MaxPn = j;
                    }
                }
                freepf_head = &pfc[pl[MaxPn].pfn];
                freepf_head->next = NULL;
                pl[MaxPn].pfn = INVALID;
            }
            pl[page[i]].pfn = freepf_head->pfn;
            freepf_head = freepf_head->next;
        } // if
    }   // for
    printf("%6.3f\t", 1 - (float)diseffect / 320);
    return 0;
}

int CLOCK(int total_pf) 
{
    int i;
    int use[total_vp];
    int swap;
    swap = 0;
    initialize(total_pf);
    pfc_type *pnext;
    pfc_type *head;
    pnext = freepf_head;
    head = freepf_head;
    for (i = 0; i < total_vp; i++) 
    {
        use[i] = 0;
    }//都没有使用过
    diseffect = 0;
    for (i = 0; i < total_instruction; i++) 
    {
        if (pl[page[i]].pfn == INVALID) 
        {
            diseffect++;//缺页
            if (freepf_head == NULL) 
            {
                while (use[pnext->pfn] == 1) 
                {//访问过，就改为0
                    use[pnext->pfn] = 0;
                    pnext = pnext->next;//下一页
                    if (pnext == NULL)
                        pnext = head;
                }
                pl[pnext->pn].pfn = INVALID;
                swap = 1;
            }
            if (use[pnext->pfn] == 0) 
            {
                pl[page[i]].pfn = pnext->pfn;
                pnext->pn = page[i];
                use[pnext->pfn] = 1; //重置使用位为
                pnext = pnext->next;
                if (pnext == NULL)
                    pnext = head;
                if (swap == 0)
                    freepf_head = freepf_head->next;
            }

        } 
        else 
        { //页面在主存中
            use[pl[page[i]].pfn] = 1;
        }
    }
    printf("%6.3f\t", 1 - (float)diseffect / 320);
    return 0;
}

int FIFO(int total_pf)
{
    int i;
    int use[total_vp];
    int swap = 0;
    initialize(total_pf);

    pfc_type *pnext, *head;
    pnext = freepf_head;
    head = freepf_head;
    for (i = 0; i < total_vp; i++) 
    {
        use[i] = 0;
    }
    diseffect = 0;

    for (i = 0; i < total_instruction; i++) 
    {//开始对每一条指令进行操作
        if (pl[page[i]].pfn == INVALID) 
        {
            diseffect++;//缺页
            if (freepf_head == NULL) 
            {//
                while (use[pnext->pfn] == 1) 
                {//
                    use[pnext->pfn] = 0;
                    pnext = pnext->next;
                    if (pnext == NULL)
                        pnext = head;
                }

                pl[pnext->pn].pfn = INVALID;
                swap = 1;//标记一下，要被换出了
            }
            if (use[pnext->pfn] == 0) 
            {
                pl[page[i]].pfn = pnext->pfn;
                pnext->pn = page[i];
                use[pnext->pfn] = 1;

                pnext = pnext->next;
                if (pnext == NULL)
                    pnext = head;
                if (swap == 0)
                    freepf_head = freepf_head->next;
            }
        }
            //访问成功，不进行任何操作
    }
    printf("%6.3f\t", 1 - (float)diseffect / 320);
    return 0;
}