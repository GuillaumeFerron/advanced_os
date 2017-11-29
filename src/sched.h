#ifndef SCHED_H
#define SCHED_H

#include <stdint.h>
#include <inttypes.h>

#define PCB_REGISTERS_LENGTH 13

/*************** type declaration *****************/
struct pcb_s {
	uint32_t regs_process[PCB_REGISTERS_LENGTH];
	uint32_t lr_process;
};

/*************** Functions declaration *****************/
void sched_init();

void sys_yieldto(struct pcb_s* dest);
void do_sys_yieldto();

#endif
