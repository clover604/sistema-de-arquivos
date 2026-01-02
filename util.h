#ifndef UTIL_H
#define UTIL_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>

#define SIZEWORD 256
#define S_COMMAND 20
#define BLOCKSIZE 128
#define PARTITION 10240
#define MAX_FILES PARTITION / BLOCKSIZE
#define FREE_SPACES_TAM MAX_FILES / 8

#define TYPE_FILE 'f'
#define TYPE_DIR 'd'
#define TYPE_UNUSED 'u'


typedef struct
{
    char fileSystem[20];
    int blockSize;
    int partitionSize;
    int nextBlock;
    int firstInode;
} SuperBlock;

typedef struct
{
    int id;
    char type;
    int size;
    int block;
    int indirect_block;
} Inode;

typedef struct openInode
{
    int id;
    struct openInode *next;
}  openInode;

typedef struct
{
    char name[20];
    int inode_id;
    unsigned char is_dir;   // 0 = arquivo, 1 = diretório
} DirEntry;

typedef struct Directory
{
    char name[20];
    int inode_id;
    struct Directory *parent;

    DirEntry *entries;
    int n_entries;
} Directory;

typedef struct
{
    char content[BLOCKSIZE];
} Block;

void split(char command[], char args[][S_COMMAND]);

#endif /* util_h */
