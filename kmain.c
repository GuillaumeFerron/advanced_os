#include "./src/syscall.h"
#include <stdint.h>
#include <inttypes.h>
#include "./src/util.h"
#include "sched.h"

//Function declarations
void user_process_1();
void user_process_2();

//PCBs
struct pcb_s *p1, *p2;

void kmain( void ) {
	sched_init();

	p1 = create_process((func_t*) &user_process_1);
	p2 = create_process((func_t*) &user_process_2);

	__asm("cps 0x10"); // switch CPU to USER mode
	// **********************************************************************
	sys_yieldto(p1);

	// this is now unreachable
	PANIC();
}

void user_process_1() {
	int v1=5;
	while(1) {
		v1++;
		sys_yieldto(p2);
	}
}

void user_process_2() {
	int v2=-12;
	while(1) {
		v2-=2;
		sys_yieldto(p1);
	}
}
