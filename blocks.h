#ifndef BLOCKS_H
#define BLOCKS_H

#include "util.h"

void initializeBlocks();
int nextBlock();
void writeOnBlock(char *content, int id);

#endif /* blocks_h */
