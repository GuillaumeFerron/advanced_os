#include "./sched.h"
#include "./syscall.h"

//System call modes
int SYSCALL_SCHED_NUMBER = 5;

//Executed process
struct pcb_s *current_process;
struct pcb_s kmain_process;


/*************** INITIALIZATION ***************/

void sched_init() {
	current_process = &kmain_process;
}


/*************** SCHED ***************/

void sys_yieldto(struct pcb_s* dest) {
	
	__asm("mov r0, %0" : : "r"(SYSCALL_SCHED_NUMBER) : "r0", "r1");
	__asm("mov r1, %0" : : "r"(dest) : "r0", "r1");

	//Interruption Call
	__asm("SWI 0");
}

void do_sys_yieldto() {
	
	for(int i = 0; i < sizeof((current_process->regs)); i++) {
		(current_process->regs)[i] = *(regs + i);	
	}

	current_process = (struct pcb_s*) *(regs + 1);
	
}
