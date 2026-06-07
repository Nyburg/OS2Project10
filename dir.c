#include "block.h"
#include "dir.h"
#include "pack.h"
#include "free.h"

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

int directory_make(char *path)
{
    unsigned char block[BLOCK_SIZE];
    struct inode *parent_inode;
    struct inode *new_inode;
    char *name;
    unsigned int entry_offset;
    int block_num;

    if (path == NULL || path[0] != '/') {
        return -1;
    }

    if (strcmp(path, "/") == 0) {
        return -1;
    }

    name = path + 1;

    if (strlen(name) == 0 || strlen(name) >= DIR_ENTRY_NAME_SIZE) {
        return -1;
    }

    if (strchr(name, '/') != NULL) {
        return -1;
    }

    new_inode = namei(path);

    if (new_inode != NULL) {
        iput(new_inode);
        return -1;
    }

    parent_inode = namei("/");

    if (parent_inode == NULL) {
        return -1;
    }

    new_inode = ialloc();

    if (new_inode == NULL) {
        iput(parent_inode);
        return -1;
    }

    block_num = alloc();

    if (block_num == -1) {
        iput(new_inode);
        iput(parent_inode);
        return -1;
    }

    memset(block, 0, BLOCK_SIZE);

    write_u16(block + DIR_ENTRY_INODE_OFFSET, new_inode->inode_num);
    strcpy((char *)(block + DIR_ENTRY_NAME_OFFSET), DIR_ENTRY_DOT);

    write_u16(block + DIR_ENTRY_SIZE + DIR_ENTRY_INODE_OFFSET,
              parent_inode->inode_num);
    strcpy((char *)(block + DIR_ENTRY_SIZE + DIR_ENTRY_NAME_OFFSET),
           DIR_ENTRY_DOT_DOT);

    new_inode->flags = INODE_FLAG_DIR;
    new_inode->size = ROOT_DIR_ENTRY_COUNT * DIR_ENTRY_SIZE;
    new_inode->block_ptr[0] = (unsigned short)block_num;

    bwrite(block_num, block);

    bread(parent_inode->block_ptr[0], block);

    entry_offset = parent_inode->size;

    write_u16(block + entry_offset + DIR_ENTRY_INODE_OFFSET,
              new_inode->inode_num);
    strcpy((char *)(block + entry_offset + DIR_ENTRY_NAME_OFFSET), name);

    bwrite(parent_inode->block_ptr[0], block);

    parent_inode->size += DIR_ENTRY_SIZE;

    iput(new_inode);
    iput(parent_inode);

    return 0;
}

struct inode *namei(char *path)
{
    struct directory *dir;
    struct directory_entry ent;
    char *name;

    if (path == NULL || path[0] != '/') {
        return NULL;
    }

    if (strcmp(path, "/") == 0) {
        return iget(ROOT_INODE_NUM);
    }

    name = path + 1;

    if (strlen(name) == 0 || strchr(name, '/') != NULL) {
        return NULL;
    }

    dir = directory_open(ROOT_INODE_NUM);

    if (dir == NULL) {
        return NULL;
    }

    while (directory_get(dir, &ent) == 0) {
        if (strcmp(ent.name, name) == 0) {
            directory_close(dir);
            return iget(ent.inode_num);
        }
    }

    directory_close(dir);
    return NULL;
}