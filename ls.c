#include "dir.h"
#include "ls.h"

#include <stdio.h>

void ls(void)
{
    struct directory *dir;
    struct directory_entry ent;

    dir = directory_open(ROOT_INODE_NUM);

    if (dir == NULL) {
        return;
    }

    while (directory_get(dir, &ent) != -1) {
        printf("%d %s\n", ent.inode_num, ent.name);
    }

    directory_close(dir);
}