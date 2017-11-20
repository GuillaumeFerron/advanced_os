#include "./sched.h"

//System call modes
int SYSCALL_SCHED_NUMBER = 5;

//Executed process
struct pcb_s *current_process;

/*************** SCHED ***************/

void sys_yieldto(struct pcb_s* dest) {
	
	__asm("mov r0, %0" : : "r"(SYSCALL_SCHED_NUMBER) : "r0");
	__asm("mov r1, %0" : : "r"(dest) : "r1", "r0");
	
	//Interruption Call
	__asm("SWI 0");
}

void do_sys_yieldto() {
	
}
