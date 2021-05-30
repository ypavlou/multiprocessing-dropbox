
#include "FindAllFiles.h"

void find_all_ids(char *name,ListNode** head,int id){                       //idea from : https://stackoverflow.com/questions/8436841/how-to-recursively-list-directories-in-c-on-linux
    DIR *dir;
    struct dirent *entry;
    char path[MAX_PATHNAME_LEN];
    char fullpath[MAX_PATHNAME_LEN];
    strcpy(fullpath,name);

    if (!(dir = opendir(name)))//if the directory does not exist
        return;

    while ((entry = readdir(dir)) != NULL) {//for every directory in this path
        if (entry->d_type == DT_DIR) {
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)//if the directory is current or parent's
                continue;
            sprintf(path, "%s/%s", name, entry->d_name);
            sprintf(fullpath,"%s/%s",name,entry->d_name);
            find_all_ids(path,head,id);
        } else {
            if(strstr(entry->d_name, ".id") != NULL) {
                sprintf(fullpath, "%s/%s", name, entry->d_name);
                char my_id[MAX_PATHNAME_LEN];
                sprintf(my_id, "%d.id", id);
                if (strcmp(entry->d_name, my_id) != 0 && find(*head, entry->d_name) == false) {
                    addNode(head, entry->d_name);
                }
            }

        }
    }
    closedir(dir);
}

void findAllfiles(char *name,ListNode** head){               //idea from : https://stackoverflow.com/questions/8436841/how-to-recursively-list-directories-in-c-on-linux
    DIR *dir;
    struct dirent *entry;
    char path[MAX_PATHNAME_LEN];
    char fullpath[MAX_PATHNAME_LEN];
    strcpy(fullpath,name);

    if (!(dir = opendir(name)))//if the directory does not exist
        return;

    while ((entry = readdir(dir)) != NULL) {//for every directory in this path
        if (entry->d_type == DT_DIR) {
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)//if the directory is current or parent's
                continue;
            sprintf(path, "%s/%s", name, entry->d_name);
            sprintf(fullpath,"%s/%s",name,entry->d_name);
            findAllfiles(path,head);
        } else {
            sprintf(fullpath,"%s/%s",name,entry->d_name);
            addNode(head,fullpath);
        }
    }
    closedir(dir);
}
void addNode(ListNode **head, char *path) {
    ListNode * current  = *head;
    if(current == NULL){    //adding at the head of the list
        current = malloc(sizeof(ListNode));
        if(current == NULL){
            perror("malloc");
            exit(-1);
        }
        current->next = NULL;
        strcpy(current->input_file,path);
        current->checked = false;
        *head = current;
        return;
    }

    //adding a node at the end of the list
    while(current->next!=NULL){
        current = current->next;
    }
    current->next = malloc(sizeof(ListNode));
    if(current->next == NULL){
        perror("malloc");
        exit(-1);
    }
    current->next->next = NULL;
    strcpy(current->next->input_file,path);
    current->next->checked = false;

}
void deleteAll(ListNode **head){
    ListNode* current = *head;
    ListNode* next;
    while (current != NULL){
        next = current->next;
        free(current);
        current = next;
    }
    *head = NULL;
}
void printList(ListNode* head){
    ListNode* current = head;
    while(current!=NULL){
        printf("%s\n",current->input_file);
        current = current->next;
    }
}
bool find(ListNode* head, char *path){
    ListNode* current = head;
    while(current!=NULL){
        if(strcmp(current->input_file,path)==0){
            return true;
        }
        current = current->next;
    }
    return false;
}

int check_if_deleted(ListNode* head,char*common,char*mirror){
    ListNode* temp = head;
    int gone=0;
    while (temp!=NULL){
        if(strcmp(temp->input_file,"gone")!=0) {
            char id_path[MAX_PATHNAME_LEN + 1];
            sprintf(id_path, "%s/%s", common, temp->input_file);
            if (access(id_path, F_OK) == -1) {              //check if the .id file doesn't  exist
                char mirror_path[MAX_PATHNAME_LEN];
                char *token;
                char *temp_id;
                temp_id = strdup(temp->input_file);
                token = strtok(temp_id, ".");               //get the id
                sprintf(mirror_path, "%s/%s", mirror, token);
                free(temp_id);
                deleteMirrorDirectory(mirror_path);
                strcpy(temp->input_file, "gone");       //replace the id with string "gone"
                gone+=1;
            }
        }
        temp = temp->next;
    }
    return gone;
}
void deleteMirrorDirectory(char* path){
    pid_t  child_pid;
    child_pid = fork();
    if (child_pid == -1){
        perror("Failed to fork");
        exit(1);
    }
    if(child_pid == 0) {       //its the child process
        execlp("rm", "rm", "-rf", path, NULL);
        exit(0);
    }else{ //parent
        int r,returnStatus;
        r = waitpid(child_pid, &returnStatus, 0);
        if (r == child_pid){
            printf("Directory at path :%s is deleted.\n",path);
        }
    }
}