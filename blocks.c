#include "blocks.h"
#include "globals.h"
#include "freespace.h"
#include "superblock.h"

void initializeBlocks()
{
    FILE *file;
    int blocks = sb.partitionSize / sb.blockSize;

    char filename[30];
    for (int i = 0; i < blocks; i++)
    {
        sprintf(filename, "fs/blocks/%d.dat", i);
        file = fopen(filename, "rb");
        if (file == NULL)
        {
            Block b;
            memset(&b.content, 0, BLOCKSIZE);

            if (i == 0)    // 0 é o bloco raiz / root
            {
                DirEntry rootEntries[2];

                strcpy(rootEntries[0].name, ".");
                rootEntries[0].inode_id = 0;
                rootEntries[0].is_dir = 1;

                strcpy(rootEntries[1].name, "..");
                rootEntries[1].inode_id = 0;
                rootEntries[1].is_dir = 1;

                memcpy(b.content, rootEntries, sizeof(rootEntries));
                printf("Criando bloco ROOT (%d)\n", i);
            }
            else
            {
                printf("Criando bloco %d\n", i);
            }

            file = fopen(filename, "wb");
            fwrite(&b, 1, BLOCKSIZE, file);
            fclose(file);
        }
        else
        {
            fclose(file);
        }
    }
}

int nextBlock()
{
    int blockId = sb.nextBlock;

    if (blockId < 0 || blockId >= MAX_FILES || !getBitFS(blockId))
    {
        blockId = -1;
        for (int i = 0; i < MAX_FILES; i++)
        {
            if (getBitFS(i))
            {
                blockId = i;
                break;
            }
        }
        if (blockId == -1)
        {
            printf("Nenhum bloco livre disponivel!\n");
            return -1;
        }
    }

    clearBitFS(blockId);

    sb.nextBlock = -1;
    for (int i = blockId + 1; i < MAX_FILES; i++)
    {
        if (getBitFS(i))
        {
            sb.nextBlock = i;
            break;
        }
    }

    saveSuperBlock();
    saveFreeSpaces();
    return blockId;
}

void writeOnBlock(char *content, int id)
{
    FILE *file;
    Block b;

    memcpy(b.content, content, BLOCKSIZE);

    char filename[30];
    sprintf(filename, "fs/blocks/%d.dat", id);

    file = fopen(filename, "wb");

    if (file == NULL)
    {
        printf("Erro ao abrir o arquivo para escrita");
        exit(1);
    }

    fwrite(&b, sizeof(Block), 1, file);

    fclose(file);
}
