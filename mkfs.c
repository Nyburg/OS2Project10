#include "block.h"
#include "dir.h"
#include "free.h"
#include "mkfs.h"
#include "pack.h"

#include <string.h>

void mkfs(void)
{
    unsigned char block[BLOCK_SIZE];
    struct inode *root_inode;
    int root_block_num;

    memset(block, 0, BLOCK_SIZE);

    bwrite(0, block);
    bwrite(1, block);
    bwrite(3, block);
    bwrite(4, block);
    bwrite(5, block);
    bwrite(6, block);

    memset(block, 0, BLOCK_SIZE);
    set_free(block, 0, 1);
    set_free(block, 1, 1);
    set_free(block, 2, 1);
    set_free(block, 3, 1);
    set_free(block, 4, 1);
    set_free(block, 5, 1);
    set_free(block, 6, 1);

    bwrite(2, block);

    root_inode = ialloc();
    root_block_num = alloc();

    memset(block, 0, BLOCK_SIZE);

    root_inode->flags = INODE_FLAG_DIR;
    root_inode->size = ROOT_DIR_ENTRY_COUNT * DIR_ENTRY_SIZE;
    root_inode->block_ptr[0] = (unsigned short)root_block_num;

    write_u16(block + DIR_ENTRY_INODE_OFFSET, root_inode->inode_num);
    strcpy((char *)(block + DIR_ENTRY_NAME_OFFSET), DIR_ENTRY_DOT);

    write_u16(block + DIR_ENTRY_SIZE + DIR_ENTRY_INODE_OFFSET,
              root_inode->inode_num);
    strcpy((char *)(block + DIR_ENTRY_SIZE + DIR_ENTRY_NAME_OFFSET),
           DIR_ENTRY_DOT_DOT);

    bwrite(root_block_num, block);

    iput(root_inode);
}