#ifndef SYSPRO2_FINDALLFILES_H
#define SYSPRO2_FINDALLFILES_H

#include "createFifos.h"
#include <stdbool.h>

#define MAX_PATHNAME_LEN 260
typedef struct ListNode{
    char input_file[MAX_PATHNAME_LEN];
    bool checked;
    struct ListNode* next;
}ListNode;
void findAllfiles(char *,ListNode**);
void printList(ListNode* );
void addNode(ListNode ** , char* );
void deleteAll(ListNode **);
void find_all_ids(char *,ListNode**,int);
bool find(ListNode* , char *);
int check_if_deleted(ListNode* ,char*,char*);
void deleteMirrorDirectory(char* );



#endif //SYSPRO2_FINDALLFILES_H
