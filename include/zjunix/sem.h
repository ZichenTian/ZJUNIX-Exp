#ifndef _ZJUNIX_SEM_H
#define _ZJUNIX_SEM_H

//信号量等待列表中的节点
typedef struct sem_wait_node{
    unsigned int asid;      //等待信号量的进程的ID
    struct sem_wait_node* next;
}sem_wait_node_typedef;


//信号量
typedef struct sem{
    unsigned int id;        //信号量ID，内核通过ID寻找到信号量
    unsigned int num;     //信号量的数量
    unsigned int max_num;   //信号量容量
    unsigned char state;    //信号量是否已经被注册， SEM_USED or SEM_UNUSED
    sem_wait_node_typedef* wait_list;    //等待列表
}sem_typedef;

#define SEM_MAX_NUM 10      //信号量最大数量

#define SEM_USED    1
#define SEM_UNUSED  0

extern sem_typedef sem[SEM_MAX_NUM];       //信号量实体
extern unsigned int  sem_num;      //已注册的信号量的数量    

void sem_init(void);


#endif
