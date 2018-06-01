#ifndef DBUFFER_H
#define DBUFFER_H

typedef struct 
{
    uint8_t buffer1[256];
    uint16_t index_buffer1;

    uint8_t buffer2[256];
    uint16_t index_buffer2;

    uint8_t speaker_buff;
    uint8_t saving_buff;
} double_buff;

void initialize_double_buffer(double_buff *db);
int save_to_buffer(double_buff *db, uint8_t val);
int speak_from_buffer(double_buff *db, uint8_t *ret_val);
void check_swap(double_buff *db);

#endif
