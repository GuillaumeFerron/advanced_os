#include "util.h"
#include "syscall.h"
#include <stdint.h>
#include <inttypes.h>
#include "./hw.h"
#include "./sched.h"

//System call modes
int SYSCALL_REBOOT_NUMBER = 1;
int SYSCALL_NOP_NUMBER = 2;
int SYSCALL_SETTIME_NUMBER = 3;
int SYSCALL_GETTIME_NUMBER = 4;


/*************** REBOOT ***************/

void sys_reboot() {
	//Interruption Call
	__asm("mov r0, %0" : : "r"(SYSCALL_REBOOT_NUMBER) : "r0");
	__asm("SWI 0");
}

void do_sys_reboot() {
	//Jumps to the address 0x0000
	__asm("b 0x0000");
}


/*************** NOP ***************/
void sys_nop() {
	//Interruption Call
	__asm("mov r0, %0" : : "r"(SYSCALL_NOP_NUMBER) : "r0");
  	__asm("SWI 0");
	return;
}

void do_sys_nop() {
	//Does nothing
	return;
}


/*************** SET TIME ***************/
void sys_settime(uint64_t date_ms) {
	//64 bit variable splicing
	uint32_t date_msb = date_ms >> 32;
	uint32_t date_lsb = date_ms;
	
	//Backing up each two parts of the 64 bit long in the registers. We chose r1 and r2 
	__asm("mov r0, %0" : : "r"(SYSCALL_SETTIME_NUMBER) : "r0");
	__asm("mov r1, %0" : : "r"(date_msb) : "r1", "r0");
	__asm("mov r2, %0" : : "r"(date_lsb) : "r2", "r1", "r0");

	//Interruption Call
  	__asm("SWI 0");
}

void do_sys_settime() {
	//Gets the MSB of the date
	uint64_t date_ms = *(regs + 1);
	
	//Put the bits as MSB of date_ms
	date_ms <<= 32;
	
	//Add the LSB
	date_ms += *(regs + 2);

	set_date_ms(date_ms);
}


/*************** GET TIME ***************/
uint64_t sys_gettime() {
	uint64_t date_ms;
	uint32_t date_msb, date_lsb;	

	// Interruption call
	__asm("mov r0, %0" : : "r"(SYSCALL_GETTIME_NUMBER) : "r0");
	__asm("SWI 0");
	
	//Get the value of time in the registers r0 and r1
	__asm("mov %0, r0" : "=r"(date_msb) : : "r0", "r1");
	__asm("mov %0, r1" : "=r"(date_lsb) : : "r0", "r1");

	//Recompose the full date
	date_ms = date_msb;
	date_ms <<= 32;
	date_ms += date_lsb;

	return date_ms;
}

void do_sys_gettime() {
	//Get the value of the time
	uint64_t date_ms = get_date_ms();

	//64 bit variable splicing
	uint32_t date_msb = date_ms >> 32;
	uint32_t date_lsb = date_ms;
	
	//Backing up each two parts of the 64 bit long in the registers. We chose r0 and r1 of the USER stack
	*(regs) = date_msb;
	*(regs+1) = date_lsb;
}


/*************** INTERRUPTION HANDLER ***************/
void __attribute__((naked)) swi_handler(void) {
	//Stores the User registers
	__asm("stmfd sp!, {r0-r12, lr}");

	//Backs up the registers to access the values later on
	__asm("mov %0, sp" : "=r"(regs) : : "sp");

	//Handling of syscall mode
	if(*regs == 1) {
		do_sys_reboot();	
	}
	else if(*regs == 2) {
		do_sys_nop();	
	}
	else if(*regs == 3) {
		do_sys_settime();
	}
	else if(*regs == 4) {
		do_sys_gettime();
	}
	else if(*regs == 5) {
		do_sys_yieldto();	
	}
	else {
		PANIC();		
	}

	//Gets the user registers back to the stack and load lr into pc
	__asm("ldmfd sp!, {r0-r12, pc}^");
}
