#include "./sched.h"
#include "./syscall.h"
#include "./kheap.h"

//Executed process
struct pcb_s *current_process;
struct pcb_s kmain_process;
struct pcb_s *linked_list;


/*************** INITIALIZATION ***************/

void sched_init() {
	kheap_init();
	
	current_process = &kmain_process;
	linked_list = &kmain_process;
}

void create_process(func_t entry) {

	struct pcb_s* allocated_pcb;

	allocated_pcb = (struct pcb_s*) kAlloc(sizeof(struct pcb_s));
	allocated_pcb->sp_user = (uint32_t*) (kAlloc(STACK_SIZE) + STACK_SIZE); //SP is decreasing, so we need to point at the top of the memory
	allocated_pcb->lr_svc = (uint32_t) entry;
	
	/** Insert new struct in the linked list **/
	allocated_pcb->previous = linked_list;
	linked_list->next = allocated_pcb;
	linked_list = allocated_pcb;
}


/*************** YIELDTO ***************/

void sys_yieldto(struct pcb_s* dest) {
	
	__asm("mov r0, %0" : : "r"(SYSCALL_YIELDTO_NUMBER) : "r0", "r1");

	//Stores the pcb address in r1
	__asm("mov r1, %0" : : "r"(dest) : "r0", "r1");

	//Interruption Call
	__asm("SWI 0");
}

void do_sys_yieldto() {
	
	/*************** Save context *****************/
	/* -- general registers -- */
	for(int i = 0; i < PCB_REGISTERS_LENGTH; i++) {
		current_process->regs_process[i] = *(regs_user + i);	
	}

	//stores the value of the lr which is the svc one, since lr is the same register as in user and svc mode
	/* -- Program Counter (PC), held by lr_svc -- */
	current_process->lr_svc = *(regs_user + 13);
	/* -- lr_user and sp_user are inreachable from SVC mode => switch to SYSTEM mode -- */
	__asm("cps 0x1f"); // switch CPU to SYSTEM mode (system and user modes have the same sp and lr registers)
	__asm("mov %0, sp" : "=r"(current_process->sp_user) : : "sp", "lr");
	__asm("mov %0, lr" : "=r"(current_process->lr_user) : : "sp", "lr");
	__asm("cps 0x13"); // switch CPU to SVC mode

	/*************** Change context *****************/
	current_process = (struct pcb_s*) *(regs_user + 1);

	/*************** Restore other context *****************/
	for(int i = 0; i < PCB_REGISTERS_LENGTH; i++) {
		*(regs_user + i) = current_process->regs_process[i];	
	}

	//Makes sure it stores in regs_user the right lr to continue execution
	*(regs_user + 13) = current_process->lr_svc;

	/* -- lr_user and sp_user are inreachable from SVC mode => switch to SYSTEM mode -- */
	__asm("cps 0x1f"); // switch CPU to SYSTEM mode (system and user modes have the same sp and lr registers)
	__asm("mov sp, %0" : : "r"(current_process->sp_user) : "sp", "lr");
	__asm("mov lr, %0" : : "r"(current_process->lr_user) : "sp", "lr");
	__asm("cps 0x13"); // switch CPU to SVC mode

}


/*************** YIELD ***************/

void sys_yield() {
	__asm("mov r0, %0" : : "r"(SYSCALL_YIELD_NUMBER) : "r0", "r1");

	//Interruption Call
	__asm("SWI 0");
}

void do_sys_yield(){
	/*************** Save context *****************/
	/* -- general registers -- */
	for(int i = 0; i < PCB_REGISTERS_LENGTH; i++) {
		current_process->regs_process[i] = *(regs_user + i);	
	}

	//stores the value of the lr which is the svc one, since lr is the same register as in user and svc mode
	/* -- Program Counter (PC), held by lr_svc -- */
	current_process->lr_svc = *(regs_user + 13);
	/* -- lr_user and sp_user are inreachable from SVC mode => switch to SYSTEM mode -- */
	__asm("cps 0x1f"); // switch CPU to SYSTEM mode (system and user modes have the same sp and lr registers)
	__asm("mov %0, sp" : "=r"(current_process->sp_user) : : "sp", "lr");
	__asm("mov %0, lr" : "=r"(current_process->lr_user) : : "sp", "lr");
	__asm("cps 0x13"); // switch CPU to SVC mode

	/*************** Change context *****************/
	elect();

	/*************** Restore other context *****************/
	for(int i = 0; i < PCB_REGISTERS_LENGTH; i++) {
		*(regs_user + i) = current_process->regs_process[i];	
	}

	//Makes sure it stores in regs_user the right lr to continue execution
	*(regs_user + 13) = current_process->lr_svc;

	/* -- lr_user and sp_user are inreachable from SVC mode => switch to SYSTEM mode -- */
	__asm("cps 0x1f"); // switch CPU to SYSTEM mode (system and user modes have the same sp and lr registers)
	__asm("mov sp, %0" : : "r"(current_process->sp_user) : "sp", "lr");
	__asm("mov lr, %0" : : "r"(current_process->lr_user) : "sp", "lr");
	__asm("cps 0x13"); // switch CPU to SVC mode
}

void elect() {
	if(current_process && current_process->next) {
		current_process = current_process->next;
	}
}