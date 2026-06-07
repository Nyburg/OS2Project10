#include "block.h"
#include "free.h"
#include "inode.h"

#include <stddef.h>

void set_free(unsigned char *block, int num, int set)
{
    int byte_num = num / 8;
    int bit_num = num % 8;

    if (set) {
        block[byte_num] |= (1 << bit_num);
    } else {
        block[byte_num] &= ~(1 << bit_num);
    }
}

int find_free(unsigned char *block)
{
    int byte_num;
    int bit_num;

    for (byte_num = 0; byte_num < BLOCK_SIZE; byte_num++) {
        if (block[byte_num] != 0xff) {
            for (bit_num = 0; bit_num < 8; bit_num++) {
                if (!(block[byte_num] & (1 << bit_num))) {
                    return byte_num * 8 + bit_num;
                }
            }
        }
    }

    return -1;
}

struct inode *ialloc(void)
{
    unsigned char block[BLOCK_SIZE];
    struct inode *in;
    int inode_num;
    int i;

    bread(INODE_MAP_BLOCK, block);

    inode_num = find_free(block);

    if (inode_num == -1) {
        return NULL;
    }

    in = iget(inode_num);

    if (in == NULL) {
        return NULL;
    }

    set_free(block, inode_num, 1);
    bwrite(INODE_MAP_BLOCK, block);

    in->size = 0;
    in->owner_id = 0;
    in->permissions = 0;
    in->flags = 0;
    in->link_count = 0;
    in->inode_num = (unsigned int)inode_num;

    for (i = 0; i < INODE_PTR_COUNT; i++) {
        in->block_ptr[i] = 0;
    }

    write_inode(in);

    return in;
}