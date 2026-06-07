#include "block.h"
#include "dir.h"
#include "pack.h"

#include <stdlib.h>
#include <string.h>

struct directory *directory_open(int inode_num)
{
    struct directory *dir;
    struct inode *inode;

    inode = iget(inode_num);

    if (inode == NULL) {
        return NULL;
    }

    dir = malloc(sizeof(struct directory));

    if (dir == NULL) {
        iput(inode);
        return NULL;
    }

    dir->inode = inode;
    dir->offset = 0;

    return dir;
}

int directory_get(struct directory *dir, struct directory_entry *ent)
{
    unsigned char block[BLOCK_SIZE];
    unsigned int data_block_index;
    unsigned int data_block_num;
    unsigned int offset_in_block;

    if (dir == NULL || ent == NULL) {
        return -1;
    }

    if (dir->offset >= dir->inode->size) {
        return -1;
    }

    data_block_index = dir->offset / BLOCK_SIZE;
    data_block_num = dir->inode->block_ptr[data_block_index];

    bread((int)data_block_num, block);

    offset_in_block = dir->offset % BLOCK_SIZE;

    ent->inode_num = read_u16(block + offset_in_block + DIR_ENTRY_INODE_OFFSET);
    strcpy(ent->name, (char *)(block + offset_in_block + DIR_ENTRY_NAME_OFFSET));

    dir->offset += DIR_ENTRY_SIZE;

    return 0;
}

void directory_close(struct directory *dir)
{
    if (dir == NULL) {
        return;
    }

    iput(dir->inode);
    free(dir);
}