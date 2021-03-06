#include "ext2.h"
#include "globals.h"
#include "os.h"
#include "SdReader.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

void readBlock(uint32_t blockNo, uint8_t *data){
    sdReadData(2*blockNo,0,data,512);
    sdReadData(2*blockNo+1,0,&(data[512]),512);
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

uint16_t fileFormat(struct ext2_inode *in){
    return in->i_mode & 0xF000;
}

int isDirectory(struct ext2_inode *in){
    if(fileFormat(in) == EXT2_S_IFDIR){
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

void getInnerFiles(uint32_t blockNo,uint32_t* song_inodes,uint32_t *idx){
    uint8_t block[1024];
    uint16_t i=0;
    struct ext2_dir_entry *temp;
    readBlock(blockNo,block);
    while(i<1023){
        temp = (struct ext2_dir_entry *)&(block[i]);
        //song_inodes[*idx] = temp->inode;
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
    song_inodes[1] = 4;
    *num_songs = entriesIdx;
}
