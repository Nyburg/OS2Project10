#ifndef DIR_H
#define DIR_H

#include "inode.h"

#define DIR_ENTRY_SIZE 32
#define DIR_ENTRY_NAME_SIZE 16
#define DIR_ENTRY_RESERVED_SIZE 14

#define DIR_ENTRY_INODE_OFFSET 0
#define DIR_ENTRY_NAME_OFFSET 2

#define ROOT_INODE_NUM 0
#define ROOT_DIR_ENTRY_COUNT 2
#define DIR_ENTRY_DOT "."
#define DIR_ENTRY_DOT_DOT ".."

#define INODE_FLAG_UNKNOWN 0
#define INODE_FLAG_FILE 1
#define INODE_FLAG_DIR 2

struct directory_entry {
    unsigned int inode_num;
    char name[DIR_ENTRY_NAME_SIZE];
};

struct directory {
    struct inode *inode;
    unsigned int offset;
};

struct directory *directory_open(int inode_num);
int directory_get(struct directory *dir, struct directory_entry *ent);
void directory_close(struct directory *dir);

#endif