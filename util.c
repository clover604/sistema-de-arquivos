#include "util.h"

void cleanBuffer() {
    int ch;
    while ((ch = getchar()) != '\n' && ch != EOF);
}

void split(char command[], char args[][S_COMMAND])
{
    int size = 0;
    int i = 0;
    int start = 0;

    while (command[i] != '\0' && size < 2)
    {
        if (command[i] == ' ')
        {
            int tam = i - start;
            if (tam > S_COMMAND)
                tam = S_COMMAND;

            strncpy(args[size], &command[start], tam);
            args[size][tam] = '\0';

            size++;
            start = i + 1;
        }
        i++;
    }

    if (size < 2 && command[start] != '\0')
    {
        int tam = 0;
        while (command[start + tam] != '\0' &&
               command[start + tam] != ' ' &&
               command[start + tam] != '\n')
        {
            tam++;
        }

        if (tam > SIZEWORD)
            tam = SIZEWORD;

        strncpy(args[size], &command[start], tam);
        args[size][tam] = '\0';
        size++;
    }
}
