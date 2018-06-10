#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include "globals.h"
#include "os.h"
#include "synchro.h"

#define MAX_THREADS 8

system_t sys;

//This interrupt routine is automatically run every 10 milliseconds
ISR(TIMER0_COMPA_vect) {
   //At the beginning of this ISR, the registers r0, r1, and r18-31 have 
   //already been pushed to the stack

   //The following statement tells GCC that it can use registers r18-r31 
   //for this interrupt routine.  These registers (along with r0 and r1) 
   //will automatically be pushed and popped by this interrupt routine.
   asm volatile ("" : : : "r18", "r19", "r20", "r21", "r22", "r23", "r24", \
                 "r25", "r26", "r27", "r30", "r31");                        

   //Insert your code here
   //Call get_next_thread to get the thread id of the next thread to run
   //Call context switch here to switch to that next thread

    volatile uint8_t new_thread, old_thread;

    remove_sleep();

    old_thread = sys.curr_thread_id;
    new_thread = get_next_thread();

    //sys.interrupts = sys.interrupts + 1;
    sys.interrupts++;

    sys.curr_thread_id = new_thread;
    sys.thread_buff[new_thread].sched++;

    sys.thread_buff[new_thread].status = THREAD_RUNNING;
    sys.thread_buff[old_thread].status = THREAD_READY;

    context_switch(&sys.thread_buff[new_thread].stack_pointer,
                   &sys.thread_buff[old_thread].stack_pointer);
   //At the end of this ISR, GCC generated code will pop r18-r31, r1, 
   //and r0 before exiting the ISR
}

ISR(TIMER1_COMPA_vect)
{
    //called every second
    uint8_t n;
    for (n = 0; n < sys.num_threads; n++)
    {
        sys.thread_buff[n].last_sched = sys.thread_buff[n].sched;
        sys.thread_buff[n].sched = 0;
    }
    sys.sys_time = sys.sys_time + 100;
    sys.last_interrupts = sys.interrupts;
    sys.interrupts = 0;
}

//Call this to start the system timer interrupt
/*void start_system_timer() {
   TIMSK0 |= _BV(OCIE0A);  //interrupt on compare match
   TCCR0A |= _BV(WGM01);   //clear timer on compare match

   //Generate timer interrupt every ~10 milliseconds
   TCCR0B |= _BV(CS02) | _BV(CS00) | _BV(CS02);    //prescalar /1024
   OCR0A = 156;             //generate interrupt every 9.98 milliseconds

    //start timer 1 to generate interrupt every 1 second
    OCR1A = 15625;
    TIMSK1 |= _BV(OCIE1A);  //interrupt on compare
    TCCR1B |= _BV(WGM12) | _BV(CS12) | _BV(CS10); //slowest prescalar /1024
}*/

__attribute__((naked)) void context_switch(uint16_t* new_sp, uint16_t* old_sp)
{
    asm volatile("push r2\n");
    asm volatile("push r3\n");
    asm volatile("push r4\n");
    asm volatile("push r5\n");
    asm volatile("push r6\n");
    asm volatile("push r7\n");
    asm volatile("push r8\n");
    asm volatile("push r9\n");
    asm volatile("push r10\n");
    asm volatile("push r11\n");
    asm volatile("push r12\n");
    asm volatile("push r13\n");
    asm volatile("push r14\n");
    asm volatile("push r15\n");
    asm volatile("push r16\n");
    asm volatile("push r17\n");

    asm volatile("push r28\n");
    asm volatile("push r29\n"); 

    //asm volatile("in r16, __SP_L__\n");
    //asm volatile("in r17, __SP_H__\n");

    asm volatile("clr r31\n");
    asm volatile("ldi r30, 0x5D\n");
    asm volatile("ld r16, Z\n");

    asm volatile("clr r31\n");
    asm volatile("ldi r30, 0x5E\n");
    asm volatile("ld r17, Z\n");
    
    //old tp into Z
    asm volatile("mov r30, r22\n");
    asm volatile("mov r31, r23\n");

    asm volatile("st z+, r16\n");
    asm volatile("st z, r17\n");

    //new to into Z
    asm volatile("mov r30, r24\n");
    asm volatile("mov r31, r25\n");

    asm volatile("ld r16, z+\n");
    asm volatile("ld r17, z\n");

    //asm volatile("out __SP_L__, r16\n");
    //asm volatile("out __SP_H__, r17\n");

    asm volatile("clr r31\n");
    asm volatile("ldi r30, 0x5D\n");
    asm volatile("st Z, r16\n");

    asm volatile("clr r31\n");
    asm volatile("ldi r30, 0x5E\n");
    asm volatile("st Z, r17\n");
    
    asm volatile("pop r29\n");
    asm volatile("pop r28\n");

    asm volatile("pop r17\n");
    asm volatile("pop r16\n");
    asm volatile("pop r15\n");
    asm volatile("pop r14\n");
    asm volatile("pop r13\n");
    asm volatile("pop r12\n");
    asm volatile("pop r11\n");
    asm volatile("pop r10\n");
    asm volatile("pop r9\n");
    asm volatile("pop r8\n");
    asm volatile("pop r7\n");
    asm volatile("pop r6\n");
    asm volatile("pop r5\n");
    asm volatile("pop r4\n");
    asm volatile("pop r3\n");
    asm volatile("pop r2\n");
    
    asm volatile("ret\n");
}

__attribute__((naked)) void thread_start(void)
{
    sei(); //enable interrupts - leave as the first statement in thread_start()
    
    asm volatile("mov r24, r2\n");
    asm volatile("mov r25, r3\n");

    asm volatile("mov r30, r4\n");
    asm volatile("mov r31, r5\n");

    asm volatile("ijmp\n");    
}


void os_init(void)
{
    cli();
    sys.curr_thread_id = 0;
    sys.num_threads = 0;
    sys.interrupts = 0;
    sys.last_interrupts = 0;
    sys.sys_time = 0;
}

void create_thread(char* name, uint16_t address, void* args, uint16_t stack_size)
{
    //room for new thread
    if (sys.num_threads == MAX_THREADS)
    {
        return;
    }

    //save data to thread index
    regs_context_switch *context_registers;    
    uint8_t thread_id = sys.num_threads;
    sys.num_threads = sys.num_threads + 1;
    //thread_id = index
    sys.thread_buff[thread_id].thread_id = thread_id;
    sys.thread_buff[thread_id].function_ptr = address;

    sys.thread_buff[thread_id].status = THREAD_READY;
    sys.thread_buff[thread_id].ticks = 0;
    sys.thread_buff[thread_id].sched = 0;
    sys.thread_buff[thread_id].last_sched = 0;

    sys.thread_buff[thread_id].stack_size = stack_size +
                                            sizeof(regs_interrupt) +
                                            sizeof(regs_context_switch);

    strcpy(&(sys.thread_buff[thread_id].thread_name[0]), name);

    sys.thread_buff[thread_id].stack_low = calloc(1,
                                  sys.thread_buff[thread_id].stack_size);
    sys.thread_buff[thread_id].stack_high = (void *)sys.thread_buff[thread_id].stack_low +
                                            sys.thread_buff[thread_id].stack_size;
    
    context_registers = (regs_context_switch *)
        ((void *)(sys.thread_buff[thread_id].stack_high -
        sizeof(regs_context_switch)));

    context_registers->pcl = (uint8_t) (((uint16_t)thread_start & 0x00FF));
    context_registers->pch = (uint8_t) (((uint16_t)thread_start >> 8) & 0x00FF);

    context_registers->r2 = (uint8_t) (((uint16_t)args & 0x00FF));
    context_registers->r3 = (uint8_t) (((uint16_t)args >> 8) & 0x00FF);

    context_registers->r4 = (uint8_t) (((uint16_t)address & 0x00FF));
    context_registers->r5 = (uint8_t) (((uint16_t)address >> 8) & 0x00FF);

    sys.thread_buff[sys.num_threads-1].stack_pointer = (uint16_t)context_registers;
}

void os_start(void)
{
    uint16_t starting_ptr;
    start_system_timer();
    //sei();    
    context_switch(&sys.thread_buff[0].stack_pointer, &starting_ptr);
}

uint8_t get_next_thread(void)
{
    uint8_t next_thread = ((sys.curr_thread_id + 1) % (sys.num_threads));
    uint8_t n;

    while (sys.thread_buff[next_thread].status != THREAD_READY)
    {
        next_thread = (next_thread + 1) % (sys.num_threads);
    }
    if (next_thread == 0)
    {
        for (n = 1; n < sys.num_threads; n++)
        {
            if (sys.thread_buff[n].status == THREAD_READY)
            {
                return n;
            }
        }
        return next_thread;
    }
    return next_thread;
}

void force_context_switch(uint8_t thread_id)
{
    uint8_t holder;
    cli();
    //sys.thread_buff[thread_id].status = THREAD_RUNNING;
    sys.thread_buff[thread_id].sched++;
    if (sys.thread_buff[sys.curr_thread_id].status == THREAD_RUNNING)
    {
        sys.thread_buff[sys.curr_thread_id].status = THREAD_READY;
    }
    holder = sys.curr_thread_id;
    sys.curr_thread_id = thread_id;
    sys.thread_buff[thread_id].status = THREAD_RUNNING;

    context_switch(&sys.thread_buff[thread_id].stack_pointer,
                   &sys.thread_buff[holder].stack_pointer);

    sei();
}

void non_interrupted_context()
{
    cli();
    uint8_t next_thread = get_next_thread();
    force_context_switch(next_thread);
    sei();
}

void yield()
{
    cli();
    non_interrupted_context();
    sei();
}

uint8_t get_thread_id()
{
    return sys.curr_thread_id;
}

void thread_sleep(uint16_t ticks)
{
    cli();
    sys.thread_buff[sys.curr_thread_id].ticks = ticks;
    sys.thread_buff[sys.curr_thread_id].status = THREAD_SLEEPING;
    non_interrupted_context();
    sei();
}

void remove_sleep()
{
    uint8_t n;
    cli();
    for (n = 0; n < sys.num_threads; n++)
    {
        if (sys.thread_buff[n].ticks > 0)
        {
            sys.thread_buff[n].ticks = sys.thread_buff[n].ticks - 1;
            if (sys.thread_buff[n].ticks == 0)
            {
                sys.thread_buff[n].status = THREAD_READY;
            }
        }
    }
    sei();
}
