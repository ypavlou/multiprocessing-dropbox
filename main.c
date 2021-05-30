#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

#include "createFifos.h"
#include "FindAllFiles.h"

#define MAX_PATHNAME_LEN 260
static int catch = 0;
void sigint_handler(int);

int main(int argc,char* argv[]) {
    int id=0,buffer_size=0;
    struct sigaction sa;

    sigfillset(&sa.sa_mask);
    sa.sa_flags = 0;
    sa.sa_handler = sigint_handler;

    sigaction(SIGINT, &sa, NULL);
    char common[MAX_PATHNAME_LEN],input[MAX_PATHNAME_LEN],mirror[MAX_PATHNAME_LEN],logfile[MAX_PATHNAME_LEN];
    if(argc > 13){
        printf("Command Execution Failed\n");
        exit(1);
    }
    else if(argc == 13){
        for(int i=1; i< argc; i+=2) {//read input from command line
            if (strcmp(argv[i], "-n") == 0) {
                id = atoi(argv[i+1]);

            } else if (strcmp(argv[i], "-c") == 0) {
                strcpy(common,argv[i+1]);

            } else if (strcmp(argv[i], "-i") == 0) {
                strcpy(input,argv[i+1]);

            } else if (strcmp(argv[i], "-m") == 0) {
                strcpy(mirror,argv[i+1]);
            } else if (strcmp(argv[i], "-b") == 0) {
               buffer_size = atoi(argv[i+1]);
            } else if (strcmp(argv[i], "-l") == 0) {
                strcpy(logfile,argv[i+1]);
            }
        }
    }
    else{
        printf("Command Execution Failed\n");
        exit(-1);
    }

    struct stat s;
    int err = stat(input, &s);      //check for input directory
    if(-1 == err) {
        if(ENOENT == errno) {
                //if the directory does not exist
            printf("Input directory %s does not exists\n",input);
            exit(1);
        } else {
            perror("stat");
            exit(1);
        }
    } else {
        if(S_ISDIR(s.st_mode)) {
                // it's a directory
        } else {
                //exists but is not a directory
            printf("Input %s is not a directory.\n",input);
        }
    }

    err = stat(mirror, &s);      //check for mirror directory
    if(-1 == err) {
        if(ENOENT == errno) {
                //if the directory does not exist
            if( mkdir(mirror,0777) ) {
                printf("error while trying to create '%s'\n", mirror);
                exit(1);
            }

        } else {
            perror("stat");
            exit(1);
        }
    } else {
        if(S_ISDIR(s.st_mode)) {
                // it's a directory
            printf("Mirror directory %s already exists\n",mirror);
            exit(1);
        }
    }
    err = stat(common, &s);      //check for common directory
    if(-1 == err) {
        if(ENOENT == errno) {
                //if the directory does not exist
            if( mkdir(common,0777) ) {
                printf("error while trying to create '%s'\n", common);
                exit(1);
            }
        } else {
            perror("stat");
            exit(1);
        }
    }

    char path[MAX_PATHNAME_LEN];
    sprintf(path, "%s/%d.id", common,id);   //create the path's name

    if( access( path, F_OK ) != -1 ) {              //check if the file exists
        printf("File %s already exists.\n",path);
        exit(-1);
    }
    FILE* fp = fopen(path, "w+");              //creates the file .id for write and read
    if(fp==NULL){
        perror("file");
    }
    char pid[10];
    snprintf(pid, 10,"%d",(int)getpid());
    fprintf(fp,"%s",pid);   //write the pid of this process at the .id file
    fclose(fp);


    FILE* f = fopen(logfile, "a");        //writes the id in the logfile
    if(f==NULL){
        perror("file");
    }
    fprintf(f,"1:%d\n",id);
    fclose(f);

    ListNode* ids_list=NULL;
    find_all_ids(common,&ids_list,id);
    int id2 = 0;
    int gone=0;
    find_all_ids(common,&ids_list,id);
    ListNode* temp = ids_list;
    while (1) {
        if(catch == 1)
            break;
        for (int i = 0; i < 10000; i++) {
            if(catch == 1)
                break;
            find_all_ids(common, &ids_list, id);
            gone+=check_if_deleted(ids_list, common, mirror);   //keep the number of the disconnected clients
            find_all_ids(common, &ids_list, id);
            temp = ids_list;
            while (temp != NULL) {
                if (temp->checked == false &&
                    strcmp(temp->input_file, "gone") != 0) {   //if the current id is not already checked
                    id2 = atoi(temp->input_file);
                    createChildren(common, id, id2, buffer_size, mirror, logfile, input);
                    temp->checked = true;

                }
                temp = temp->next;
            }

        }
    }


    deleteAll(&ids_list);
    deleteMirrorDirectory(mirror);
    char t[MAX_PATHNAME_LEN];
    sprintf(t,"%s/%d.id",common,id);
    remove(t); //delete .id file

    FILE* file = fopen(logfile, "a");        //writes how many clients left in logfile
    if(file==NULL){
        perror("file");
    }
    fprintf(file,"6:%d\n",gone);
    fclose(file);

    return 0;
}
void sigint_handler(int sig){
    catch=1;
}