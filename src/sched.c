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
	
	for(int i = 0; i < PCB_REGISTERS_LENGTH; i++) {	//regs have 4 bytes long cells
		current_process->regs_process[i] = *(regs + i);	
	}

	current_process->lr_process = *(regs + 14);

	current_process = (struct pcb_s*) *(regs + 1);
	
}
