#ifndef FREE_H
#define FREE_H

#define INODE_MAP_BLOCK 1
#define FREE_MAP_BLOCK 2

struct inode;

void set_free(unsigned char *block, int num, int set);
int find_free(unsigned char *block);

struct inode *ialloc(void);
int alloc(void);

#endif