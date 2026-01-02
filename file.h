#ifndef FILE_H
#define FILE_H

#include "util.h"

void writeFileContent(int inode_id, char *content);
void createFile(char *name, char *content);
void catFile(char *name);
void removeFile(Directory *currentDir, char *name);

#endif /* file_h */
