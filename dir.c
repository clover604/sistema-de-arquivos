#include "dir.h"
#include "globals.h"
#include "inode.h"
#include "blocks.h"
#include "freespace.h"
#include "file.h"

int compareEntries(const void *a, const void *b)
{
    DirEntry *entryA = (DirEntry *)a;
    DirEntry *entryB = (DirEntry *)b;
    return strcmp(entryA->name, entryB->name);
}

void insertEntry(DirEntry newEntry)
{
    DirEntry *tmp = realloc(currentDir->entries, (currentDir->n_entries + 1) * sizeof(DirEntry));
    if (!tmp)
    {
        printf("Erro ao alocar memoria!\n");
        return;
    }

    currentDir->entries = tmp;
    currentDir->entries[currentDir->n_entries] = newEntry;
    currentDir->n_entries++;

    qsort(currentDir->entries, currentDir->n_entries, sizeof(DirEntry), compareEntries);
}

int findEntry(Directory *dir, char *name)
{
    int left = 0, right = dir->n_entries - 1;
    while (left <= right)
    {
        int mid = (left + right) / 2;
        int cmp = strcmp(dir->entries[mid].name, name);
        if (cmp == 0)
        {
            return mid;
        }
        else if (cmp < 0)
        {
            left = mid + 1;
        }
        else
        {
            right = mid - 1;
        }
    }
    return -1;
}

void createDir(char *name)
{
    if (findEntry(currentDir, name) != -1)
    {
        printf("Ja existe um diretorio com esse nome: %s\n", name);
        return;
    }

    FILE *file;

    Inode inode = createInode(1);
    DirEntry entry;

    strncpy(entry.name, name, sizeof(entry.name) - 1);
    entry.name[19] = '\0';

    entry.inode_id = inode.id;
    entry.is_dir = 1;

    Block b;
    memset(b.content, 0, BLOCKSIZE);

    DirEntry entrada[2];
    strcpy(entrada[0].name, ".");
    entrada[0].inode_id = inode.id;
    entrada[0].is_dir = 1;

    strcpy(entrada[1].name, "..");
    entrada[1].inode_id = currentDir->inode_id;
    entrada[1].is_dir = 1;

    memcpy(b.content, entrada, sizeof(entrada));

    char filename[30];
    sprintf(filename, "fs/blocks/%d.dat", inode.block);
    file = fopen(filename, "wb");
    fwrite(&b, sizeof(Block), 1, file);
    fclose(file);

    DirEntry *tmp = realloc(currentDir->entries,
                            (currentDir->n_entries + 1) * sizeof(DirEntry));
    if (!tmp)
    {
        printf("Erro ao alocar memoria!\n");
        return;
    }
    currentDir->entries = tmp;

    insertEntry(entry);

    saveDir(currentDir);

    printf("Diretorio '%s' criado.\n", name);
}

void saveDir()
{
    Inode inode = readInode(currentDir->inode_id);

    int capacity = BLOCKSIZE / sizeof(DirEntry);
    int total = currentDir->n_entries;

    Block b;
    memset(&b, 0, sizeof(Block));

    // Bloco direto
    int num_entries = (total < capacity) ? total : capacity;
    memcpy(b.content, currentDir->entries, num_entries * sizeof(DirEntry));
    writeOnBlock(b.content, inode.block);

    int entries_written = num_entries;

    int entries_left = total - entries_written;
    int ptr_count = BLOCKSIZE / sizeof(int);
    int pointers[ptr_count];
    memset(pointers, 0, sizeof(pointers));

    if (entries_left > 0)
    {
        if (inode.indirect_block >= 0)
        {
            char filename[30];
            sprintf(filename, "fs/blocks/%d.dat", inode.indirect_block);
            FILE *fp = fopen(filename, "rb");
            if (fp)
            {
                Block iblk;
                fread(&iblk, sizeof(Block), 1, fp);
                fclose(fp);
                memcpy(pointers, iblk.content, sizeof(pointers));
            }
        }
        else
        {
            inode.indirect_block = nextBlock();
        }

        int p = 0;
        while (entries_left > 0 && p < ptr_count)
        {
            int blk = pointers[p];
            if (blk <= 0)
            {
                blk = nextBlock();
                pointers[p] = blk;
            }

            memset(&b, 0, sizeof(Block));
            int to_write = (entries_left < capacity) ? entries_left : capacity;
            memcpy(b.content, currentDir->entries + entries_written, to_write * sizeof(DirEntry));
            writeOnBlock(b.content, blk);

            entries_written += to_write;
            entries_left -= to_write;
            p++;
        }

        Block iblk;
        memset(&iblk, 0, sizeof(Block));
        memcpy(iblk.content, pointers, sizeof(pointers));
        writeOnBlock(iblk.content, inode.indirect_block);
    }

    inode.size = currentDir->n_entries * sizeof(DirEntry);

    FILE *file = fopen("fs/inodes.dat", "r+b");
    fseek(file, inode.id * sizeof(Inode), SEEK_SET);
    fwrite(&inode, sizeof(Inode), 1, file);
    fclose(file);
}

Directory *loadDirectory(int inode_id, char *name, Directory *parent)
{
    Inode inode = readInode(inode_id);

    if (inode.type != TYPE_DIR) return NULL;

    Directory *dir = malloc(sizeof(Directory));
    dir->inode_id = inode_id;
    strcpy(dir->name, name);
    dir->parent = parent;

    int total_entries = inode.size / sizeof(DirEntry);
    DirEntry *all = malloc(total_entries * sizeof(DirEntry));


    if (total_entries == 0)
    {
        dir->entries = NULL;
        dir->n_entries = 0;
        return dir;
    }

    Block b;
    char filename[20];
    sprintf(filename, "fs/blocks/%d.dat", inode.block);
    FILE *file = fopen(filename, "rb");
    if (!file)
    {
        printf("Erro ao abrir bloco direto");
        free(all);
        free(dir);
        return NULL;
    }

    fread(&b, sizeof(Block), 1, file);
    fclose(file);

    int entries_per_block = BLOCKSIZE / sizeof(DirEntry);
    int entries_read = 0;

    int to_read = (total_entries < entries_per_block) ? total_entries : entries_per_block;
    memcpy(all, b.content, to_read * sizeof(DirEntry));
    entries_read += to_read;

    if (entries_read < total_entries && inode.indirect_block >= 0)
    {
        sprintf(filename, "fs/blocks/%d.dat", inode.indirect_block);
        file = fopen(filename, "rb");
        if (!file)
        {
            printf("Erro ao abrir blço indireto");
            free(all);
            free(dir);
            return NULL;
        }
        Block indirect_block;
        fread(&indirect_block, sizeof(Block), 1, file);
        fclose(file);

        int *indirect_blocks = (int*)indirect_block.content;
        int ptr_count = BLOCKSIZE / sizeof(int);

        for (int i = 0; i < ptr_count && entries_read < total_entries; i++)
        {
            if (indirect_blocks[i] == 0) break;

            sprintf(filename, "fs/blocks/%d.dat", indirect_blocks[i]);
            file = fopen(filename, "rb");
            if (!file)
            {
                printf("Erro ao abrir bloco de dados via indireto");
                continue;
            }
            Block data_block;
            fread(&data_block, sizeof(Block), 1, file);
            fclose(file);

            to_read = (total_entries - entries_read) < entries_per_block ?
                      (total_entries - entries_read) : entries_per_block;
            memcpy(all + entries_read, data_block.content, to_read * sizeof(DirEntry));
            entries_read += to_read;
        }
    }

    dir->entries = all;
    dir->n_entries = total_entries;
    return dir;
}

void showActualDir(Directory *dir)
{
    if (!dir) return;
    if (dir->parent == NULL)
    {
        printf("root/");
        return;
    }
    showActualDir(dir->parent);
    printf("%s/", dir->name);
}

void removeDir(Directory *dir, char *name)
{
    int i = findEntry(dir, name);

    if(i != -1 && dir->entries[i].is_dir)
    {
        char targetName[SIZEWORD];
        strncpy(targetName, dir->entries[i].name, SIZEWORD);
        targetName[SIZEWORD - 1] = '\0';


        Inode inode = readInode(dir->entries[i].inode_id);
        Directory *targetDir = loadDirectory(inode.id, name, dir);
        if (!targetDir)
        {
            printf("Erro ao carregar o diretório: %s\n", name);
            return;
        }

        for (int j = targetDir->n_entries - 1; j >= 0; j--)
        {
            if (strcmp(targetDir->entries[j].name, ".") == 0 ||
                strcmp(targetDir->entries[j].name, "..") == 0)
                continue;

            if (targetDir->entries[j].is_dir)
                removeDir(targetDir, targetDir->entries[j].name);
            else
                removeFile(targetDir, targetDir->entries[j].name);
        }


        saveDir(targetDir);

        setBitFS(inode.block);

        if (inode.block < sb.nextBlock)
        {
            sb.nextBlock = inode.block;

            saveSuperBlock();
        }

        if (inode.indirect_block >= 0)
        {

            setBitFS(inode.indirect_block);

            if (inode.indirect_block < sb.nextBlock)
            {
                sb.nextBlock = inode.indirect_block;

                saveSuperBlock();
            }
        }

        saveFreeSpaces();

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

        printf("Diretorio '%s' removido.\n", targetName);

        saveDir(dir);

        inode.type = TYPE_UNUSED;
        writeInode(inode.id, inode);
        addOpenInode(inode.id);

        if (inode.id < sb.firstInode) {
            sb.firstInode = inode.id;
        }

        saveSuperBlock();

        free(targetDir->entries);
        free(targetDir);
    }

    else
    {
        printf("Diretorio nao encontrado: %s\n", name);
    }
}
