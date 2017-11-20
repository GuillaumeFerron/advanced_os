#ifndef SCHED_H
#define SCHED_H

#include <stdint.h>
#include <inttypes.h>

/*************** type declaration *****************/
struct pcb_s {
	uint32_t regs[14];
};

/*************** Functions declaration *****************/
void sched_init();

void sys_yieldto(struct pcb_s* dest);
void do_sys_yieldto();

#endif
