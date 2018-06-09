#include <stdlib.h>
#include "ext2.h"

FILE *fp;
uint32_t inodes_c;
uint32_t blocks_c;
uint32_t inodes_per_g;
uint32_t blocks_per_g;

void set_blocks_count(uint32_t bc){
   blocks_c = bc;
}

void set_inodes_count(uint32_t ic){
   inodes_c = ic;
}

void set_inodes_per_group(uint32_t ipg){
   inodes_per_g=ipg;
}

void set_blocks_per_group(uint32_t bpg){
   blocks_per_g=bpg;
}
uint32_t inodes_per_group(){
   //printf("inodes_per_group: %u\n",inodes_per_g);
   return inodes_per_g;
}

uint32_t blocks_per_group(){
   //printf("blocks_per_group: %u\n",blocks_per_g);
   return blocks_per_g;
}

uint32_t inodes_count(){
   return inodes_c;
}

uint32_t blocks_count(){
   return blocks_c;
}

void setFile(FILE *filepointer){
   fp = filepointer;
}

//the block argument is in terms of SD card 512 byte sectors
/*void read_data(uint32_t block, uint16_t offset, uint8_t* data, uint16_t size) {
   if (offset > 511) {
      printf ("Offset greater than 511.\n");
      exit(0);
   }

   fseek(fp,block*512 + offset,SEEK_SET);
   fread(data,size,1,fp);
}*/
