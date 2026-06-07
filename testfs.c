#include "block.h"
#include "ctest.h"
#include "free.h"
#include "image.h"
#include "inode.h"
#include "mkfs.h"
#include "dir.h"

#include <string.h>
#include <unistd.h>

#define TEST_IMAGE "testfs.img"

static int test_image_open_close(void)
{
    int fd = image_open(TEST_IMAGE, 1);

    CTEST_ASSERT(fd >= 0);
    CTEST_ASSERT(image_close() == 0);

    unlink(TEST_IMAGE);

    return 0;
}

static int test_block_write_read(void)
{
    unsigned char write_block[BLOCK_SIZE];
    unsigned char read_block[BLOCK_SIZE];
    int i;

    CTEST_ASSERT(image_open(TEST_IMAGE, 1) >= 0);

    for (i = 0; i < BLOCK_SIZE; i++) {
        write_block[i] = (unsigned char)(i % 256);
        read_block[i] = 0;
    }

    bwrite(0, write_block);
    bread(0, read_block);

    CTEST_ASSERT(memcmp(write_block, read_block, BLOCK_SIZE) == 0);
    CTEST_ASSERT(image_close() == 0);

    unlink(TEST_IMAGE);

    return 0;
}

static int test_set_free(void)
{
    unsigned char block[BLOCK_SIZE];

    memset(block, 0, BLOCK_SIZE);

    set_free(block, 0, 1);
    CTEST_ASSERT(block[0] == 1);

    set_free(block, 1, 1);
    CTEST_ASSERT(block[0] == 3);

    set_free(block, 0, 0);
    CTEST_ASSERT(block[0] == 2);

    return 0;
}

static int test_find_free(void)
{
    unsigned char block[BLOCK_SIZE];

    memset(block, 0xff, BLOCK_SIZE);
    block[0] = 0xfe;

    CTEST_ASSERT(find_free(block) == 0);

    block[0] = 0xff;
    block[1] = 0xfd;

    CTEST_ASSERT(find_free(block) == 9);

    return 0;
}

static int test_incore_find_free(void)
{
    struct inode *in;

    incore_free_all();

    in = incore_find_free();

    CTEST_ASSERT(in != NULL);
    CTEST_ASSERT(in->ref_count == 0);

    return 0;
}

static int test_incore_find(void)
{
    struct inode *in;

    incore_free_all();

    in = incore_find_free();

    CTEST_ASSERT(in != NULL);

    in->ref_count = 1;
    in->inode_num = 42;
    in->size = 1234;

    CTEST_ASSERT(incore_find(42) == in);
    CTEST_ASSERT(incore_find(99) == NULL);

    return 0;
}

static int test_read_write_inode(void)
{
    struct inode write_in;
    struct inode read_in;
    int i;

    CTEST_ASSERT(image_open(TEST_IMAGE, 1) >= 0);

    memset(&write_in, 0, sizeof(write_in));
    memset(&read_in, 0, sizeof(read_in));

    write_in.size = 123456;
    write_in.owner_id = 25;
    write_in.permissions = 7;
    write_in.flags = 3;
    write_in.link_count = 2;
    write_in.inode_num = 5;

    for (i = 0; i < INODE_PTR_COUNT; i++) {
        write_in.block_ptr[i] = (unsigned short)(100 + i);
    }

    write_inode(&write_in);
    read_inode(&read_in, 5);

    CTEST_ASSERT(read_in.size == 123456);
    CTEST_ASSERT(read_in.owner_id == 25);
    CTEST_ASSERT(read_in.permissions == 7);
    CTEST_ASSERT(read_in.flags == 3);
    CTEST_ASSERT(read_in.link_count == 2);
    CTEST_ASSERT(read_in.inode_num == 5);

    for (i = 0; i < INODE_PTR_COUNT; i++) {
        CTEST_ASSERT(read_in.block_ptr[i] == (unsigned short)(100 + i));
    }

    CTEST_ASSERT(image_close() == 0);

    unlink(TEST_IMAGE);

    return 0;
}

static int test_iget_iput(void)
{
    struct inode write_in;
    struct inode *in1;
    struct inode *in2;
    struct inode read_in;

    CTEST_ASSERT(image_open(TEST_IMAGE, 1) >= 0);

    incore_free_all();

    memset(&write_in, 0, sizeof(write_in));
    memset(&read_in, 0, sizeof(read_in));

    write_in.size = 888;
    write_in.owner_id = 12;
    write_in.permissions = 5;
    write_in.flags = 1;
    write_in.link_count = 3;
    write_in.inode_num = 9;
    write_in.block_ptr[0] = 77;

    write_inode(&write_in);

    in1 = iget(9);

    CTEST_ASSERT(in1 != NULL);
    CTEST_ASSERT(in1->inode_num == 9);
    CTEST_ASSERT(in1->ref_count == 1);
    CTEST_ASSERT(in1->size == 888);
    CTEST_ASSERT(in1->block_ptr[0] == 77);

    in2 = iget(9);

    CTEST_ASSERT(in2 == in1);
    CTEST_ASSERT(in1->ref_count == 2);

    in1->size = 999;

    iput(in1);
    CTEST_ASSERT(in1->ref_count == 1);

    iput(in1);
    CTEST_ASSERT(in1->ref_count == 0);

    read_inode(&read_in, 9);
    CTEST_ASSERT(read_in.size == 999);

    CTEST_ASSERT(image_close() == 0);

    unlink(TEST_IMAGE);

    return 0;
}

static int test_ialloc(void)
{
    unsigned char block[BLOCK_SIZE];
    struct inode *in1;
    struct inode *in2;

    CTEST_ASSERT(image_open(TEST_IMAGE, 1) >= 0);

    incore_free_all();

    memset(block, 0, BLOCK_SIZE);
    bwrite(1, block);
    bwrite(3, block);
    bwrite(4, block);
    bwrite(5, block);
    bwrite(6, block);

    in1 = ialloc();
    CTEST_ASSERT(in1 != NULL);
    CTEST_ASSERT(in1->inode_num == 0);
    CTEST_ASSERT(in1->ref_count == 1);
    CTEST_ASSERT(in1->size == 0);
    CTEST_ASSERT(in1->owner_id == 0);
    CTEST_ASSERT(in1->permissions == 0);
    CTEST_ASSERT(in1->flags == 0);

    in2 = ialloc();
    CTEST_ASSERT(in2 != NULL);
    CTEST_ASSERT(in2->inode_num == 1);
    CTEST_ASSERT(in2->ref_count == 1);

    bread(1, block);
    CTEST_ASSERT(block[0] == 3);

    CTEST_ASSERT(image_close() == 0);

    unlink(TEST_IMAGE);

    return 0;
}

static int test_alloc(void)
{
    unsigned char block[BLOCK_SIZE];

    CTEST_ASSERT(image_open(TEST_IMAGE, 1) >= 0);

    memset(block, 0, BLOCK_SIZE);
    bwrite(2, block);

    CTEST_ASSERT(alloc() == 0);
    CTEST_ASSERT(alloc() == 1);

    bread(2, block);
    CTEST_ASSERT(block[0] == 3);

    CTEST_ASSERT(image_close() == 0);

    unlink(TEST_IMAGE);

    return 0;
}

static int test_mkfs(void)
{
    unsigned char block[BLOCK_SIZE];

    CTEST_ASSERT(image_open(TEST_IMAGE, 1) >= 0);

    incore_free_all();
    mkfs();

    bread(1, block);
    CTEST_ASSERT(block[0] == 1);

    bread(2, block);
    CTEST_ASSERT(block[0] == 0xff);

    CTEST_ASSERT(alloc() == 8);

    CTEST_ASSERT(image_close() == 0);

    unlink(TEST_IMAGE);

    return 0;
}

static int test_directory_open_close(void)
{
    struct directory *dir;

    CTEST_ASSERT(image_open(TEST_IMAGE, 1) >= 0);

    incore_free_all();
    mkfs();

    dir = directory_open(ROOT_INODE_NUM);

    CTEST_ASSERT(dir != NULL);
    CTEST_ASSERT(dir->inode != NULL);
    CTEST_ASSERT(dir->inode->inode_num == ROOT_INODE_NUM);
    CTEST_ASSERT(dir->offset == 0);

    directory_close(dir);

    CTEST_ASSERT(image_close() == 0);

    unlink(TEST_IMAGE);

    return 0;
}

static int test_directory_get_root_entries(void)
{
    struct directory *dir;
    struct directory_entry ent;

    CTEST_ASSERT(image_open(TEST_IMAGE, 1) >= 0);

    incore_free_all();
    mkfs();

    dir = directory_open(ROOT_INODE_NUM);

    CTEST_ASSERT(dir != NULL);

    CTEST_ASSERT(directory_get(dir, &ent) == 0);
    CTEST_ASSERT(ent.inode_num == ROOT_INODE_NUM);
    CTEST_ASSERT(strcmp(ent.name, DIR_ENTRY_DOT) == 0);

    CTEST_ASSERT(directory_get(dir, &ent) == 0);
    CTEST_ASSERT(ent.inode_num == ROOT_INODE_NUM);
    CTEST_ASSERT(strcmp(ent.name, DIR_ENTRY_DOT_DOT) == 0);

    CTEST_ASSERT(directory_get(dir, &ent) == -1);

    directory_close(dir);

    CTEST_ASSERT(image_close() == 0);

    unlink(TEST_IMAGE);

    return 0;
}

int main(void)
{
    int failures = 0;

    CTEST_RUN(test_image_open_close);
    CTEST_RUN(test_block_write_read);
    CTEST_RUN(test_set_free);
    CTEST_RUN(test_find_free);
    CTEST_RUN(test_incore_find_free);
    CTEST_RUN(test_incore_find);
    CTEST_RUN(test_read_write_inode);
    CTEST_RUN(test_iget_iput);
    CTEST_RUN(test_ialloc);
    CTEST_RUN(test_alloc);
    CTEST_RUN(test_mkfs);
    CTEST_RUN(test_directory_open_close);
    CTEST_RUN(test_directory_get_root_entries);

    if (failures == 0) {
        printf("All tests passed\n");
    }

    return failures;
}