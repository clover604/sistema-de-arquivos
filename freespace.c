#include "freespace.h"
#include "globals.h"

void setBitFS(int x)
{
    int byteIndex = x / 8;
    int bitIndex = x % 8;
    freeSpaces[byteIndex] |= (1 << bitIndex);
}

void clearBitFS(int x)
{
    int byteIndex = x / 8;
    int bitIndex = x % 8;
    freeSpaces[byteIndex] &= ~(1 << bitIndex);
}


int getBitFS(int x)
{
    int byteIndex = x / 8;
    int bitIndex = x % 8;
    return (freeSpaces[byteIndex] >> bitIndex) & 1;
}


void initializeFreeSpaces()
{
    FILE *file = fopen("fs/freespace.dat", "rb");

    if(file == NULL)
    {
        memset(freeSpaces, 0xFF, FREE_SPACES_TAM);

        saveFreeSpaces();
    }
    else
    {
        fread(freeSpaces, sizeof(unsigned char), FREE_SPACES_TAM, file);
        fclose(file);
    }
}

void saveFreeSpaces()
{
    FILE *file = fopen("fs/freespace.dat", "wb");
    fwrite(freeSpaces, 1, FREE_SPACES_TAM, file);
    fclose(file);
}
