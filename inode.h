#ifndef INODE_H
#define INODE_H

#include "util.h"

void initializeInodes();
Inode createInode(unsigned char is_dir);
Inode readInode(int id);
void writeInode(int id, Inode newInfo);
int loadFreeInodes();
void addOpenInode(int id);

#endif /* inode_h */
