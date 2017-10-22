#include "sem.h"

#include <driver/vga.h>
#include <zjunix/syscall.h>
#include <zjunix/slab.h>
#include <zjunix/type.h>

sem_typedef sem[SEM_MAX_NUM];       //信号量实体
unsigned int  sem_num;      //已注册的信号量的数量

//系统初始化信号量功能
void sem_init(void)
{
    int i;
    for(i = 0; i < SEM_MAX_NUM; i++)
    {
        sem[i].state = SEM_UNUSED;
        sem[i].id = i;
        sem[i].count = 0;
        sem[i].wait_list = NULL;
    }
    sem_num = 0;
}

//创建信号量，返回的是信号量的id，若为-1则创建失败
int create_sem(void)
{
    int i;
    if(sem_num >= SEM_MAX_NUM)      //信号量已达到上限
    {
        kernel_printf("error: no more space for new mem! \n");
        return -1;
    }
    for(i = 0; i < SEM_MAX_NUM; i++)
    {
        if(sem[i].state = SEM_UNUSED)
        {
            sem[i].state = SEM_USED;
            sem_num++;
            return i;
        }
    }
}

//清空信号量等待列表
static void clean_sem_wait_list(int id)
{
    sem_wait_node_typedef* p = sem[id].wait_list;
    sem_wait_node_typedef* tmp;
    while(p)
    {
        tmp = p;
        p = p->next;
        kfree(tmp);
    }
    sem[id].wait_list = NULL;
    sem[id].count = 0;
}

//删除一个信号量
int delete_sem(int id)
{
    if(id < 0 || id >= SEM_MAX_NUM)
    {
        kernel_printf("error: invalid sem id! \n");
        return -1;
    }
    if(sem[id].state == SEM_UNUSED)
    {
        kernel_printf("error: sem %d is already unused!\n", id);
        return -1;
    }
    clean_sem_wait_list(id);
    sem[id].state = SEM_UNUSED;
    sem_num--;
    return id;
}

//发布信号量
int post_sem(int id)
{
    if(sem[id].state == SEM_UNUSED)
    {
        kernel_printf("error: sem %d is unused!\n", id);
        return -1;
    }
    
}


