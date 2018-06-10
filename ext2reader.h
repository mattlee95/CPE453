#ifndef EXT2READER_H
#define EXT2READER_H

void getSongByte(uint32_t inode_number,uint32_t byteLoc, uint8_t *data);
void getSongTitle(uint32_t inode, char *name);
void getSongDuration(uint32_t inode, uint32_t *song_dur);
void getInfo(uint32_t *num_songs, uint32_t *song_inodes);
void getInitialInfo();

#endif
