#include "ext2.h"
#include "globals.h"
#include "os.h"
#include "SdReader.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

uint32_t findInodeFromBlock(uint32_t blockNo, uint32_t inodeNo, char* name){
    uint8_t block[1024];
    uint16_t i=0;
    uint16_t j;
    struct ext2_dir_entry *temp;
    readBlock(blockNo,block);
    while(i<1023){
        temp = (struct ext2_dir_entry *)&(block[i]);
        if(temp->inode == inodeNo){
            for(j=0;j<temp->name_len;j++){
                name[j] = temp->name[j];
            }
            //End with null char
            name[j]=(char)0;
            return temp->inode;
        }
        i+=temp->rec_len;
    }
    return 0;
}

void getSongTitle(uint32_t inode, char *name){
    uint32_t i;
    struct ext2_inode root;
    struct ext2_inode in;
    getInode(2, &root);
    uint32_t numPtrBlocks = in.i_blocks / 2;
    uint32_t directMax = (numPtrBlocks > 12 ? 11 : numPtrBlocks);
    //Let's assume we're only going to use direct blocks.
    uint32_t temp;

    for(i = 0; i < directMax;i++){
        temp = findInodeFromBlock(in.i_block[i],inode,name);
        if(temp > 0){
            return;
        } 
    }
}

void getSongDuration(uint32_t inode, uint32_t *song_dur){
    struct ext2_inode in;
    getInode(inode, &in);
    *song_dur = in.i_size;
}

void readBlock(uint32_t blockNo, uint8_t *data){
    sdReadData(2*blockNo,0,data,512);
    sdReadData(2*blockNo+1,0,&(data[512]),512);
}


void readFromSD(uint32_t blockNo, uint8_t *data, uint16_t offset, uint16_t size){
    uint32_t bn = 2*blockNo;
    uint16_t os = 0;
    if(offset>512){
        bn+=1;
        os+=offset%512;
    }
    sdReadData(bn,os,data,size);
}

/*
 * inode_number: song inode
 * byteLoc: how far into the song to get byte 
 * data: place to store found note
 */
void getSongByte(uint32_t inode_number,uint32_t byteLoc, uint8_t *data){
    //blockNo: block to look in
    // Should be found in one of the blocks of the file.
    struct ext2_inode in;
    getInode(inode_number,&in);
    uint32_t whichBlock = byteLoc/1024;
    uint16_t offset = byteLoc %1024;
    uint32_t blockNo = 0;
    if(whichBlock < 12){ //We look in direct
        blockNo = in.i_block[whichBlock];
    }else if(whichBlock < (12+256)){ //Singly Indirect
        whichBlock -= 12;
        uint32_t singlyIndirect[256];
        readBlock(in.i_block[12],(uint8_t*)singlyIndirect);
        blockNo = singlyIndirect[whichBlock];
    }else{ //Doubly Indirect
        whichBlock -= (12+256);
        uint32_t doublyIndirect[256];
        uint32_t singlyIndirect[256];
        readBlock(in.i_block[13],(uint8_t*)doublyIndirect);
        readBlock(doublyIndirect[whichBlock/256],(uint8_t*)singlyIndirect);
        blockNo = singlyIndirect[whichBlock%256];
    }
    readFromSD(2*blockNo,data,offset,1);
}

void getInode(int index,struct ext2_inode *in){
    uint32_t group_number = (uint32_t)floor(index/(inodes_per_group()));
    uint32_t group_start = group_number*blocks_per_group()+1;
    uint32_t inodes_start = group_start+4;
    //relative_inode is inode as if the first index in this block group is 0
    uint32_t relative_inode = ((index-1) % inodes_per_group());
    uint32_t extra_chunks = (uint32_t)floor(relative_inode/4);
    uint16_t offset = (uint16_t)(128*(relative_inode%4));
    sdReadData(2*inodes_start+extra_chunks,offset,(uint8_t*)in,128);
}

uint16_t fileFormat(struct ext2_inode *in){
    return in->i_mode & 0xF000;
}


char getMode(uint16_t mode){
    if(mode==EXT2_S_IFSOCK){
        return 'S';
    }
    else if(mode==EXT2_S_IFLNK){
        return 'L';
    }
    else if(mode==EXT2_S_IFREG){
        return 'F';
    }
    else if(mode==EXT2_S_IFBLK){
        return 'B';
    }
    else if(mode==EXT2_S_IFDIR){
        return 'D';
    }
    else if(mode==EXT2_S_IFCHR){
        return 'C';
    }
    else if(mode==EXT2_S_IFIFO){
        return 'Q';
    }else{
        return 'X';
    }
}

int isDirectory(struct ext2_inode *in){
    if(fileFormat(in) == EXT2_S_IFDIR){
        return 1;
    }
    return 0;
}

int isFile(struct ext2_inode *in){
    if(fileFormat(in) == EXT2_S_IFREG){
        return 1;
    }
    return 0;
}

void getInitialInfo(){
    struct ext2_super_block *sb;
    uint8_t data[1024];
    uint32_t super_block_idx = 1;
    readBlock(super_block_idx,data);
    sb = (struct ext2_super_block *)data;
    set_blocks_count(sb->s_blocks_count);
    set_inodes_count(sb->s_inodes_count);
    set_inodes_per_group(sb->s_inodes_per_group);
    set_blocks_per_group(sb->s_blocks_per_group);
}

void formatDirEntry(struct ext2_dir_entry *dir){
    uint8_t i;
    struct ext2_inode in;
    uint8_t maxLen = dir->name_len;
    
    maxLen = dir->name_len > 100 ? 100 : maxLen;
    getInode(dir->inode,&in);

    for(i=0;i<maxLen;i++){
        song_name[i] = dir->name[i];
    }

    song_dur = in.i_size;
}

void printEntries(char **entries,uint32_t size){
    uint32_t i;
    for(i=0;i<size;i++){
        printf("%s",entries[i]);
    }
}

void printBlock(uint32_t blockNo,uint32_t count, uint32_t size){
    uint8_t block[1024];
    uint16_t i;
    readBlock(blockNo,block);
    uint32_t max=size-count>1024?1024:size-count;
    for(i=0;i<max;i++){
        printf("%c",block[i]);
    }
}

void printFileContents(uint32_t inode){
    struct ext2_inode in;
    getInode(inode,&in);
    if(isDirectory(&in)){
        printf("Cannot print this, because it is a directory!\n");
        exit(0);
    }
    uint32_t numPtrBlocks = in.i_blocks / 2;
    //Go through 12 direct blocks
    uint32_t directMax = (numPtrBlocks > 12 ? 12 : numPtrBlocks);
    uint32_t singleMax = (numPtrBlocks > 256+12 ? 256 : numPtrBlocks);
    uint32_t doubleMax = (numPtrBlocks > 256*256+12 ? 256*256 : numPtrBlocks);
    uint32_t i;
    //Go through singly indirect blocks
    uint32_t size = in.i_size;
    uint32_t count = 0;
    for(i = 0; i < directMax;i++){
        printBlock(in.i_block[i],count,size);
        count+=1024;
        if(count >= size){
            return;
        }
    }
    //Go through singly indirect blocks
    uint32_t singlyIndirect[256];
    readBlock(in.i_block[12],(uint8_t*)singlyIndirect);
    for(i = 0; i < singleMax;i++){
        printBlock(singlyIndirect[i],count,size);
        count+=1024;
        if(count >= size){
            return;
        }
    }
    //Go through doubly indirect blocks
    uint32_t doublyIndirect[256];
    uint32_t currentBlock[256];
    int doubleIdx = -1;
    readBlock(in.i_block[13],(uint8_t*)doublyIndirect);
    for(i = 0; i < doubleMax;i++){
        uint32_t idx=i%256;
        if(idx == 0){
            doubleIdx++;
            readBlock(doublyIndirect[doubleIdx],(uint8_t*)currentBlock);
        }
        printBlock(currentBlock[idx],count,size);
        count+=1024;
        if(count >= size){
            return;
        }
    }
}

void getInnerFiles(uint32_t blockNo,uint32_t* song_inodes,uint32_t *idx){
    uint8_t block[1024];
    uint16_t i=0;
    struct ext2_dir_entry *temp;
    readBlock(blockNo,block);
    while(i<1023){
        temp = (struct ext2_dir_entry *)&(block[i]);
        song_inodes[*idx] = temp->inode;
        (*idx)++;
        i+=temp->rec_len;
    }
}


void getInfo(uint32_t *num_songs, uint32_t *song_inodes){
    struct ext2_inode in;
    getInode(2, &in);
    if(!isDirectory(&in)){
        exit(0);
    }
    uint32_t numPtrBlocks = in.i_blocks / 2;
    uint32_t directMax = (numPtrBlocks > 12 ? 11 : numPtrBlocks);
    uint32_t i;
    uint32_t entriesIdx = 0;
    for(i = 0; i < directMax;i++){
        getInnerFiles(in.i_block[i],song_inodes,&entriesIdx);
    }
    *num_songs = entriesIdx;

}

void printContents(uint32_t inode){
    struct ext2_inode in;
    getInode(inode,&in);
    if(!isDirectory(&in)){
        printf("Cannot print files within this, because it is not a directory!\n");
        exit(0);
    }
    uint32_t numPtrBlocks = in.i_blocks / 2;
    //Go through 12 direct blocks
    uint32_t directMax = (numPtrBlocks > 12 ? 11 : numPtrBlocks);
    uint32_t singleMax = (numPtrBlocks > 256+12 ? 256 : numPtrBlocks);
    uint32_t doubleMax = (numPtrBlocks > 256*256+12 ? 256*256 : numPtrBlocks);
    uint32_t i;
    uint32_t entriesIdx = 0;
    //Go through singly indirect blocks
    //printf("name          size                     type\n");
    char **entries = malloc(276*sizeof(char*));
    for(i = 0; i < directMax;i++){
        getInnerFiles(in.i_block[i],entries,&entriesIdx);
    }
    if(numPtrBlocks <= 12){
        printEntries(entries,entriesIdx);
        return;
    }
    //Go through singly indirect blocks
    uint32_t singlyIndirect[256];
    readBlock(in.i_block[12],(uint8_t*)singlyIndirect);
    for(i = 0; i < singleMax;i++){
        getInnerFiles(singlyIndirect[i],entries,&entriesIdx);
    }
    if(numPtrBlocks <= 12+256){
        printEntries(entries,numPtrBlocks);
        return;
    }
    //Go through doubly indirect blocks
    uint32_t doublyIndirect[256];
    uint32_t currentBlock[256];
    int doubleIdx = -1;
    readBlock(in.i_block[13],(uint8_t*)doublyIndirect);
    for(i = 0; i < doubleMax;i++){
        uint32_t idx=i%256;
        if(idx == 0){
            doubleIdx++;
            readBlock(doublyIndirect[doubleIdx],(uint8_t*)currentBlock);
        }
        getInnerFiles(currentBlock[idx],entries,&entriesIdx);
    }
    printEntries(entries,numPtrBlocks);
}
