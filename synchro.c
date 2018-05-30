#include "globals.h"
#include "os.h"
#include "synchro.h"

#include <avr/interrupt.h>

extern system_t sys;


void wait_list_set(wait_list_t* wlist, uint8_t add)
{
    wlist->wl[(wlist->head_ind + wlist->size) % 8] = add;
    wlist->size++;
}


uint8_t wait_list_get(wait_list_t* wlist)
{
    uint8_t ret = wlist->wl[wlist->head_ind];
    wlist->head_ind = (wlist->head_ind + 1) % 8;
    wlist->size--;
    return ret;
}


void mutex_init(struct mutex_t* m)
{
    cli();
    m->val = 1;
    m->wait.head_ind = 0;
    m->wait.size = 0;
    sei();
}

void mutex_lock(struct mutex_t* m)
{
    cli();
    /*uint8_t ticket;

    if (m->val != 10) //locked
    {
        sys.thread_buff[sys.curr_thread_id].status = THREAD_WAITING;
        wait_list_set(&(m->wait),sys.curr_thread_id);
        //context switch
        non_interrupted_context();
    }
    //if unlocked
    m->val = sys.curr_thread_id;*/
    if (m->val <= 0)
    {
        sys.thread_buff[sys.curr_thread_id].status = THREAD_WAITING;
        wait_list_set(&(m->wait),sys.curr_thread_id);
        //context switch
        non_interrupted_context();
    }
    m->val = m->val - 1;
    sei();
}

void mutex_unlock(struct mutex_t* m)
{
    uint8_t thread_id;
    cli();
    //check if correct caller
    /*if (m->wait.size == 0)
    {
        //no waitlist
        //context switch
        m->val = 10;
        non_interrupted_context();
    }
    thread_id = wait_list_get(&(m->wait));
    m->val = thread_id;
    sys.thread_buff[thread_id].status = THREAD_READY;
    //context switch to thread_id 
    force_context_switch(thread_id);*/

    m->val = m->val + 1;
    if (m->wait.size > 0)
    {   
        thread_id = wait_list_get(&(m->wait));
        sys.thread_buff[thread_id].status = THREAD_READY;
        //switch to thread_id
        force_context_switch(thread_id);
    }
   
    sei();
}

void sem_init(struct semaphore_t* s, int8_t value)
{
    cli();
    s->val = value;
    s->wait.head_ind = 0;
    s->wait.size = 0;
    sei();
}

void sem_wait(struct semaphore_t* s)
{
    cli();
    if (s->val <= 0)
    {
        sys.thread_buff[sys.curr_thread_id].status = THREAD_WAITING;
        wait_list_set(&(s->wait),sys.curr_thread_id);
        //context switch
        non_interrupted_context();
    }
    s->val = s->val - 1;
    sei();
}

void sem_signal(struct semaphore_t* s)
{
    uint8_t thread_id;
    cli();
    s->val = s->val + 1;
    if (s->wait.size > 0)
    {
        thread_id = wait_list_get(&(s->wait));
        sys.thread_buff[thread_id].status = THREAD_READY;
        //switch to thread_id
        force_context_switch(thread_id);
    }   
    sei();
}

void sem_signal_swap(struct semaphore_t* s)
{
    uint8_t thread_id;
    cli();
    s->val = s->val + 1;
    if (s->wait.size > 0)
    {
        thread_id = wait_list_get(&(s->wait));
        wait_list_set(&(s->wait), sys.curr_thread_id);
        sys.thread_buff[thread_id].status = THREAD_READY;
        sys.thread_buff[sys.curr_thread_id].status = THREAD_WAITING;
        //Switch to determined thread
        force_context_switch(thread_id);
    }
    sei();
}






