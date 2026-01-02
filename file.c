#include "file.h"
#include "globals.h"
#include "inode.h"
#include "blocks.h"
#include "freespace.h"
#include "dir.h"

void writeFileContent(int inode_id, char *content)
{
    Inode inode = readInode(inode_id);
    int size = strlen(content);
    inode.size = size;

    int blocks_needed = (size + BLOCKSIZE - 1) / BLOCKSIZE;

    Block blk;
    memset(&blk.content, 0, BLOCKSIZE);

    if (size < BLOCKSIZE)
    {
        memcpy(blk.content, content, size);
    }
    else
    {
        memcpy(blk.content, content, BLOCKSIZE);
    }

    writeOnBlock(blk.content, inode.block);

    if (blocks_needed > 1)
    {
        int indirect_id = nextBlock();
        inode.indirect_block = indirect_id;

        int counter = blocks_needed - 1;
        int pointers[blocks_needed - 1];

        for (int i = 1; i < blocks_needed; i++)
        {
            pointers[i - 1] = nextBlock();
        }

        Block indirect_block;
        memset(&indirect_block.content, 0, BLOCKSIZE);
        memcpy(indirect_block.content, pointers, counter * sizeof(int));
        writeOnBlock(indirect_block.content, indirect_id);

        for (int i = 1; i < blocks_needed; i++)
        {
            int jump = i * BLOCKSIZE;
            memset(&blk.content, 0, BLOCKSIZE);
            int size_b = size - jump;
            if (size_b > BLOCKSIZE) size_b = BLOCKSIZE;
            memcpy(blk.content, content + jump, size_b);
            writeOnBlock(blk.content, pointers[i - 1]);
        }
    }

    writeInode(inode_id, inode);
}

void createFile(char *name, char *content)
{
    if (findEntry(currentDir, name) != -1)
    {
        printf("Ja existe um arquivo com esse nome: %s\n", name);
        return;
    }

    Inode inode = createInode(0);

    DirEntry entry;

    strncpy(entry.name, name, sizeof(entry.name) - 1);
    entry.name[19] = '\0';

    entry.inode_id = inode.id;
    entry.is_dir = 0;

    writeFileContent(inode.id, content);

    DirEntry *tmp = realloc(currentDir->entries,
                            (currentDir->n_entries + 1) * sizeof(DirEntry));
    if (!tmp)
    {
        printf("Erro ao alocar memoria\n");
        return;
    }
    currentDir->entries = tmp;

    insertEntry(entry);

    saveDir(currentDir);

    printf("Arquivo '%s' criado\n", name);
}

void catFile(char *name)
{
    int id = findEntry(currentDir, name);
    if (id == -1 || currentDir->entries[id].is_dir) {
        printf("Arquivo nao encontrado ou nao eh arquivo: %s\n", name);
        return;
    }

    int inode_id = currentDir->entries[id].inode_id;

    Inode inode = readInode(inode_id);
    char output[BLOCKSIZE + 1];

    FILE *file;
    Block b;
    char filename[30];
    sprintf(filename, "fs/blocks/%d.dat", inode.block);

    file = fopen(filename, "rb");
    fread(&b, sizeof(Block), 1, file);
    fclose(file);
    memcpy(output, b.content, BLOCKSIZE);

    output[BLOCKSIZE] = '\0';
    printf("%s", output);

    if (inode.indirect_block >= 0)
    {
        sprintf(filename, "fs/blocks/%d.dat", inode.indirect_block);
        file = fopen(filename, "rb");
        fread(&b, sizeof(Block), 1, file);
        fclose(file);

        int counter = BLOCKSIZE / sizeof(int);
        int *indirect_blocks = (int*)b.content;

        int total = inode.size - BLOCKSIZE;
        for (int i = 0; i < counter && total > 0; i++)
        {
            int blk = indirect_blocks[i];
            if (blk <= 0) break;

            sprintf(filename, "fs/blocks/%d.dat", blk);
            file = fopen(filename, "rb");
            fread(&b, sizeof(Block), 1, file);
            fclose(file);

            int size_b = (total < BLOCKSIZE) ? total : BLOCKSIZE;
            memcpy(output, b.content, size_b);
            output[size_b] = '\0';
            printf("%s", output);

            total -= size_b;
        }
    }

    printf("\n");
}

void removeFile(Directory *dir, char *name)
{
    int i = findEntry(dir, name);
    if(i != -1 && !dir->entries[i].is_dir)
    {
        Inode inode = readInode(dir->entries[i].inode_id);

        setBitFS(inode.block);

        if (inode.block < sb.nextBlock)
        {
            sb.nextBlock = inode.block;

            saveSuperBlock();
        }

        if (inode.indirect_block >= 0)
        {
            Block iblk;
            char filename[30];
            sprintf(filename, "fs/blocks/%d.dat", inode.indirect_block);
            FILE *fb = fopen(filename, "rb");

            fread(&iblk, sizeof(Block), 1, fb);
            fclose(fb);

            int ptr_count = BLOCKSIZE / sizeof(int);
            int *pointers = (int*)iblk.content;

            for (int j = 0; j < ptr_count; j++)
            {
                if (pointers[j] > 0)
                {
                    setBitFS(pointers[j]);
                }
            }

            setBitFS(inode.indirect_block);

        }

        saveFreeSpaces();

        printf("Arquivo '%s' removido.\n", name);

        for (int j = i; j < dir->n_entries - 1; j++)
        {
            dir->entries[j] = dir->entries[j + 1];
        }
        dir->n_entries--;

        DirEntry *tmp = realloc(dir->entries, dir->n_entries * sizeof(DirEntry));
        if (dir->n_entries > 0 || tmp != NULL)
        {
            dir->entries = tmp;
        }

        inode.type = TYPE_UNUSED;
        writeInode(inode.id, inode);
        addOpenInode(inode.id);

        if (inode.id < sb.firstInode) {
            sb.firstInode = inode.id;
        }

        saveDir(dir);


    }
    else
    {
        printf("Arquivo nao encontrado: %s\n", name);
    }
}
