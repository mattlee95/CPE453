#ifndef SYNCHRO_H
#define SYNCHRO_H

typedef struct
{
    uint8_t wl[8];
    uint8_t head_ind;
    uint8_t size;
} wait_list_t;

struct mutex_t
{
    uint8_t val;
    wait_list_t wait;
};

struct semaphore_t
{
    uint16_t val;
    wait_list_t wait;
};


void wait_list_set(wait_list_t* wlist, uint8_t add);
uint8_t wait_list_get(wait_list_t* wlist);

void mutex_init(struct mutex_t* m);
void mutex_lock(struct mutex_t* m);
void mutex_unlock(struct mutex_t* m);

void sem_init(struct semaphore_t* s, int16_t value);
void sem_wait(struct semaphore_t* s);
void sem_signal(struct semaphore_t* s);
void sem_signal_swap(struct semaphore_t* s);

#endif
