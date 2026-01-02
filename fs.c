#include "fs.h"
#include "globals.h"
#include "superblock.h"
#include "blocks.h"
#include "freespace.h"
#include "inode.h"

#include <direct.h>

/*
    programa responsável por iniciar o sistema, chama as funções de inicialização de cada estrutura (inode, sb, blocos, free sspace)
*/

void boot()
{
    _mkdir("fs");
    _mkdir("fs/blocks");

    initializeSuperBlock();
    initializeBlocks();
    initializeFreeSpaces();

    initializeInodes();

    if (readInode(0).type == TYPE_UNUSED)
    {
        Inode root = createInode(1);
        root.size = 2 * sizeof(DirEntry);
        root.block = 0;
        root.type = TYPE_DIR;

        writeInode(0, root);

        printf("Inode ROOT criado (id=%d, bloco=%d)\n", root.id, root.block);
    }

    Directory *start = loadDirectory(0, "/", NULL);

    currentDir = start;
}
