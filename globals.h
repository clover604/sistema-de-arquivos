#ifndef GLOBALS_H
#define GLOBALS_H

#include "util.h"

extern SuperBlock sb;
extern openInode *openInodes;
extern Directory *currentDir;
extern unsigned char freeSpaces[FREE_SPACES_TAM];

#endif /* globals_h */
