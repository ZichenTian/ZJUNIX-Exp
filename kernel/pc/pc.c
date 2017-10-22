#include "pc.h"

#include <driver/vga.h>
#include <intr.h>
#include <zjunix/syscall.h>
#include <zjunix/utils.h>

task_struct pcb[8];
int curr_proc;

static void copy_context(context* src, context* dest) {
    dest->epc = src->epc;
    dest->at = src->at;
    dest->v0 = src->v0;
    dest->v1 = src->v1;
    dest->a0 = src->a0;
    dest->a1 = src->a1;
    dest->a2 = src->a2;
    dest->a3 = src->a3;
    dest->t0 = src->t0;
    dest->t1 = src->t1;
    dest->t2 = src->t2;
    dest->t3 = src->t3;
    dest->t4 = src->t4;
    dest->t5 = src->t5;
    dest->t6 = src->t6;
    dest->t7 = src->t7;
    dest->s0 = src->s0;
    dest->s1 = src->s1;
    dest->s2 = src->s2;
    dest->s3 = src->s3;
    dest->s4 = src->s4;
    dest->s5 = src->s5;
    dest->s6 = src->s6;
    dest->s7 = src->s7;
    dest->t8 = src->t8;
    dest->t9 = src->t9;
    dest->hi = src->hi;
    dest->lo = src->lo;
    dest->gp = src->gp;
    dest->sp = src->sp;
    dest->fp = src->fp;
    dest->ra = src->ra;
}

void init_pc() {
    int i;
    for (i = 1; i < 8; i++)
        pcb[i].ASID = -1;
    pcb[0].ASID = 0;
    pcb[0].counter = PROC_DEFAULT_TIMESLOTS;
    kernel_strcpy(pcb[0].name, "init");
    curr_proc = 0;
    register_syscall(10, pc_kill_syscall);
    register_interrupt_handler(7, pc_schedule);

    asm volatile(
        "li $v0, 1000000\n\t"
        "mtc0 $v0, $11\n\t"
        "mtc0 $zero, $9");
}

void pc_schedule(unsigned int status, unsigned int cause, context* pt_context) {
    // Save context
    copy_context(pt_context, &(pcb[curr_proc].context));
    int i;
    int last_proc = curr_proc;
    for (i = 0; i < 8; i++) {
        curr_proc = (curr_proc + 1) & 7;
        if (pcb[curr_proc].ASID >= 0 && pcb[curr_proc].state == TASK_READY)
        {
            if(curr_proc == last_proc)
            {
                break;
            }
            pcb[curr_proc].state = TASK_RUNNING;
            if(pcb[last_proc].state == TASK_RUNNING)    //只有正在运行的才会返回就绪列表
                pcb[last_proc].state = TASK_READY;      //如果进程在运行状态下被阻塞，则维持阻塞状态
            break;
        }
    }
    if (i == 8) {
        kernel_puts("Error: PCB[0] is invalid!\n", 0xfff, 0);   //问一下助教，这个0号任务到底是干嘛的
        while (1)
            ;
    }
    // Load context
    copy_context(&(pcb[curr_proc].context), pt_context);
    asm volatile("mtc0 $zero, $9\n\t");
}

int pc_peek() {
    int i = 0;
    for (i = 0; i < 8; i++)
        if (pcb[i].ASID < 0)
            break;
    if (i == 8)
        return -1;
    return i;
}

void pc_create(int asid, void (*func)(), unsigned int init_sp, unsigned int init_gp, char* name) {
    pcb[asid].context.epc = (unsigned int)func;
    pcb[asid].context.sp = init_sp;
    pcb[asid].context.gp = init_gp;
    kernel_strcpy(pcb[asid].name, name);
    pcb[asid].ASID = asid;
    pcb[asid].state = TASK_READY;
}

void pc_kill_syscall(unsigned int status, unsigned int cause, context* pt_context) {
    if (curr_proc != 0) {
        pcb[curr_proc].ASID = -1;
        pcb[curr_proc].state = TASK_KILLED;
        pc_schedule(status, cause, pt_context);
    }
}

int pc_kill(int proc) {
    proc &= 7;
    if (proc != 0 && pcb[proc].ASID >= 0) {
        pcb[proc].ASID = -1;
        pcb[proc].state = TASK_KILLED;
        return 0;
    } else if (proc == 0)
        return 1;
    else
        return 2;
}

task_struct* get_curr_pcb() {
    return &pcb[curr_proc];
}

const char State_String[6][15] = {
    "ready",
    "running",
    "blocked",
    "suspend",
    "killed",
    "not created"
};

int print_proc() {
    int i;
    kernel_puts("PID name\n", 0xfff, 0);
    for (i = 0; i < 8; i++) {
        if (pcb[i].ASID >= 0)
        {
            kernel_printf(" %x  %s  %s\n", pcb[i].ASID, pcb[i].name, State_String[pcb[i].state]);
        }
    }
    return 0;
}

//added by ZichenTian
void wakeup_proc(int proc)
{
    pcb[proc].state = TASK_READY;
}

//added by ZichenTian
void block_proc(int proc)
{
    pcb[proc].state = TASK_BLOCKED;
}

//added by ZichenTian
task_state get_proc_state(int proc)
{
    return pcb[proc].state;
}
