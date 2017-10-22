#include "sem.h"

#include <driver/vga.h>
#include <intr.h>
#include <zjunix/syscall.h>
#include <zjunix/slab.h>
#include <zjunix/type.h>
#include <zjunix/pc.h>

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
        sem[i].num = 0;
        sem[i].wait_list = NULL;
    }
    sem_num = 0;
}

//创建信号量，max_num为信号量的容量，返回的是信号量的id，若为-1则创建失败
int create_sem(int max_num)
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
            sem[i].max_num = max_num;
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
    sem[id].num = 0;
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

    //异常状况
    if(id < 0 || id >= SEM_MAX_NUM)
    {
        kernel_printf("error: invalid sem id! \n");
        return -1;
    }
    if(sem[id].state == SEM_UNUSED)
    {
        kernel_printf("error: sem %d is unused!\n", id);
        return -1;
    }
    if(sem[id].num >= sem[id].max_num)  //信号量已满
    {
        kernel_printf("error: this sem is already full!\n");
        return -1;
    }

    //正常发布

    //这里要不要加锁，或是要不要关中断，或是要不要关调度器有待商榷
    //实际上这个函数应当是使用系统调用调用的，系统调用能否嵌套
    //如果能够嵌套，则必须得有锁机制，不然会乱
    //如果不能嵌套，则没关系

    disable_interrupts();       //进入临界段

    sem[id].num++;

    //没有进程等待该信号量
    if(sem[id].wait_list == NULL)
    {
        ;
    }
    //有进程等待该信号量
    else
    {
        sem_wait_node_typedef* p = sem[id].wait_list;
        while(sem[id].num > 0 && p)     //尚有信号量且等待列表不为空
        {
            wakeup_proc(p->asid);       //唤醒等待的进程
            sem[id].num--;
            p = p->next;                //删除一个结点
            kfree(sem[id].wait_list);
            sem[id].wait_list = p;
        }
    }

    enable_interrupts();        //退出临界段

    return 0;
    
}

//获取信号量，目前先写成无限等待的方式的
int task_sem(int id)
{
    //异常状况
    if(id < 0 || id >= SEM_MAX_NUM)
    {
        kernel_printf("error: invalid sem id! \n");
        return -1;
    }
    if(sem[id].state == SEM_UNUSED)
    {
        kernel_printf("error: sem %d is unused!\n", id);
        return -1;
    }
    
    
    //正常获取

    disable_interrupts();   //进入关键段

    //有多余的信号量
    if(sem[id].num > 0)
    {
        sem[id].num--;
        enable_interrupts();    //退出关键段
        return 0;
    }
    //没有多余的信号量
    else
    {
        sem_wait_node_typedef* p = (sem_wait_node_typedef*)kmalloc(sizeof(sem_wait_node_typedef));
        int this_proc = get_curr_proc_num();  //进行此系统调用的进程号
        p->next = sem[id].wait_list;    //加入等待列表
        sem[id].wait_list = p;
        block_proc(this_proc);
        enable_interrupts();    //退出关键段
        while(get_proc_state(this_proc) == TASK_BLOCKED);   //等待调度器调度走这个进程
        return 0;
        
        
    }

    
}



