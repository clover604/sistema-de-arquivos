#include "util.h"
#include "globals.h"
#include "fs.h"
#include "dir.h"
#include "file.h"
#include "freespace.h"
#include "superblock.h"

int mkdir_(const char args[2][S_COMMAND])
{
    if (args[1][0] == '\0')
    {
        printf("Nome invalido!\n");
        return 0;
    }
    createDir(args[1]);
    return 1;
}

void cd(const char args[2][S_COMMAND])
{
    if(strcmp(args[1], "..") == 0)
    {
        if(currentDir->parent != NULL)
        {
            currentDir = currentDir->parent;
        }
    }
    else
    {
        int newDir = findEntry(currentDir, args[1]);
        if(newDir != -1 && currentDir->entries[newDir].is_dir)
        {
            DirEntry entry = currentDir->entries[newDir];
            currentDir = loadDirectory(entry.inode_id, entry.name, currentDir);
        }
        else
        {
            printf("Diretorio nao encontrado / nao eh diretorio: %s\n", args[1]);
        }
    }
}

void pwd()
{
    showActualDir(currentDir);
    printf("\n");
}

int touch(const char args[2][S_COMMAND])
{
    if (args[1][0] == '\0')
    {
        printf("Nome invalido!\n");
        return 0;
    }

    char contentFile[SIZEWORD];
    char line[SIZEWORD];

    int used = 0;
    contentFile[0] = '\0';

    while (fgets(line, sizeof(line), stdin) != NULL)
    {
        int len = strlen(line);

        if (used + len >= SIZEWORD - 1)
        {
            int available = (SIZEWORD - 1) - used;
            strncat(contentFile, line, available);

            cleanBuffer();

            break;
        }

        strcat(contentFile, line);
        used += len;
    }


    int len = strlen(contentFile);

    if (len > 0 && contentFile[len - 1] == '\n')
    {
        contentFile[len - 1] = '\0';
    }

    createFile(args[1], contentFile);
    return 1;
}

void cat(const char args[2][S_COMMAND])
{
    catFile(args[1]);
}

void rm(const char args[2][S_COMMAND])
{
    if (strcmp(args[1], ".")==0 || strcmp(args[1],"..")==0)
    {
        printf("Ta ficando maluco?\n");
    }
    else
    {
        int entry = findEntry(currentDir, args[1]);
        if (entry == -1)
        {
            printf("Arquivo / diretorio nao encontrado\n");
        }
        else if (currentDir->entries[entry].is_dir)
        {
            removeDir(currentDir, args[1]);
        }
        else
        {
            removeFile(currentDir, args[1]);
        }

    }
}

void ls()
{
    printf("%s\n", currentDir->name);
    for(int i = 0; i < currentDir->n_entries; i++)
    {
        printf("%s%s\n", currentDir->entries[i].name, currentDir->entries[i].is_dir ? "/" : "");
    }
}

void viewFreeSpace()
{
    for (int i = 0; i < MAX_FILES; i++)
    {
        printf("bloco %d: %d\n", i, getBitFS(i));
    }
}

void status()
{
    int free = 0;
    for (int i = 0; i < MAX_FILES; i++)
    {
        if (getBitFS(i)) free++;
    }

    printf("Tamanho livre: %d bytes\n", free * sb.blockSize);
    printf("Blocos livres: %d\n", free);
    printf("Tamanho do bloco: %d bytes\n", sb.blockSize);
}
