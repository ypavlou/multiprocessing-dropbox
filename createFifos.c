#include "createFifos.h"
static int alarm_fired=0;
static int error_occured = 0;
static int previous = 0;
static int current =0;

void sigalarm_handler(int sig){
    kill(getppid(),SIGUSR1);    //send SIGUSR1 to parent
    exit(1);//dont continue reading
}
void sigusr_handler(int sig){
    alarm_fired =1;

}
void sigusr2_handler(int sig){
    error_occured = 1;
}

void createChildren(char* common,int id1,int id2,int bytes,char *mirror,char *logfile,char *input){
    pid_t  child_1_pid,child_2_pid;
    int pids[2];    //an int array to store the children's pids.
    pids[0] = pids[1] =-1;
    int flag = 0;
    struct sigaction sa1;

    sigfillset(&sa1.sa_mask);
    sa1.sa_flags = 0;
    sa1.sa_handler = sigusr_handler;

    sigaction(SIGUSR1, &sa1, NULL); //set sigusr1 signal

    struct sigaction sa2;

    sigfillset(&sa2.sa_mask);
    sa2.sa_flags = 0;
    sa2.sa_handler = sigusr2_handler;

    sigaction(SIGUSR2, &sa2, NULL);//set sigusr2 signal


    char fileName[MAX_PATHNAME_LEN];
    while(current < 3) {        //current keeps the number of attempts to fork both children
        if(current == previous && flag!=0 ) //if it's the fist repetition
            break;
        flag = 1;
        child_1_pid = fork();       //child no1

        if (child_1_pid == -1) {
            perror("Failed to fork");
            exit(1);
        }
        if (child_1_pid == 0) {       //its the child process

            createFifos(common, id1, id2);      //child1 creates  named pipe id1_to_id2.fifo
            char fifo[MAX_PATHNAME_LEN];
            sprintf(fifo, "%s/id%d_to_id%d.fifo", common, id1, id2);

            int fd = open(fifo, O_WRONLY);
            if (fd == -1) {
                perror("open");
                kill(getppid(),SIGUSR2);
                close(fd);
                exit(1);
            }
            int total_bytes = 0;    //for logfile information
            int total_files = 0 ;

            ListNode *input_files = NULL;
            findAllfiles(input,
                         &input_files);  //find all paths of input files of this client and store them at a linked list

            if (input_files != NULL) {    //if the client has input files
                ListNode *curr = input_files;
                while (curr != NULL) {
                    strcpy(fileName, curr->input_file);
                    total_files+=1;

                    writeShortIntFifo((short int) strlen(fileName) + 1, fd);
                    total_bytes+=(short int) strlen(fileName) + 1;
                    writeNameFifo(fd, fileName);
                    FILE *fp = fopen(fileName, "r");

                    if (fp == NULL) {
                        perror("open");
                        kill(getppid(),SIGUSR2);
                        fclose(fp);
                        exit(1);
                    }
                    fseek(fp, 0L, SEEK_END);
                    // calculating the size of the file
                    int res = ftell(fp);
                    total_bytes+=res;
                    fclose(fp);

                    if (res > bytes) {              //"split" the file in bytes of buffer size
                        int div = res / bytes;
                        int mod = res % bytes;
                        int i;
                        for (i = 0; i < div; i++) {
                            writeIntFifo(bytes, fd);
                            writeFifo(bytes, fifo, fileName, fd, i, bytes);
                        }
                        if (mod != 0) {
                            writeIntFifo(mod, fd);
                            writeFifo(mod, fifo, fileName, fd, i, bytes);
                        }
                        writeIntFifo(0, fd);    //all file is written so write 0 to the pipe
                    } else {
                        writeIntFifo(res, fd);
                        writeFifo(res, fifo, fileName, fd, 0, bytes);
                        writeIntFifo(0, fd);

                    }
                    curr = curr->next;
                }
                writeShortIntFifo(00, fd);
            }
            deleteAll(&input_files);

            close(fd);
            FILE* f = fopen(logfile, "a");
            if(f==NULL){
                perror("file");
                kill(getppid(),SIGUSR2);
                fclose(f);
                exit(1);
            }
            fprintf(f,"2:%d\n",total_bytes);
            fprintf(f,"4:%d\n",total_files);
            fclose(f);

            exit(0);

        } else {                      //its the parent

            if (pids[0] == -1) {//store the child's pid
                pids[0] = child_1_pid;
            } else if (pids[1] == -1) {
                pids[1] = child_1_pid;
            }

            child_2_pid = fork();       //child no2
            if (child_2_pid == -1) {
                perror("Failed to fork");
                exit(1);
            }
            if (child_2_pid == 0) {       //its the child process
                struct sigaction sa3;

                sigfillset(&sa3.sa_mask);
                sa3.sa_flags = 0;
                sa3.sa_handler = sigalarm_handler;

                sigaction(SIGALRM, &sa3, NULL);//set sigalarm signal

                createFifos(common, id2, id1);        //child2 creates  named pipe id2_to_id1.fifo
                char fifo[MAX_PATHNAME_LEN];
                sprintf(fifo, "%s/id%d_to_id%d.fifo", common, id2, id1);

                int fd = open(fifo, O_RDONLY);                  //open the .fifo pipe
                if (fd == -1) {
                    perror("open");
                    kill(getppid(),SIGUSR2);
                    close(fd);
                    exit(1);
                }

                alarm(30);
                short int size_of_name = readShortIntFifo(fifo, fd);        //get the bytes of the name of file
                alarm(0);
                int total_bytes = size_of_name;
                int total_files =0;
                while (size_of_name != 00) {
                    total_files+=1;
                    alarm(30);
                    char *file_name = readNameFifo(size_of_name, fifo, fd);    //get the name  of file
                    alarm(0);

                    alarm(30);
                    int size_of_file = readIntFifo(fifo, fd);//get the bytes of the file to read next
                    alarm(0);
                    total_bytes+=size_of_file;

                    while (size_of_file != 0) {
                        alarm(30);
                        readFifo(size_of_file, fifo, fd, mirror, file_name, id2);
                        alarm(0);

                        alarm(30);
                        size_of_file = readIntFifo(fifo, fd);
                        alarm(0);
                        total_bytes+=size_of_file;
                    }
                    free(file_name);

                    alarm(30);
                    size_of_name = readShortIntFifo(fifo, fd);
                    alarm(0);
                    total_bytes+=size_of_name;
                }

                close(fd);

                FILE* f = fopen(logfile, "a");
                if(f==NULL){
                    perror("file");
                    kill(getppid(),SIGUSR2);
                    fclose(f);
                    exit(1);
                }
                fprintf(f,"3:%d\n",total_bytes);
                fprintf(f,"5:%d\n",total_files);
                fclose(f);

                exit(0);

            } else {      //its the parent
                int r, returnStatus;
                if (pids[0] == -1) {//store the child's pid
                    pids[0] = child_2_pid;
                } else if (pids[1] == -1) {
                    pids[1] = child_2_pid;
                }
                r = waitpid(child_2_pid, &returnStatus, 0); //wait for child

                if (alarm_fired == 1) {
                    printf("Execution failed!No reading in 30 seconds.\n");
                    kill(pids[0], SIGKILL);  //kill both children processes
                    kill(pids[1], SIGKILL);
                    exit(EXIT_FAILURE);

                }
                if(error_occured == 1){
                    previous = current;
                    error_occured = 0;  //so the parent kills both children only 1 time
                    current++;
                    printf("Execution failed.Retrying reading and writing.\n");
                    kill(pids[0], SIGKILL);  //kill both children processes
                    kill(pids[1], SIGKILL);
                    pids[0] = pids[1] =-1;      //for the next repetition
                }

                if (r == child_2_pid) {
                    printf("%d reading from %d complete.\n", id1, id2);
                } else {
                    printf("%d reading from %d terminated with an error!.\n", id1, id2);
                }

            }
            int returnStatus;
            int r = waitpid(child_1_pid, &returnStatus, 0);//wait for child

            if(error_occured == 1){
                previous = current;
                error_occured = 0; //so the parent kills both children only 1 time
                current++;
                printf("Execution failed.Retrying reading and writing.\n");
                kill(pids[0], SIGKILL);  //kill both children processes
                kill(pids[1], SIGKILL);
                pids[0] = pids[1] =-1;

            }
            if (r == child_1_pid) {
                printf("Writing from %d to %d complete.\n", id1, id2);
            } else {
                printf("Writing from %d to %d terminated with an error!.\n", id1, id2);
            }

        }
    }
}

void createFifos(char* common,int id1,int id2){
    if(id1 != id2) {
        char fifo1_name[MAX_PATHNAME_LEN];
        sprintf(fifo1_name, "%s/id%d_to_id%d.fifo", common, id1, id2);   //create the path's name
        if (mkfifo(fifo1_name, PERMS) < 0) {
            if(errno!= EEXIST) {   //if fifo already exists ignore it
                perror("mkfifo");
                kill(getppid(),SIGUSR2);
                exit(1);
            }
        }

    }
}

int writeFifo(int bytes,char *fifo,char* input,int fd,int i,int buffer_size){
    char* buffer;
    buffer = malloc(bytes+1);
    if(buffer == NULL){
        perror("malloc");
        kill(getppid(),SIGUSR2);
        exit(1);
    }
    FILE* in;

    size_t n = 0;
    int c;
    in = fopen(input, "r");
    if(in == NULL){
        printf("Cannot find file at path %s\n",input);
        kill(getppid(),SIGUSR2);
        exit(1);
    }
    if(i != 0){
        fseek(in,buffer_size*i,SEEK_SET);               //if it's not the first write place the pointer at the next number of bytes of the file
    }
    while ((char)(c = fgetc(in)) != EOF && n != bytes ){   //read the file char by char
        buffer[n] = (char) c;           //store it
        n++;
    }


    buffer[n] = '\0';
    fclose(in);

    int w = write(fd, buffer, n);
    if(w!=n){
        perror("write");
        kill(getppid(),SIGUSR2);
        exit(1);
    }
    free(buffer);
    return n;
}
void writeShortIntFifo(short int bytes,int fd){
    short int temp = bytes;


    int w = write(fd, &temp, 2);
    if(w!=2){
        perror("write");
        kill(getppid(),SIGUSR2);
        exit(1);
    }

}
void writeIntFifo(int bytes,int fd){
    int temp = bytes;

    int w = write(fd, &temp, 4);
    if(w!=4){
        perror("write");
        kill(getppid(),SIGUSR2);
        exit(1);
    }
}

void writeNameFifo(int fd, char *name){

    char* temp = strdup(name);

    if(fd==-1) {
        perror("open");
        kill(getppid(),SIGUSR2);
        exit(1);
    }
    int w = write(fd, temp, strlen(temp)+1);
    if(w!=strlen(temp)+1){
        perror("write");
        kill(getppid(),SIGUSR2);
        exit(1);
    }


    free(temp);

}
void readFifo(int bytes, char *fifo,int fd,char *p,char* fileName,int id2){

    char* buffer = malloc((size_t)bytes+1);
    if(buffer == NULL){
        perror("malloc");
    }

    int in=read(fd, buffer, (size_t)bytes);

    if(in != bytes){
        perror("read bytes");
        kill(getppid(),SIGUSR2);
        exit(1);
    }

    buffer[in] = '\0';


    char path[MAX_PATHNAME_LEN];
    char needle[] = "input";
    char ret[MAX_PATHNAME_LEN];
    char temp[MAX_PATHNAME_LEN];
    char directory_path[MAX_PATHNAME_LEN];
    char mirror[MAX_PATHNAME_LEN];
    char t[MAX_PATHNAME_LEN];
    sprintf(mirror,"%s/%d",p,id2);
    if( mkdir(mirror,0777) && errno != EEXIST ) //create the directory /.../mirror/id2
        printf("error while trying to create %s",mirror );

    strcpy(ret, strstr(fileName,needle));      //take the string after "input" in path
    strcpy(t,strstr(ret,"/"));
    strcpy(ret,t);
    strcpy(temp,ret);
    char *token;

    token = strtok(temp, "/");
    sprintf(directory_path,"%s",mirror);
    while( token != NULL ) {
        if(strcmp(token,basename(fileName))!=0) { //create the full path of file in input dir
            sprintf(directory_path,"%s/%s",directory_path,token);

            if( mkdir(directory_path,0777) && errno != EEXIST ) //create the directory in mirror directory
                printf("error while trying to create %s",directory_path );
        }

        token = strtok(NULL, "/");
    }
    sprintf(path, "%s%s", mirror,ret);   //create the file's path in mirror dir
    FILE* fp = fopen(path, "a");
    if(fp==NULL){
        perror("file");
        kill(getppid(),SIGUSR2);
        fclose(fp);
        exit(1);
    }
    fputs(buffer,fp); //copy the content of file in the file in mirror dir
    fclose(fp);

    free(buffer);
}
short int readShortIntFifo(char *fifo,int fd){

    short int buffer;
    // Read from FIFO

    int in=read(fd, &buffer,(size_t) 2);
    if(in != 2){
        perror("read");
        kill(getppid(),SIGUSR2);
        exit(1);
    }

    return buffer;

}
int readIntFifo(char *fifo, int fd){
    int buffer;
    // Read from FIFO

    int in=read(fd, &buffer, (size_t)4);
    if(in != 4){
        perror("read");
        kill(getppid(),SIGUSR2);
        exit(1);
    }

    return buffer;
}

char* readNameFifo(int len,char *fifo,int fd){
    char *buffer;
    buffer = malloc((size_t)len+1);
    if(buffer == NULL){
        perror("malloc");
        kill(getppid(),SIGUSR2);
        exit(1);
    }
    // Read from FIFO

    int in=read(fd, buffer,(size_t) len);
    if(in != len){
        perror("read");
        kill(getppid(),SIGUSR2);
        exit(1);
    }

    buffer[in] = '\0';
    return buffer;
}