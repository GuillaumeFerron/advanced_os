#ifndef SCHED_H
#define SCHED_H

#include <stdint.h>
#include <inttypes.h>

#define SYSCALL_YIELDTO_NUMBER 5
#define SYSCALL_YIELD_NUMBER 6
#define SYSCALL_EXIT_NUMBER 7
#define PCB_REGISTERS_LENGTH 13
#define STACK_SIZE 1250

/*************** type declaration *****************/
struct pcb_s {
	uint32_t regs_process[PCB_REGISTERS_LENGTH];
	uint32_t lr_user;
	uint32_t lr_svc;
	uint32_t* sp_user;
	struct pcb_s *next;
	struct pcb_s *previous;
};

typedef int (func_t) (void);

/*************** Functions declaration *****************/
void sched_init();
void create_process(func_t entry);

void sys_yieldto(struct pcb_s* dest);
void do_sys_yieldto();

void sys_yield();
void do_sys_yield();
void elect();

void sys_exit(int status);
void do_sys_exit();

#endif
