
#ifndef SYSPRO2_CREATEFIFOS_H
#define SYSPRO2_CREATEFIFOS_H
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>
#include <dirent.h>
#include <libgen.h>
#include <fcntl.h>
#include "FindAllFiles.h"
#define PERMS 0666
#define MAX_PATHNAME_LEN 260

void createChildren(char*, int,int ,int,char*,char*,char*);
void createFifos(char*, int, int);
int writeFifo(int ,char *,char*, int,int,int);
void writeShortIntFifo(short int ,int);
void writeIntFifo(int ,int);
void writeNameFifo(int , char*);
void readFifo(int, char *,int, char*,char*,int);
char* readNameFifo(int ,char *,int);
short int readShortIntFifo(char *,int);
int readIntFifo(char *,int);
void sigalarm_handler(int );
void sigusr_handler(int );
void sigusr2_handler(int );

#endif //SYSPRO2_CREATEFIFOS_H
