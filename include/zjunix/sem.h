#ifndef _ZJUNIX_SEM_H
#define _ZJUNIX_SEM_H

//信号量等待列表中的节点
typedef struct sem_wait_node{
    unsigned int asid;      //等待信号量的进程的ID
    struct sem_wait_node* next;
}sem_wait_node_typedef;

//信号量等待列表
typedef struct sem_wait_list{
    sem_wait_node_typedef* first;
    unsigned int cnt;
}sem_wait_list_typedef;

//信号量
typedef struct sem{
    unsigned int id;        //信号量ID，内核通过ID寻找到信号量
    unsigned int count;     //信号量的数量
    sem_wait_list_typedef wait_list;    //等待信号量的进程列表
    
}sem_typedef;

#define SEM_MAX_NUM 10      //信号量最大数量

#define SEM_USED    1
#define SEM_UNUSED  0

//信号量管理结构
typedef struct sem_management{
    sem_typedef sem[SEM_MAX_NUM];
    unsigned char sem_state[SEM_MAX_NUM];
    unsigned int num;
}sem_management_typedef;

extern sem_management_typedef sem_m;        

void sem_init(void);


#endif
