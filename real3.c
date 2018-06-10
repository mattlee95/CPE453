#include <stdio.h>
#include <stdlib.h>
#include <avr/io.h>
#include <util/delay.h>
#include "os.h"
#include "serial.h"
#include "synchro.h"

extern system_t sys;

struct mutex_t mutex;
struct semaphore_t semaphore_empty;
struct semaphore_t semaphore_full;
struct semaphore_t semaphore_print;

//Project 2 Functions
void print_stats();
void led_on();
void led_off();
void blink_led(uint8_t *w);
void idle();
void producer();
void consumer();
void display_bounded_buffer();

//Bounded Buffer Functions
void display_bounded_buffer();
void producer();
void consumer();


uint16_t buff_size;
uint16_t produce_rate;
uint16_t consume_rate;


void main()
{
    os_init();
    uint8_t del = 250;

    serial_init();

    clear_screen();
   
    create_thread("idle", (uint16_t) idle, 0, 500);
    create_thread("led", (uint16_t) blink_led, &del, 500);
    create_thread("stats", (uint16_t) print_stats, 0, 500);
    create_thread("produce", (uint16_t) producer, 0, 500);
    create_thread("consume", (uint16_t) consumer, 0, 500);
    create_thread("display", (uint16_t) display_bounded_buffer, 0, 500);

    buff_size = 0;
    produce_rate = 10;
    consume_rate = 10;
    
    sem_init(&semaphore_empty, 0);
    sem_init(&semaphore_full, 10);

    //sem_init(&semaphore_print, 1);

    mutex_init(&mutex);

    os_start();

    while (1) {}
}

void print_stats()
{
    uint32_t i, iterations = 0;
    int row = 1;
    char in;
    sys.sys_time = 0;
    while(1)
    {

        mutex_lock(&mutex);
        //sem_wait(&semaphore_print);
        while (byte_available() == 1)
        {
            in = read_byte();
            if ((in == 'r') && (produce_rate > 5))
            {
                produce_rate = produce_rate - 5;
            }
            if (in == 'f')
            {
                produce_rate = produce_rate + 5;
            }
            if ((in == 'u') && (consume_rate > 5))
            {
                consume_rate = consume_rate - 5;
            }
            if (in == 'j')
            {
                consume_rate = consume_rate + 5;
            }
        }

        set_cursor(row,0);
        row++;
        print_string("System Time: ");
        print_int32(sys.sys_time/100);//iterations);

        set_cursor(row,0);
        row++;
        print_string("Interrupts/Sec: ");
        print_int32(sys.last_interrupts);
        print_string(" ");

        set_cursor(row,0);
        row++;       
        print_string("Number of Threads: ");
        print_int(sys.num_threads);
        //print_int(buff_size);

        set_cursor(row,0);
        row++;
        print_string("Current Thread ID: ");
        print_int(sys.curr_thread_id);

        for (i = 0; i < sys.num_threads; i++)
        {
            row++;
            set_cursor(row,0);
            row++;
            print_string("Thread ");
            print_int(i);

            set_cursor(row,0);
            row++;
            print_string("Thread ID: ");
            print_int(sys.thread_buff[i].thread_id);

            set_cursor(row,0);
            row++;
            print_string("Thread Name: ");
            print_string(sys.thread_buff[i].thread_name);

            set_cursor(row,0);
            row++;
            print_string("Thread Status: ");
            if (sys.thread_buff[i].status == THREAD_READY)
            {
                print_string("THREAD_READY");
            }
            if (sys.thread_buff[i].status == THREAD_WAITING) 
            {
                print_string("THREAD_WAITING");
            }
            if (sys.thread_buff[i].status == THREAD_RUNNING) 
            {
                print_string("THREAD_RUNNING");
            }
            if (sys.thread_buff[i].status == THREAD_SLEEPING) 
            {
                print_string("THREAD_SLEEPING");
            }
            set_cursor(row,0);
            row++;
            print_string("Calls/Sec: ");
            print_int(sys.thread_buff[i].last_sched);
            print_string(" ");

            set_cursor(row,0);
            row++;
            print_string("Functional Address (PC): ");
            print_int(sys.thread_buff[i].function_ptr);

            set_cursor(row,0);
            row++;
            print_string("Stack Usage: ");
            print_int((uint16_t)sys.thread_buff[i].stack_high - 
                      (uint16_t)sys.thread_buff[i].stack_pointer);


            set_cursor(row,0);
            row++;
            print_string("Stack Size: ");
            print_int(sys.thread_buff[i].stack_size);

            set_cursor(row,0);
            row++;
            print_string("Stack Pointer: ");
            print_int(sys.thread_buff[i].stack_pointer);

            set_cursor(row,0);
            row++;
            print_string("Stack: ");
            print_int((uint16_t)sys.thread_buff[i].stack_high);
            print_string(" - ");
            print_int((uint16_t)sys.thread_buff[i].stack_low);
            
        }
        iterations++;
        row = 1;
        mutex_unlock(&mutex);
        //_delay_ms(50);
        //mutex_unlock(&mutex);
        //sem_signal(&semaphore_print);
        thread_sleep(5);
    }
}


void blink_led(uint8_t *w)
{
    uint8_t i;
    volatile uint8_t wait;
    wait = *w;
    while (1)
    {
        if (buff_size != 10)
        {
            led_on();
        }
        else
        {
        /*for (i = 0; i < wait; i++)
        {
            _delay_ms(1);
        }*/   
            led_off();
        }
        /*for (i = 0; i < wait; i++)
        {
            _delay_ms(1);
        }*/
    }
}

void producer()
{
    while (1)
    {
        sem_wait(&semaphore_full);
        buff_size++;
        //_delay_ms(10);
        sem_signal(&semaphore_empty);
        thread_sleep(produce_rate);
    }
}

void consumer()
{
    while (1)
    {
        sem_wait(&semaphore_empty);
        buff_size--;
        //_delay_ms(20);
        sem_signal(&semaphore_full);
        thread_sleep(consume_rate);
    }
}

void display_bounded_buffer()
{
    while (1)
    {
        //sem_wait(&semaphore_print);
        mutex_lock(&mutex);
        int row = 1;
        int n;
        set_cursor(row,40);
        print_string("BOUNDED BUFFER");
        row++;
        row++;

        set_cursor(row,40);
        print_string("Buffer Fill: ");
        print_int(buff_size);
        print_string(" ");
        row++;


        if (produce_rate > consume_rate)
        {
            set_color(RED);
        }
        if (produce_rate == consume_rate)
        {
            set_color(YELLOW);
        }
        if (produce_rate < consume_rate)
        {
            set_color(GREEN);
        }
        set_cursor(row,40);
        print_string("Produce Rate: ");
        print_int(produce_rate*10);   
        print_string(" ");
        row++;

        if (produce_rate > consume_rate)
        {
            set_color(GREEN);
        }
        if (produce_rate == consume_rate)
        {
            set_color(YELLOW);
        }
        if (produce_rate < consume_rate)
        {
            set_color(RED);
        }
        set_cursor(row,40);
        print_string("Comsume Rate: ");
        print_int(consume_rate*10); 
        print_string(" ");
        row++;
        row++;
        row++;
        if (buff_size > 8)
        {
            set_color(RED);
        }
        else if (buff_size > 4)
        {
            set_color(YELLOW);
        }
        else
        {
            set_color(GREEN);
        }
        set_cursor(row,40);
        print_string("|");
        for (n = 10; n > 0; n--)
        {
            if (n <= buff_size)
            {
                print_string("X");
            }
            else
            {
                print_string(" ");
            }
        }
        print_string("|");
        set_color(BLUE);
        //sem_signal(&semaphore_print);
        mutex_unlock(&mutex);
        thread_sleep(5);
        //_delay_ms(50);
    }
}

void idle()
{
    int i;
    while(1)
    {
        i = 1;
    }
}


void led_on()
{
    DDRB |= 0x80;
    PORTB |= 0x80;
}


void led_off()
{
    DDRB |= 0x80;
    PORTB &= 0x7F;
}
