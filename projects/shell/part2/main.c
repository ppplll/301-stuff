/*
 * project 1 (shell) main.c template 
 *
 * project partner names and other information up here, please
 *
 */

/* you probably won't need any other header files for this project */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <ctype.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <poll.h>
#include <signal.h>
#include <errno.h>
#include <assert.h>

char **tokenify (char *s);
//char isSpecial(char *s); //checks if command is "special".
//void exit(char **s);
int mode(char **s, int md);
void removeComment(char *s);
char ***parsecmd( char *s);
void exitShell(struct rusage start_usage);
int runSeq(char*** cmd);
int runPar (char ***cmd);
int runcmd( char **cmd);
void clean(char ***s);


int 
main(int argc, char **argv) {
    struct rusage usage;
    //struct timeval startu, startsys; 
    getrusage( RUSAGE_SELF, &usage);
    const char SEQ = 0;
    char *prompt = "I'm Mark Wahlberg, what can i do for ya?\n> ";
    printf("%s", prompt);
    fflush(stdout);
    char cmdMode = SEQ;
    char buffer[1024];

    while (fgets(buffer, 1024, stdin) != NULL) {
        /* process current command line in buffer */
        /* just a hard-coded command here right now */
        
        removeComment(buffer);
        char ***cmd = parsecmd(buffer);
        if (cmdMode == SEQ){
            cmdMode = runSeq(cmd);
            }
        else{
            cmdMode = runPar(cmd);
        }
        clean(cmd);
        if (cmdMode == 2){
            exitShell(usage);
        }
        printf("%s", prompt);
        fflush(stdout);
    }

    return 0;
}

void clean(char ***s){
    int i = 0;
    while(s[i]){
        free(s[i]);
        i++;
    }
    free(s);
}

char **tokenify (char *s){
     if (s == NULL){
          return NULL;
     }
     int letstate = 0;
     int wordcount = 0;
     int len = strlen(s);
     int i = 0;
     for(; i<len; i++){//go through and count the number of words
          if ( (!(isspace(s[i]))) && (!letstate) ){//start of a word
               wordcount++;
               letstate++; 
          }
          if ( (isspace(s[i]) || s[i] == '\0') && letstate){//end of a word
               letstate = 0;
          }
     }
     char **tkfy  = (char **) malloc(sizeof(char*)*(wordcount+1));
     letstate = 0;
     char* holder = " ";
     tkfy[0] = strtok(s," \t\n");
     i =1;
     while ( holder = (strtok(NULL," \t\n"))){
        tkfy[i] = holder;
        i++;
     }
     tkfy[i] = NULL;
     return tkfy;
}

int mode( char **cmd, int md){
    if (cmd[2]){
        printf( "unknown command %s, no mode changes made\n", cmd[2]);
        return md;
    }
    else{
    if (!cmd[1]){
        if (md==0){
            printf("we are currently in sequential mode\n");
            return md;
        }
        printf("we are currently in parallel mode\n");
            return md;
        }
    else {
        if (!strcmp(cmd[1],"s") || !strcmp(cmd[1],"seq") || !strcmp(cmd[1],"sequential")){
            printf("switching into sequential mode\n");
            return 0 ;
        } else if (!strcmp(cmd[1],"p") || !strcmp(cmd[1],"par") || !strcmp(cmd[1],"parallel")){
            printf("switching into parallel mode\n");
            return 1 ;
        } else {
            printf ("unreconized command %s\n", cmd[1]);
            return md;
            }
        }
    }
} 


void removeComment(char *s){
    int len = strlen(s);
    int i = 0;
    for (; i<len;i++){
        if (s[i] == '#'){
            s[i] = '\0';
        }
    }
    return;
}


char ***parsecmd( char *s){
    int len = strlen(s);
    int semis = 0;
    int i = 0;
    for (; i<len; i++){// count the semi colons
        if (s[i] == ';') {
            semis++;
        }
    }
    char **temp = (char **) malloc(sizeof(char*)*(semis+1));
    char *holder;
    temp[0] = strtok(s,";");
    i =1;
    while ( holder = (strtok(NULL,";"))){
        temp[i] = holder;
        i++;
    }
    char ***cmd = (char ***) malloc (sizeof(char**)*(semis+2));
    i= 0;
    for (; i<=semis; i++){
        cmd[i] = tokenify(temp[i]);
    }
    cmd[semis +1] = NULL;
    free(temp);
    return cmd;
}


int runSeq(char*** cmd){
    int i = 0;
    int cmdmode = 0;
    while (cmd[i]){
        if (cmd[i][0]){
            if(!strcmp(cmd[i][0],"exit") ){
                cmdmode = 2;
            }
            else if (!strcmp(cmd[i][0],"mode") ){
                cmdmode = mode(cmd[i], 0);
                if (cmdmode == 1){
                    return runPar(&(cmd[(i+1)]));
                }
            } else{
                runcmd(cmd[i]);
            }
        }
        i++;
    }
    return cmdmode;
}
        

int runPar (char ***cmd){
    int i = 0;
    int cmdmode = 1;
    for (;cmd[i];i++){//run through commands
        if (cmd[i][0]){// ignore if NULL
            pid_t p = fork();
            if (p == 0) {//run non special commands in child
                if ((strcmp(cmd[i][0],"exit")) && (strcmp(cmd[i][0],"mode")) && (execv(cmd[i][0], cmd[i]) < 0)){//using c's short circuiting to my advantage
                fprintf(stderr, "execv failed: %s\n", strerror(errno));
                }
            exit(0);//kill the child
            }//the following only operates on parent
            if(!strcmp(cmd[i][0],"exit") ){
                        cmdmode = 2;
            }
            else if (!strcmp(cmd[i][0],"mode") ){//the last mode call will be used to set the mode, which will occur after all other processes are finished
                        cmdmode = mode(cmd[i], 1);        
            }
        }
    }
    int rstatus = 0;
    int j = 0;
    while (j<i){
        waitpid(-1,&rstatus,0);
        //printf("waiting is the hardest part");
    j++;
    }
    return cmdmode;
}


int runcmd( char **cmd){
    if (cmd[0]){
        pid_t p = fork();
        if (p == 0) {
            /* in child */
            if ((execv(cmd[0], cmd) < 0)) {
                fprintf(stderr, "execv failed: %s\n", strerror(errno));
                exit(0);
                }
            }
         else if (p > 0) {
            /* in parent */
            int rstatus = 0;
            pid_t childp = wait(&rstatus);
            /* for this simple starter code, the only child process we should
               "wait" for is the one we just spun off, so check that we got the
               same process id */ 
            assert(p == childp);
            printf("Parent got carcass of child process %d, return val %d\n", childp, rstatus);
            } else {
                /* fork had an error; bail out */
                fprintf(stderr, "fork failed: %s\n", strerror(errno));
                }


            }
    return 0;
}

void exitShell(struct rusage usage){
    struct timeval endu, endsys;
    endu = usage.ru_utime;
    endsys = usage.ru_stime;
    printf("Program spent %ld.%ld seconds in user mode \n", endu.tv_sec, endu.tv_usec);
    printf("Program spent %ld.%ld seconds in kernal mode \n", endsys.tv_sec, endsys.tv_usec);
    printf("Say hi to ya mother for me\n");    
    exit(0);
    return;
}
      
 

