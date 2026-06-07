#include "block.h"
#include "inode.h"
#include "pack.h"

#include <stddef.h>

static struct inode incore[MAX_SYS_OPEN_FILES] = {0};

struct inode *incore_find_free(void)
{
    int i;
    for (i = 0; i < MAX_SYS_OPEN_FILES; i++) {
        if (incore[i].ref_count == 0) {
            return &incore[i];
        }
    }
    return NULL;
}

struct inode *incore_find(unsigned int inode_num)
{
    int i;
    for (i = 0; i < MAX_SYS_OPEN_FILES; i++) {
        if (incore[i].ref_count != 0 && incore[i].inode_num == inode_num) {
            return &incore[i];
        }
    }
    return NULL;
}

void incore_free_all(void)
{
    int i;
    for (i = 0; i < MAX_SYS_OPEN_FILES; i++) {
        incore[i].ref_count = 0;
    }
}

void read_inode(struct inode *in, int inode_num)
{
    unsigned char block[BLOCK_SIZE];
    unsigned char *inode_start;
    int block_num;
    int block_offset;
    int block_offset_bytes;
    int i;

    block_num = inode_num / INODES_PER_BLOCK + INODE_FIRST_BLOCK;
    block_offset = inode_num % INODES_PER_BLOCK;
    block_offset_bytes = block_offset * INODE_SIZE;

    bread(block_num, block);

    inode_start = block + block_offset_bytes;

    in->size = read_u32(inode_start + 0);
    in->owner_id = read_u16(inode_start + 4);
    in->permissions = read_u8(inode_start + 6);
    in->flags = read_u8(inode_start + 7);
    in->link_count = read_u8(inode_start + 8);

    for (i = 0; i < INODE_PTR_COUNT; i++) {
        in->block_ptr[i] = read_u16(inode_start + 9 + i * 2);
    }

    in->inode_num = (unsigned int)inode_num;
}

void write_inode(struct inode *in)
{
    unsigned char block[BLOCK_SIZE];
    unsigned char *inode_start;
    int block_num;
    int block_offset;
    int block_offset_bytes;
    int i;

    block_num = in->inode_num / INODES_PER_BLOCK + INODE_FIRST_BLOCK;
    block_offset = in->inode_num % INODES_PER_BLOCK;
    block_offset_bytes = block_offset * INODE_SIZE;

    bread(block_num, block);

    inode_start = block + block_offset_bytes;

    write_u32(inode_start + 0, in->size);
    write_u16(inode_start + 4, in->owner_id);
    write_u8(inode_start + 6, in->permissions);
    write_u8(inode_start + 7, in->flags);
    write_u8(inode_start + 8, in->link_count);

    for (i = 0; i < INODE_PTR_COUNT; i++) {
        write_u16(inode_start + 9 + i * 2, in->block_ptr[i]);
    }

    bwrite(block_num, block);
}

struct inode *iget(int inode_num)
{
    struct inode *in;

    in = incore_find((unsigned int)inode_num);

    if (in != NULL) {
        in->ref_count++;
        return in;
    }

    in = incore_find_free();

    if (in == NULL) {
        return NULL;
    }

    read_inode(in, inode_num);

    in->ref_count = 1;
    in->inode_num = (unsigned int)inode_num;

    return in;
}

void iput(struct inode *in)
{
    if (in->ref_count == 0) {
        return;
    }

    in->ref_count--;

    if (in->ref_count == 0) {
        write_inode(in);
    }
}