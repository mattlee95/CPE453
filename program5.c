#include <avr/io.h>
#include <avr/interrupt.h>
#include "globals.h"
#include "os.h"
#include <util/delay.h>
#include <stdlib.h>
#include "ext2.h"
#include "SdReader.h"
#include "serial.h"
#include "dbuffer.h"

extern system_t sys;

double_buff db;

void display_song_stats();
void speak_value();
void idle();
void print_stats();

uint8_t val;

int main(void) {

    val = 0;

    os_init();

    uint8_t sd_card_status;

    sd_card_status = sdInit(1);   //initialize the card with slow clock

    serial_init(); 
    print_string("HERE");

    create_thread("idle", (uint16_t) idle, 0, 500);
    //create_thread("save value", (uint16_t) save_value, &val, 500);
    //create_thread("speak value", (uint16_t) speak_value, 0, 500);
    //create_thread("display song stats", (uint16_t) display_song_stats, 0, 500);
    create_thread("stats", (uint16_t) print_stats, 0, 500);
    print_string("HERE2");

    start_audio_pwm();
    print_string("HERE3");

    os_start();   
    while (1) {}
}


void display_song_stats()
{
    int row = 1;
    char in;

    while (byte_available() == 1)
    {
        in = read_byte();
        if (in == 'n')
        {
            //next song
        }
        if (in == 'p')
        {
            //prev song
        }
    }
    set_cursor(row,0);
    print_string("Song Name: ");
    print_string(song_name);
    row++;

    set_cursor(row,0);
    print_string("Seconds In: ");
    print_int(curr_dur);
    print_string("   ");
    row++;

    set_cursor(row,0);
    print_string("Total Duration: ");
    print_int(song_dur);
    print_string("   ");
    row++;
}

void speak_value()
{
    //do the speaker stuff
    uint8_t value;
    int ret = 1;
    while (1)
        //ret = speak_from_buffer(&db, &value);
        print_string("X");
        val = val + 10;    

        if (ret > 0)
        {
            //OCR2B = value;
            OCR2B = val;
            thread_sleep(1);
        }
        else
        {
            thread_sleep(1);
        }
}



void idle()
{
    int i;
    while(1)
    {
        //noop
        print_string("Z");
        i = 0;
    }

}




void print_stats()
{
    uint32_t i, iterations = 0;
    int row = 1;
    char in;
    sys.sys_time = 0;
    while(1)
    {

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
        //_delay_ms(50);
        //mutex_unlock(&mutex);
        //sem_signal(&semaphore_print);
        thread_sleep(5);
    }
}
