#ifndef SYSCALL_H
#define SYSCALL_H
#include <stdint.h>
#include <inttypes.h>

//Back up of registers
uint32_t* regs;

//System call modes
#define SYSCALL_REBOOT_NUMBER 1
#define SYSCALL_NOP_NUMBER 2
#define SYSCALL_SETTIME_NUMBER 3
#define SYSCALL_GETTIME_NUMBER 4

/*************** Functions declaration *****************/

void __attribute__((naked)) swi_handler(void);

void sys_reboot();
void do_sys_reboot();

void sys_nop();
void do_sys_nop();

void sys_settime(uint64_t date_ms);
void do_sys_settime();

uint64_t sys_gettime();
void do_sys_gettime(); 

#endif
