#include "inode.h"
#include "globals.h"
#include "blocks.h"

/*
    Nesse sistema, todos os inodes possíveis (80) são criados no início, uma lista encadeada dita quais estão livres
*/

void initializeInodes()
{
    /*
        Cria o arquivo responsável pelos inodes caso não exista, chama a função que carrega os inodes livres na lista global open inodes
    */
    FILE *file = fopen("fs/inodes.dat", "r+b");

    if (file == NULL)
    {
        file = fopen("fs/inodes.dat", "w+b");
        if (file == NULL)
        {
            printf("Erro ao abrir o arquivo de inodes");
            exit(1);
        }

        int max_inodes = sb.partitionSize / sb.blockSize;
        Inode inode;

        for (int i = 0; i < max_inodes; i++)
        {
            // Carrega metadados padrão de um inode

            inode.id = i;
            inode.type = TYPE_UNUSED;
            inode.size = 0;
            inode.block = -1;
            inode.indirect_block = -1;
            fwrite(&inode, sizeof(Inode), 1, file);
        }
    }

    fclose(file);

    openInodes = NULL;
    loadFreeInodes();
}


Inode createInode(unsigned char is_dir)   // aqui, 1 significa diretório e 0 significa arquivo
{
    if (openInodes == NULL)
    {
        printf("Erro: Nenhum inode livre disponivel!\n");
        exit(1);
    }

    Inode newInode = readInode(openInodes->id);

    openInode *used = openInodes;
    openInodes = openInodes->next;
    free(used);

    newInode.type = is_dir ? TYPE_DIR : TYPE_FILE;
    newInode.size = is_dir ? 2 * sizeof(DirEntry) : 0;
    newInode.block = nextBlock();
    newInode.indirect_block = -1;

    writeInode(newInode.id, newInode);

    if(newInode.id < sb.firstInode) {
        sb.firstInode = openInodes->id;
    }

    return newInode;
}

Inode readInode(int id)
{
    FILE *file = fopen("fs/inodes.dat", "rb");
    if (!file)
    {
        printf("Erro ao abrir inodes.dat");
        exit(1);
    }

    Inode inode;
    fseek(file, id * sizeof(Inode), SEEK_SET);
    fread(&inode, sizeof(Inode), 1, file);
    fclose(file);

    return inode;
}

void writeInode(int id, Inode newInfo)
{
    FILE *file = fopen("fs/inodes.dat", "r+b");
    fseek(file, id * sizeof(Inode), SEEK_SET);
    fwrite(&newInfo, sizeof(Inode), 1, file);
    fclose(file);
}

int loadFreeInodes()
{
    FILE *file = fopen("fs/inodes.dat", "rb");
    if (file == NULL)
    {
        printf("Arquivo de inodes não encontrado.\n");
        return 0;
    }

    fseek(file, sb.firstInode, SEEK_SET);

    int count = 0;
    Inode inode;

    while (openInodes != NULL)
    {
        openInode *temp = openInodes;
        openInodes = openInodes->next;
        free(temp);
    }

    while (fread(&inode, sizeof(Inode), 1, file) == 1)
    {
        if (inode.type == TYPE_UNUSED)
        {
            addOpenInode(inode.id);
            count++;
        }
    }

    fclose(file);
    return count;
}


static openInode * newOpenInode(int id)
{
    openInode *newNode = (openInode*) malloc(sizeof(openInode));
    newNode->id = id;
    newNode->next = NULL;
    return newNode;
}

void addOpenInode(int id)
{
    openInode *newNode = (openInode*) malloc(sizeof(openInode));
    if (newNode == NULL)
    {
        printf("Erro ao alocar memoria para openInode\n");
        return;
    }
    newNode->id = id;
    newNode->next = NULL;

    if (openInodes == NULL)
    {
        openInodes = newNode;
    }
    else
    {
        openInode *curr = openInodes;
        while (curr->next != NULL)
        {
            curr = curr->next;
        }
        curr->next = newNode;
    }
}
