#ifndef SCHED_H
#define SCHED_H

#include <stdint.h>
#include <inttypes.h>

#define SYSCALL_YIELDTO_NUMBER 5
#define SYSCALL_YIELD_NUMBER 6
#define PCB_REGISTERS_LENGTH 13
#define STACK_SIZE 1250

/*************** type declaration *****************/
struct pcb_s {
	uint32_t regs_process[PCB_REGISTERS_LENGTH];
	uint32_t lr_user;
	uint32_t lr_svc;
	uint32_t* sp_user;
};

typedef int (func_t) (void);

/*************** Functions declaration *****************/
void sched_init();

void sys_yieldto(struct pcb_s* dest);
void do_sys_yieldto();

struct pcb_s* create_process(func_t entry);

#endif
