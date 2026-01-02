#include "superblock.h"
#include "globals.h"

void initializeSuperBlock()
{
    FILE *file;

    file = fopen("fs/superblock.dat", "rb");

    if(file == NULL)
    {
        file = fopen("fs/superblock.dat", "wb");

        strcpy((&sb)->fileSystem, "pedroekauanfs");
        (&sb)->blockSize = BLOCKSIZE;
        (&sb)->partitionSize = PARTITION;
        (&sb)->nextBlock = 0;
        (&sb)->firstInode = 0;

        fwrite((&sb), sizeof(SuperBlock), 1, file);
        fclose(file);
    }
    else
    {
        fread((&sb), sizeof(SuperBlock), 1, file);
        fclose(file);
    }
}

void saveSuperBlock()
{
    FILE *file = fopen("fs/superblock.dat", "wb");
    if (file)
    {
        fwrite(&sb, sizeof(SuperBlock), 1, file);
        fclose(file);
    }
}
