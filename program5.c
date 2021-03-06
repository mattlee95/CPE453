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
#include "synchro.h"
#include "ext2reader.h"

extern system_t sys;

double_buff db;

struct mutex_t mutex;

void send_value_to_speaker();
void idle();
void print_stats();

void changeSong();
void next_song();
void prev_song();
void globals_init();
void save_value(int a);
void display_bounded_buffer();

uint8_t val;

int main(void) {

    val = 0;

    initialize_double_buffer(&db);

    serial_init();

    uint8_t sd_card_status;
    sd_card_status = sdInit(1);   //initialize the card with slow clock
    if (sd_card_status == 0)
    {
        print_string("BAD SD CARD INIT");
    }
    start_audio_pwm();

    globals_init();

    os_init();

    //create_thread("idle", (uint16_t) idle, 0, 500);
    create_thread("speak", (uint16_t) send_value_to_speaker, 0, 500);
    create_thread("save value", (uint16_t) save_value, &val, 500);
    create_thread("stats", (uint16_t) print_stats, 0, 500);
    create_thread("idle", (uint16_t) idle, 0, 500);

    mutex_init(&mutex);

    clear_screen();

    os_start(); 

    while (1)
    {
    }
}

void changeSong(){
    // Change curr_dur to 0
    curr_dur = 0;
    //get current inode number 
    uint32_t curr_inode =3;//= song_inodes[curr_song_idx];
    // Change song_name
    getSongTitle(curr_inode,song_name);
    //Change song_dur
    getSongDuration(curr_inode,(uint32_t*)&song_dur);    
}

void next_song(){
    // Change curr_song_idx
    curr_song_idx++;
    curr_song_idx %= num_songs;
    //Do the rest
    changeSong();
}


void prev_song(){
    // Change curr_song_idx
    curr_song_idx = curr_song_idx == 0 ? num_songs-1 : curr_song_idx - 1;
    //Do the rest
    changeSong();
}

void globals_init(){
    getInitialInfo();
    curr_song_idx = 0;
    curr_dur = 0;
    getInfo((uint32_t*)&num_songs,(uint32_t*)song_inodes); //Initializes num_songs and song_inodes
    uint32_t curr_inode = song_inodes[curr_song_idx]; 
    getSongTitle(curr_inode,song_name); //Initializes song_name
    getSongDuration(curr_inode,(uint32_t*)&song_dur); //Initializes song_dur
}

void send_value_to_speaker()
{
    while (1)
    {
        uint8_t ret_val;
        int ret = speak_from_buffer(&db, &ret_val);
        while (ret == -1)
        {
            yield();
            ret = speak_from_buffer(&db, &ret_val);
        }
        OCR2B = ret_val;
        yield();
    }
}

void save_value(int a){
    
    uint8_t value = 0;
    while (1)
    {
        uint8_t data[1];
        uint8_t byte;
        int ret;
        value = value + 25;
        if ((value > 240) || (value < 25))
        {
            value = 0;
        }
        // What is blockNo
        //getSongByte(song_inodes[curr_song_idx],curr_dur,data);
        byte = data[0];
        //ret = save_to_buffer(&db, byte);
        ret = save_to_buffer(&db, value);
        while(ret == -1){
            ret = save_to_buffer(&db, value);
            yield();
        }
        yield();
    }
}

void idle()
{
    int i;
    while(1)
    {
        i = 0;
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
        print_string(" ");
        row++;


        set_cursor(row,40);
        print_string("Produce Rate: ");
        print_string(" ");
        row++;

        set_cursor(row,40);
        print_string("Comsume Rate: ");
        print_string(" ");
        row++;
        row++;
        row++;
        set_cursor(row,40);
        print_string("|");
        print_string("|");
        mutex_unlock(&mutex);
        yield();
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
        mutex_lock(&mutex);
        set_cursor(row,0);
        row++;
        print_string("System Time: ");
        print_int32(sys.sys_time/100);//iterations);
        //print_int32(song_inodes[0]);
        set_cursor(row,0);

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

        row = 1;
        set_cursor(row,40);
        row++;
        print_string("Song Title: ");
        //print_string(song_title);        

        set_cursor(row,40);
        row++;
        print_string("Song Duration:");
        //print_int(song_dur);

        set_cursor(row,40);
        row++;
        print_string("Current Duration:");
        //print_int(curr_dur);

        set_cursor(row,40);
        print_string("|");
        /*for (i = 0; i < 10);
        {
            if (curr_dur > song_dur%10 * i)
            {
                print_string("X");
            }
            else
            {
                print_string(" ");
            }
        }*/   

        print_string("|");
    

        iterations++;
        row = 1;
        //_delay_ms(50);
        mutex_unlock(&mutex);
        //sem_signal(&semaphore_print);
        yield();
    }
}
