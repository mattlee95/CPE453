#include <avr/io.h>
#include <avr/interrupt.h>
#include "globals.h"
#include "os.h"
#include <util/delay.h>
#include <stdlib.h>
#include "ext2.h"
#include "SdReader.h"

#include "dbuffer.h"

doublebuff db;

int main(void) {
    uint8_t sd_card_status;

    sd_card_status = sdInit(1);   //initialize the card with slow clock

    serial_init(); 

    create_thread("idle", (uint16_t) idle, 0, 500);
    create_thread("save value", (uint16_t) save_value, &val, 500);
    create_thread("speak value", (uint16_t) speak_value, 0, 500);
    create_thread("display song stats", (uint16_t) display_song_stats, 0, 500);

    start_audio_pwm();
    os_init();
   
   return 0;
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
    row++:
}

void speak_value()
{
    //do the speaker stuff
    uint8_t value;
    int ret;

    ret = speak_from_buffer(&db, &value);
    
    if (ret > 0)
    {
        OCR2B = value;
    }
    else
    {
        sleep(1);
    }
}



void idle()
{
    int i;
    while(1)
    {
        //noop
        i = 0;
    }

}
