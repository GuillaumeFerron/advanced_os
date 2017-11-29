#include "./sched.h"
#include "./syscall.h"

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

	//Stores the pcb address in r1
	__asm("mov r1, %0" : : "r"(dest) : "r0", "r1");

	//Interruption Call
	__asm("SWI 0");
}

void do_sys_yieldto() {
	
	//Backs up the current process stack
	for(int i = 0; i < PCB_REGISTERS_LENGTH; i++) {
		current_process->regs_process[i] = *(regs_user + i);	
	}

	//stores the value of the lr which is the svc one, since lr is the same register as in user and svc mode
	current_process->lr_svc = *(regs_user + 14);

	//Gives current process the dest structure
	current_process = (struct pcb_s*) *(regs_user + 1);

	//Context switch
	for(int i = 0; i < PCB_REGISTERS_LENGTH; i++) {
		*(regs_user + i) = current_process->regs_process[i];	
	}

	//Makes sur it stores in regs_user the right lr to go to user_process_X
	*(regs_user + 13) = current_process->lr_user;
	
}
