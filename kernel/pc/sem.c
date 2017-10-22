#include "sem.h"

#include <driver/vga.h>
#include <zjunix/syscall.h>
#include <zjunix/slab.h>
#include <zjunix/type.h>

sem_management_typedef sem_m;

//系统初始化信号量功能
void sem_init(void)
{
    int i;
    for(i = 0; i < SEM_MAX_NUM; i++)
    {
        sem_m.sem_state[i] = SEM_UNUSED;    //全部初始化为未使用
    }
    sem_m.num = 0;
}

//创建信号量，返回的是信号量的id，若为-1则创建失败
int create_sem(void)
{
    int i;
    if(sem_m.num >= SEM_MAX_NUM)        //信号量以达到上限
    {
        kernel_printf("error: no more space for new mem! \n");
        return -1;
    }
    for(i = 0; i < SEM_MAX_NUM; i++)
    {
        if(sem_m.sem_state[i] == SEM_UNUSED)
        {
            sem_m.sem_state[i] = SEM_USED;
            sem_m.num++;
            //将要分配的信号量初始化
            sem_m.sem[i].id = i;
            sem_m.sem[i].count = 0;
            sem_m.sem[i].wait_list.cnt = 0;
            sem_m.sem[i].wait_list.first = NULL;
            return i;
        }
    }
}

static void clean_sem_wait_list(int sem_id)
{
    sem_wait_node_typedef* p = sem_m.sem[sem_id].wait_list.first;
    sem_wait_node_typedef* tmp;
    while(p)
    {
        tmp = p;
        p = p->next;
        kfree(tmp);
    }
    sem_m.sem[sem_id].wait_list.first = NULL;
    sem_m.sem[sem_id].count = 0;
}

int delete_sem(int sem_id)
{
    if(sem_m.sem_state[sem_id] == SEM_UNUSED)
    {
        kernel_printf("error: sem %d is already unused!\n", sem_id);
        return -1;
    }
    clean_sem_wait_list(sem_id);
    sem_m.sem_state[sem_id] = SEM_UNUSED;
    sem_m.num--;
    return sem_id;
}

//发布信号量
int post_sem(int sem_id)
{
    if(sem_m.sem_state[sem_id] == SEM_UNUSED)
    {
        kernel_printf("error: sem %d is unused!\n", sem_id);
        return -1;
    }
    
}



