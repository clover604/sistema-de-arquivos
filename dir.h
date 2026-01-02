#ifndef DIR_H
#define DIR_H

#include "util.h"


void saveDir();
void createDir(char *name);
Directory *loadDirectory(int inode_id, char *name, Directory *parent);
void insertEntry(DirEntry newEntry);
int findEntry(Directory *currentDir, char *name);
void showActualDir(Directory *dir);
void removeDir(Directory *dir, char *name);

#endif /* dir_h */
