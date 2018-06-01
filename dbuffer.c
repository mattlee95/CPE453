#include <util/delay.h>
#include <stdlib.h>

#include "dbuffer.h"

void initialize_double_buffer(double_buff *db)
{
    db->index_buffer1 = 0;
    db->index_buffer2 = 0;
    
    db->speaker_buff = 3;
    db->saving_buff = 1;
}

//Returns 1 if save successfull
//Returns -1 if save bad, and value must be sent again
int save_to_buffer(double_buff *db, uint8_t val)
{
    if (((db->index_buffer1 > 255) && (db->saving_buff == 1)) ||
        ((db->index_buffer2 > 255) && (db->saving_buff == 2)))
    {
        //no more room, waiting for buffer swap
        check_swap(db);
        return -1;
    }

    if (db->saving_buff == 1)
    {
        db->buffer1[db->index_buffer1] = val;
        db->index_buffer1++;
        check_swap(db);
        return 1;
    }

    if (db->saving_buff == 2)
    {
        db->buffer2[db->index_buffer2] = val;
        db->index_buffer2++;
        check_swap(db);
        return 1;
    }
    //should not get here
    return -1;
}


//Returns 1 if data given back
//Returns -1 if no data given back
int speak_from_buffer(double_buff *db, uint8_t *ret_val)
{
    if (((db->index_buffer1 > 255) && (db->speaker_buff == 1)) ||
        ((db->index_buffer2 > 255) && (db->speaker_buff == 2)) ||
        (db->speaker_buff > 2))
    {
        //no data ready
        check_swap(db);
        return -1;
    }

    if (db->speaker_buff == 1)
    {
        *ret_val = db->buffer1[db->index_buffer1];
        db->index_buffer1++;
        check_swap(db);
        return 1;
    }
    if (db->speaker_buff == 2)
    {
        *ret_val = db->buffer2[db->index_buffer2];
        db->index_buffer2++;
        check_swap(db);
        return 1;
    }
    //should get here
    return -1;
}

void check_swap(double_buff *db)
{
    //case: just starting and we haven't assigned a speaker buffer yet
    if ((db->speaker_buff > 2) && (db->index_buffer1 > 255))
    {
        db->speaker_buff = 1;
        db->saving_buff = 2;
        db->index_buffer1 = 0;
        db->index_buffer2 = 0;
    }

    if ((db->index_buffer1 > 255) && (db->index_buffer2 > 255))
    {
        if (db->saving_buff == 1)
        {
            db->saving_buff = 2;
            db->speaker_buff = 1;
        }
        else
        {
            db->saving_buff = 1;
            db->speaker_buff = 2;
        }
        db->index_buffer1 = 0;
        db->index_buffer2 = 0;
    }
}
