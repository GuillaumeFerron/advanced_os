# README

## Table of Contents

## Chapter 2

### 2.3

Dividend : r0

Divisor : r1

Result : r3

Remainder : r3

### 2.4

Before kmain returns : sp ← 0x95f8

*mov r3, #5*		→ stocke 5 in r3

*str r3, [sp, #4]		*→ stocke r3 au 4e octet à partir de sp

sp cell values : 	sp[0] : 0x20b

			sp[1] : 0x5

			sp[2] : 0x0

			sp[3] : 0x5098

in hexa *print/x *((int *) $sp)*					in decimal *print/d *((int *) $sp)*

radius and volume are stored in r3 (0x95e0)

Increasing address : at the beginning only 0x9500 used, at the end 0x95e0, 0x95f0 and 0x9600 used

sp points at the last full place

bl is a function call, and after the branch (b), the return address is pushed in lr

### 2.8

With b instruction, the return address isn’t loaded in lr, then pc isn’t loaded with the next instruction, hence it doesn’t know what to execute next.

Fix : bl instead of b

### 2.9

__asm("mov r2, %0" : : “r”(radius));

__asm("mov %0, r3" : : “=r”(radius));

### 2.10

The register lr isn’t allocated to the dummy method

90c4 : e12ffff1e 	bx	lr	is not done when attribute naked is specified as it doesn’t do a prologue or epilogue in the compiler.

### 2.12

In the tests, the prints are checked such that are indeed &xxxxx

### Code :

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

*int kmain() {*

*int radius = 5;*

*  __asm("cps #19");*

*  __asm("mrs r0, spsr");*

*  int volume;*

*  dummy();*

*  volume = compute_volume(radius);*

*  return volume;*

*}*

## Chapter 3

### 3.1

At the beginning of kmain, print/t $cpsr = 11111	→ system

### 3.2

__asm("cps #19")	$cpsr → [...]10011 and $sp → change from 0x95fc to 0x940c

### 3.3

$lr = 32920 before, 0 after

At first lr points to return value of kmain then points to start

### 3.4

User mode → SVC not allowed, cpsr stays at user mode

### 3.5

__asm("msr r0, spsr"); if system mode, null since no spsr system.

### 3.6

Prints the execution mode

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

## Chapter 5

### 5.1

In sched.c

*#include "./sched.h"*

*//System call modes*

*int SYSCALL_SCHED_NUMBER = 5;*

*/*************** SCHED ***************/*

*void sys_yieldto(struct pcb_s* dest) {*

*}*

*void do_sys_yieldto() {*

*}*

In sched.h : 

*/*************** type declaration *****************/*

*struct pcb_s {*

*};*

### 5.2

*/*************** INITIALIZATION ***************/*

*void sched_init() {*

*    current_process = &kmain_process;*

*}*

*/*************** SCHED ***************/*

*void sys_yieldto(struct pcb_s* dest) {*

*    *

*    __asm("mov r0, %0" : : "r"(SYSCALL_SCHED_NUMBER) : "r0", "r1");*

*    //Stores the pcb address in r1*

*    __asm("mov r1, %0" : : "r"(dest) : "r0", "r1");*

*    //Interruption Call*

*    __asm("SWI 0");*

*}*

*void do_sys_yieldto() {*

*    *

*    //Backs up the current process stack*

*    for(int i = 0; i < PCB_REGISTERS_LENGTH; i++) {*

*   	 current_process->regs_process[i] = *(regs_user + i);    *

*    }*

*    //stores the value of the lr which is the svc one, since lr is the same register as in user and svc mode*

*    current_process->lr_svc = *(regs_user + 14);*

*    //Gives current process the dest structure*

*    current_process = (struct pcb_s*) *(regs_user + 1);*

*    //Context switch*

*    for(int i = 0; i < PCB_REGISTERS_LENGTH; i++) {*

*   	 *(regs_user + i) = current_process->regs_process[i];    *

*    }*

*    //Makes sur it stores in regs_user the right lr to go to user_process_X*

*    *(regs_user + 13) = current_process->lr_user;*

*    *

*}*

### 5.3

*void sched_init() {*

*    current_process = &kmain_process;*

*}*

### 5.4

*// initialize p1 and p2*

*p1->lr_user = (uint32_t) &user_process_1;*

*p2->lr_user = (uint32_t) &user_process_2;*

### 5.6

*void do_sys_yieldto() {*

*    *

*    /*************** Save context *****************/*

*    /* -- general registers -- */*

*    for(int i = 0; i < PCB_REGISTERS_LENGTH; i++) {*

*   	 current_process->regs_process[i] = *(regs_user + i);    *

*    }*

*    //stores the value of the lr which is the svc one, since lr is the same register as in user and svc mode*

*    /* -- Program Counter (PC), held by lr_svc -- */*

*    current_process->lr_svc = *(regs_user + 13);*

*    /* -- lr_user and sp_user are inreachable from SVC mode => switch to SYSTEM mode -- */*

*    __asm("cps 0x1f"); // switch CPU to SYSTEM mode (system and user modes have the same sp and lr registers)*

*    __asm("mov %0, sp" : "=r"(current_process->sp_user) : : "sp", "lr");*

*    __asm("mov %0, lr" : "=r"(current_process->lr_user) : : "sp", "lr");*

*    __asm("cps 0x13"); // switch CPU to SVC mode*

*    /*************** Change context *****************/*

*    current_process = (struct pcb_s*) *(regs_user + 1);*

*    /*************** Restore other context *****************/*

*    for(int i = 0; i < PCB_REGISTERS_LENGTH; i++) {*

*   	 *(regs_user + i) = current_process->regs_process[i];    *

*    }*

*    //Makes sure it stores in regs_user the right lr to continue execution*

*    *(regs_user + 13) = current_process->lr_svc;*

*    /* -- lr_user and sp_user are inreachable from SVC mode => switch to SYSTEM mode -- */*

*    __asm("cps 0x1f"); // switch CPU to SYSTEM mode (system and user modes have the same sp and lr registers)*

*    __asm("mov sp, %0" : : "r"(current_process->sp_user) : "sp", "lr");*

*    __asm("mov lr, %0" : : "r"(current_process->lr_user) : "sp", "lr");*

*    __asm("cps 0x13"); // switch CPU to SVC mode*

*}*

### 5.8

It stores v1 and v2 in the user execution stack in r3, hence the values can’t be properly updated as there are concurrent access to the registers it is stored in.

### 5.9

*struct pcb_s {*

*    uint32_t regs_process[PCB_REGISTERS_LENGTH];*

*    uint32_t lr_user;*

*    uint32_t lr_svc;*

*    uint32_t* sp_user;*

*};*

*struct pcb_s* create_process(func_t entry) {*

*    struct pcb_s* allocated_pointer;*

*    allocated_pointer = (struct pcb_s*) kAlloc(sizeof(struct pcb_s));*

*    allocated_pointer->sp_user = (uint32_t*) kAlloc(STACK_SIZE);*

*    allocated_pointer->lr_svc = (uint32_t) entry;*

*    *

*    return allocated_pointer;*

*}*

### 5.10

*void sched_init() {*

*    kheap_init();*

*    current_process = &kmain_process;*

*}*

### 5.11

To the return value of the kalloc : *allocated_pointer->sp_user = (uint32_t) kAlloc(STACK_SIZE);*

### 5.12

store : *current_process->sp_user = regs_user;*

load : *regs_user = current_process->sp_user;*

### 5.15

It is not allowed to avoid blocking by process, in case a process chooses to loop

To let the processor handles priorities itself, and optimize the executions

## Chapter 6

### 6.1

*struct pcb_s {*

*    uint32_t regs_process[PCB_REGISTERS_LENGTH];*

*    uint32_t lr_user;*

*    uint32_t lr_svc;*

*    uint32_t* sp_user;*

*    struct pcb_s* next;*

*    struct pcb_s* previous;*

*};*

We also add a pointer to the last struct pcb_s added in the linked list

### 6.2

**Modify create_process**

*void create_process(func_t entry) {*

*    struct pcb_s* allocated_pcb;*

*    allocated_pcb = (struct pcb_s*) kAlloc(sizeof(struct pcb_s));*

*    allocated_pcb->sp_user = (uint32_t*) (kAlloc(STACK_SIZE) + STACK_SIZE); //SP is decreasing, so we need to point at the top of the memory*

*    allocated_pcb->lr_svc = (uint32_t) entry;*

*    *

*    /** Insert new struct in the linked list **/*

*    allocated_pcb->previous = linked_list;*

*    linked_list->next = allocated_pcb;*

*    linked_list = allocated_pcb;*

*}*

**Implement elect**

*void elect() {*

*    if(current_process && current_process->next) {*

*   	 current_process = current_process->next;*

*    }*

*    else {*

*   	 /** Get the last added process points to the first added one **/*

*   	 current_process->next = kmain_process.next;*

*   	 elect();*

*    }*

*}*

What’s important is that the last process doesn’t have any next process, hence issue when running the programm. What is odne here is that if the process has no next process, then the next one is the one following the kmain_process.

**System call sys_yield**

*/*************** YIELD ***************/*

*void sys_yield() {*

*    __asm("mov r0, %0" : : "r"(SYSCALL_YIELD_NUMBER) : "r0", "r1");*

*    //Interruption Call*

*    __asm("SWI 0");*

*}*

*void do_sys_yield(){*

*    /*************** Save context *****************/*

*    /* -- general registers -- */*

*    for(int i = 0; i < PCB_REGISTERS_LENGTH; i++) {*

*   	 current_process->regs_process[i] = *(regs_user + i);    *

*    }*

*    //stores the value of the lr which is the svc one, since lr is the same register as in user and svc mode*

*    /* -- Program Counter (PC), held by lr_svc -- */*

*    current_process->lr_svc = *(regs_user + 13);*

*    /* -- lr_user and sp_user are inreachable from SVC mode => switch to SYSTEM mode -- */*

*    __asm("cps 0x1f"); // switch CPU to SYSTEM mode (system and user modes have the same sp and lr registers)*

*    __asm("mov %0, sp" : "=r"(current_process->sp_user) : : "sp", "lr");*

*    __asm("mov %0, lr" : "=r"(current_process->lr_user) : : "sp", "lr");*

*    __asm("cps 0x13"); // switch CPU to SVC mode*

*    /*************** Change context *****************/*

*    elect();*

*    /*************** Restore other context *****************/*

*    for(int i = 0; i < PCB_REGISTERS_LENGTH; i++) {*

*   	 *(regs_user + i) = current_process->regs_process[i];    *

*    }*

*    //Makes sure it stores in regs_user the right lr to continue execution*

*    *(regs_user + 13) = current_process->lr_svc;*

*    /* -- lr_user and sp_user are inreachable from SVC mode => switch to SYSTEM mode -- */*

*    __asm("cps 0x1f"); // switch CPU to SYSTEM mode (system and user modes have the same sp and lr registers)*

*    __asm("mov sp, %0" : : "r"(current_process->sp_user) : "sp", "lr");*

*    __asm("mov lr, %0" : : "r"(current_process->lr_user) : "sp", "lr");*

*    __asm("cps 0x13"); // switch CPU to SVC mode*

*}*

The important parts are that the sys_yeild function doesn’t load a destination process in a register anymore, since the current process will know what process to go to. 

Furthermore, the code is pretty much the same as for the yeild to, but the destination process is now specified by the elect function

**Chain kmain_process**

*void sched_init() {*

*    kheap_init();*

*    *

*    current_process = &kmain_process;*

*    linked_list = &kmain_process;*

*}*

In order to have the first process added right in the linked list, we need to initialise the linked list to kmain_process

### 6.5

We can use the *kFree* method from kheap.c, using the pointer to the memory slot to free, and the size to free. To remove the process from the linked list, we can remap it such that :

If we have the processes in the list st A → B → C, and we want to remove B, we do :

	A.next = C;

	C.previous = A;

	Free B.sp_user memory slots;

	Free B memory slots;

	Change context to C;

By nature, this is pretty effective since the number of processes doesn’t matter in the linked list, the number of operations will be the same for 3 or 10000 processes in the list.

### 6.6

We choose to implement the first one :

**Mark the process as TERMINATED and handle its termination in function elect(). In this case, you have**

**to add the notion of state in the PCBs, and modify elect() in order to remove a potential TERMINATED**

**process met when walking through the linked list;**

*int sys_exit(int status) {*

*    __asm("mov r0, %0" : : "r"(SYSCALL_EXIT_NUMBER) : "r0", "r1");*

*    //Stores the status in r1*

*    __asm("mov r1, %0" : : "r"(status) : "r0", "r1");*

*    //Interruption Call*

*    __asm("SWI 0");*

*    return status;*

*}*

*void do_sys_exit() {*

*    current_process->state = TERMINATED;*

*    __asm("mov %0, r1" : "=r"(current_process->state) : : "r1");*

*}*

