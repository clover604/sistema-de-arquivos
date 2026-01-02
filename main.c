#include "util.h"
#include "globals.h"
#include "fs.h"
#include "dir.h"
#include "file.h"
#include "freespace.h"
#include "superblock.h"

int main()
{

    // variáveis de interface
    char command[S_COMMAND];
    char args[2][S_COMMAND];

    boot();

    while (1)
    {
        for (int i = 0; i < 3; i++)
        {
            strcpy(&args[i], "");
        }

        printf("$ ");

        fgets(command, sizeof(command), stdin);

        int len = strlen(command);

        if (len > 0 && command[len - 1] != '\n') {
            cleanBuffer();
        }

        command[strlen(command)-1] = '\0';

        split(command, args);

        if(strcmp(args[0], "mkdir") == 0)
        {
            if(!mkdir_(args)) { // Se retornar 0 é porque o nome é inválido
                continue;
            }
        }

        if(strcmp(args[0], "cd") == 0)
        {
            cd(args);
        }

        if(strcmp(args[0], "pwd") == 0)
        {
            pwd();
        }

        if (strcmp(args[0], "touch") == 0)
        {
            if(!touch(args)) { // touch = 0 significa erro
                continue;
            }
        }


        if(strcmp(args[0], "cat") == 0)
        {
            cat(args);
        }
        if(strcmp(args[0], "rm") == 0)
        {
            rm(args);
        }

        if(strcmp(args[0], "ls") == 0)
        {
            ls();
        }

        if (strcmp(args[0], "freespace") == 0)
        {
            viewFreeSpace();
        }

        if(strcmp(args[0], "stat") == 0)
        {
            status();
        }

        if(strcmp(args[0], "exit") == 0)
        {
            free(currentDir);
            break;
        }
    }
    return 0;
}
