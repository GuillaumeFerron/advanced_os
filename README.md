# README
[[TOC]]

*void dummy(){*

* 	return;*

*}*

*int div(int dividend, int divisor) {*

* int result = 0;*

*  	int remainder = dividend;*

*  	while (remainder >= divisor) {*

*		result++;*

*		remainder -= divisor;*

*  }*

* 	 return result;*

*}*

*int compute_volume(int rad) {*

*  int rad3 = rad * rad * rad;*

*  return div(4*355*rad3, 3*113);*

*}*

## Chapter 3

*int kmain() {*

*int radius = 5;*

*  __asm("cps #19");*

*  __asm("mrs r0, spsr");*

*  int volume;*

*  dummy();*

*  volume = compute_volume(radius);*

*  return volume;*

*}*

## Chapter 4

### 4.1	

void sys_reboot() {

  __asm("mov r0, %0" : : "r"(SYSCALL_NUMBER_REBOOT) : "r0");

 	 __asm("SWI 0");

}

void kmain( void ) {

  __asm("cps 0x10");

  sys_reboot();

}

### 4.2

Never invoked because the init.s doesn’t know that the SWI handler is located in syscall.c

We modify init.s :

*swi_asm_handler:*

*    b swi_handler*

Important to use b and not bl because bl override the content of lr

### 4.3

**syscall.c**

*#include "util.h"*

*#include "syscall.h"*

*void do_sys_reboot() {*

*    __asm("b 0x0000");*

*}*

*void sys_reboot() {*

*  int SYSCALL_REBOOT_NB = 1;*

*  __asm("mov r0, %0" : : "r"(SYSCALL_REBOOT_NB) : "r0");*

*  __asm("SWI 0");*

*}*

*void swi_handler(void) {*

*    int SYSCALL_NUMBER = 0;*

*    __asm("mov %0, r0" : "=r"(SYSCALL_NUMBER));*

*    if(SYSCALL_NUMBER == 1) {*

*   	 do_sys_reboot();    *

*    }*

*    else {*

*   	 PANIC();   	 *

*    }*

*}*

**syscall.h**

*#ifndef SYSCALL_H*

*#define SYSCALL_H*

*int SYSCALL_REBOOT_NB = 1;*

*int SYSCALL_NOP_NUMBER = 2;*

*void do_sys_reboot();*

*void swi_handler(void);*

*void sys_reboot();*

*#endif*

Pour tester : ./run-test.sh ../test/kmain-reboot.c ../test/sys-reboot-does-reboot.gdb

### 4.6

*void sys_nop() {*

*    __asm("mov r0, %0" : : "r"(SYSCALL_NOP_NUMBER) : "r0");*

*      __asm("SWI 0");*

*   return;*

*}*

### 4.7

There’s an issue as the return in sys_nop is done, but the swi_handler isn’t aware of the context of execution, hence doesn’t know what return address to go to

### 4.8

pc is the register containing the following instruction

lr is the register containing the return address of the current function

store:

*__asm("stmfd sp!, {r0-r12, lr}");*

We store the values of r0-r12 because they are used by all the modes, hence their values could be modified.

The sp and lr registers aren’t modified since when we trigger a swi, the execution mode is changed to supervisor, and the registers aren’t the same. Though we need to store the lr value because it contains the return address from syscall_nop.

load:

*__asm("stmfd sp!, {r0-r12, pc}^");*

We load the registers r0-r12 such that their values replace the current ones, and we load the lr content in pc to make him understand that we want to return to the user execution commands.

The *^ *makes the spsr be copied in cpsr to go back to user mode, and to use the user registers (hence to load the registers in the right ones)

SPSR isn’t present in the right side of the instruction because the spsr content has been copied in cpsr, hence changing the execution mode to user, which has no spsr register.

### 4.10

It is not necessary to save the status register since it is popped from spsr into cpsr with the *^*

### 4.12

The sp value should be the same at every iteration, but it is decremented as we can see in the kernel.list "8460:    e24dd00c     sub    sp, sp, #12"

It is done because there exists a prologue (*845c:    e52de004     push    {lr}   	 ; (str lr, [sp, #-4]!)*)  and an epilogue (*84d4:    e49df004     pop    {pc}   	 ; (ldr pc, [sp], #4)*) done by the compiler for every function. In the prologue, the compiler pushes the lr into the stack in order to get it back at the end of the function such that it knows what to do after the execution of the function.

To fix it, we need to specify the attribute naked for the function. Indeed, we don’t want the compiler to do these prologue and epilogue since it is not a basic function, but an interruption, and we handle the lr push and pop in pc ourselves with the lmfd and stmfd.

*void __attribute__((naked)) swi_handler(void)*

### 4.14

Beware of including stdint.h in syscall.c and syscall.h

Also, we need to protect r3 AND r0 when moving date ms into a register, or it could override the value in r0

*void sys_settime(uint64_t date_ms) {*

*    __asm("mov r0, %0" : : "r"(SYSCALL_SETTIME_NUMBER) : "r0");*

*    __asm("mov r3, %0" : : "r"(date_ms) : "r3", "r0");*

*      __asm("SWI 0");*

*}*

### 4.15

*r0         	0x3  	3*

*r1         	0x12345678   	305419896*

*r2         	0xcacacaca   	-892679478*

*r3         	0x3  	3*

*r4         	0x3  	3*

*r5         	0xcacacaca   	-892679478*

*r6         	0x80b4   32948*

*r7         	0x80ac   32940*

*r8         	0x80a8   32936*

*r9         	0x80b0   32944*

*r10        	0x0  	0*

*r11        	0x0  	0*

*r12        	0x0  	0*

*sp         	0x94c4   0x94c4*

*lr         	0x849c   33948*

*pc         	0x8440   0x8440 <do_sys_settime>*

*cpsr       	0x600001d3   	1610613203*

### 4.16

*void do_sys_settime() {*

*    int a = 88;*

*    int b = 66;*

*    int c = 44;    *

*    int d = 22;*

*    a = a + b + c + d;*

*}*

We notice that two registers are used for date_ms because it is a 64 bits long variable, whereas the registers are 32 bits long.

It is done such that the LSB are put in the registers you put it, and the msb are put on top of the stack

### 4.17

To make sure we access the right values of the svc stack, we can back up the sp in a value uint32_t* regs so that it contains all these values. 

*	__asm("mov %0, r0" : "=r"(SYSCALL_NUMBER));*

To access these values, we just need to access *regs for r0 or *(regs+1) for r1… Hence we don’t need the SYSCALL_NUMBER anymore

*void __attribute__((naked)) swi_handler(void) {*

*    __asm("stmfd sp!, {r0-r12, lr}");*

*    __asm("mov %0, sp" : "=r"(regs) : : "sp");*

*    if(*regs == 1) {*

*   	 do_sys_reboot();    *

*    }*

*    else if(*regs == 2) {*

*   	 do_sys_nop();    *

*    }*

*    else if(*regs == 3) {*

*   	 do_sys_settime();*

*    }*

*    else {*

*   	 PANIC();   	 *

*    }*

*    __asm("ldmfd sp!, {r0-r12, pc}^");*

*}*

### 4.18

To keep the lsb and msb separated, we fix the sys_settime :

*void sys_settime(uint64_t date_ms) {*

*    uint32_t date_msb = date_ms >> 32;*

*    uint32_t date_lsb = date_ms;*

*    __asm("mov r0, %0" : : "r"(SYSCALL_SETTIME_NUMBER) : "r0");*

*    __asm("mov r1, %0" : : "r"(date_msb) : "r1", "r0");*

*    __asm("mov r2, %0" : : "r"(date_lsb) : "r2", "r1", "r0");*

*      __asm("SWI 0");*

*}*

We decompose it, and recompose it in the do_sys_settime

*void do_sys_settime() {*

*    uint64_t date_ms = *(regs + 1);*

*    date_ms <<= 32;*

*    date_ms += *(regs + 2);*

*    set_date_ms(date_ms);*

*}*

### 4.20

*uint64_t sys_gettime() {*

*    __asm("mov r0, %0" : : "r"(SYSCALL_GETTIME_NUMBER) : "r0");*

*    __asm("SWI 0");*

*}*

