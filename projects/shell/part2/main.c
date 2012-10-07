/*
 * project 1 (shell) main.c template 
 *
 * Sean Dalrymple  --phase 2
 * I met and did some prilminary design with Andrew Kephart 
 */

/* so phase 1 went relativley smoothly, as far as i can tell it is fully functional, and runs as one would expect. Phase 2 is a slightly different story the path variables are fully functioning and working. unfortantly I lacked the forsight to save an iteration of just the path features. the background features is a different sitiuation. they have come in with very dissapointing and inconsistent results. my idea was to create a linked list of all the commands that come in, these job nodes would be written by the parent process as the child executed them. I would then use poll and waitpid(-1,&rstatus,WNOHANG) to take away zombie process, match them with their linked list counterparts and remove both.by keeping an up to date process list we should be able to run the new commands by matching their proccess ids when necessary. while in design this seemed to work the implementation was much more difficult, as dropped members of my list and disappearing processes were/are common. in an effort to fix these problems, some of phase 1 developments have changed. 

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

typedef struct node {
        char name[128];
        struct node *next; 
} node;

typedef struct jobnode {
        char name[128];
        char state;
        pid_t id;
        struct jobnode *next; 

} jobnode;



char **tokenify (char *s);
int mode(char **s, int md);
void removeComment(char *s);
char ***parsecmd( char *s);
int exitShell(struct rusage start_usage);
int runSeq(char ***cmd, node *paths, jobnode *jobs);
int runPar (char ***cmd, node *paths, jobnode *jobs);
int runcmd( char **cmd, node *paths);
void clean(char ***s);
char* plusplus(char *s1, char *s2);

int showjobs(jobnode *j, int toprnt );
void addJobNode(pid_t pid, char state, char *opp, jobnode **head);
void removejobnode(jobnode *j, pid_t p);
void listclear(node *list);
void list_insert(char *name, struct node **head);
node *makepath(FILE *f,node *paths);
void resume(char *id, jobnode *j);
void freeze(char *id, jobnode *j);
int pollin(node *paths, jobnode **j, int cmdmode);


int 
main(int argc, char **argv) {
    struct rusage usage;
    //struct timeval startu, startsys; 
    getrusage( RUSAGE_SELF, &usage);
    const char SEQ = 0;
    node *paths = malloc(sizeof( node));//create a dummy node;
    strncpy(paths->name, "", 127);
    paths->next = NULL; 
    jobnode *j;
    j = malloc(sizeof( jobnode));//create a dummy node for a job list;
    strncpy(j->name, "#$#$", 127);
    j->id = 0;
    j->state = 1;
    j->next = NULL;
    /*pid_t a = 42;
    pid_t b = 432;
    pid_t c = 496;
    addJobNode(a,1,"haha",&j);
    addJobNode(a,1,"hahsa",&j);
    addJobNode(b,1,"haha",&j);
    addJobNode(c,1,"hahaa",&j);
    addJobNode(c,1,"hera",&j);
    addJobNode(a,1,"hisss",&j);
    //showjobs(j,1);
    //removejobnode(j,a);
    //removejobnode(j,b);
    //removejobnode(j,c);
    //printf("%s",j->name);
    showjobs(j,1);
    //showjobs(j,1);*/

    FILE *configs = fopen("shell-config", "r");
    if (configs){//creating file path
        paths = makepath(configs, paths);
        fclose(configs);
    }
    else {
        printf("file read failed: %s\n", strerror(errno));
    }
    

    int pol = pollin(paths,&j,SEQ);//run pollin (aka the program)
    if (pol == 2){//if it returns 2, handle as a normal exit
            listclear(paths);//clear list
            return exitShell(usage);//go through other exit procedures
        }
    return 0;//otherwise just return a zero

    
    
}

void clean(char ***s){
    int i = 0;
    while(s[i]){//free each array of strings
        free(s[i]);
        i++;
    }
    free(s);//free thyself
}

char **tokenify (char *s){
    /*parse whitespace*/
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
    /*mode command desicion tree */
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
    //removes comment//
    int len = strlen(s);
    int i = 0;
    for (; i<len;i++){
        if (s[i] == '#'){
            s[i] = '\0';
        }
    }
    return;
}


char *plusplus(char *s1, char *s2){
    //concat 2 strings//
    if(!(s1) || !(s2)){//if either NULL, you get null back
        return NULL; 
    }
    int len1 = strlen(s1);
    int len2 = strlen(s2);
    int i = 0;
    char *s3 = (char *) malloc(sizeof(char)*(len1+len2+1));
    for(; i<(len1+len2); i++){
        if (i < len1){
            s3[i] = s1[i];
        }
        else{
            s3[i] = s2[(i-len1)];
        }
    }
    s3[i] = '\0';
    return s3;
    }

char *findcmd(char *s, node *h){
    //check for any PATH extenstions//
    struct stat statresult;
    int rv = 0;
    char* concat = " ";
    
    rv = stat(s, &statresult);
    if(!(rv)){
            return s;//if the file exists return it
        }
    while(h){//otherwise start adding
        concat = plusplus((h->name),s);
        rv = stat(concat, &statresult);
        if(!(rv)){
            return concat;//if theres a match return it
        }
    free(concat);//mem managed   
    h = (h->next);
    }
    return s; //if nothing found, send back orignal so it can trckle down to exception
}
    
           

char ***parsecmd( char *s){
    //break command apart by semi colons//
    int len = strlen(s);
    int semis = 0;
    int i = 0;
    for (; i<len; i++){// count the semi colons
        if (s[i] == ';') {
            semis++;
        }
    }
    char **temp = (char **) malloc(sizeof(char*)*(semis+1));//temporary holder for semicoloned parsed strings
    char *holder;
    temp[0] = strtok(s,";");//explictly do first to avoid null exceptions
    i =1;
    while ( holder = (strtok(NULL,";"))){
        temp[i] = holder;
        i++;
    }
    char ***cmd = (char ***) malloc (sizeof(char**)*(semis+2));
    i= 0;
    for (; i<=semis; i++){
        cmd[i] = tokenify(temp[i]);//copy commands into 
    }
    cmd[semis +1] = NULL;//last value is always null
    free(temp);
    return cmd;
}


int runSeq(char*** cmd, node *paths, jobnode *jobs){
    int i = 0;
    int cmdmode = 0;
    while (cmd[i]){//go through each full comman
        if (cmd[i][0]){//if command not null
            if(!strcmp(cmd[i][0],"exit") ){//exit command handler
                cmdmode = 2;//return exit value
            }
            else if (!strcmp(cmd[i][0],"mode") ){//mode command handler
                cmdmode = mode(cmd[i], 0);//run mode descicion tree
                if (cmdmode == 1){//move to parallel
                    return runPar(&(cmd[(i+1)]), paths, jobs);//run rest of commands in parallel
                }
            } else{
                runcmd(cmd[i], paths);//run the commands sequentially
            }//end of non special cmd handles
        }//end of cmd != null
        i++;
    }//end while
    return cmdmode;
}


int runPar (char ***cmd, node *paths, jobnode *jobs){
    int i = 0;
    //pid_t exitmode = 0;
    //pid_t statemode = 0;
    int cmdmode = 1;
    char *appended = " ";
    for (;cmd[i];i++){//run through commands
        if (cmd[i][0]){// ignore if NULL
            appended = findcmd(cmd[i][0], paths);//append any path files
            cmd[i][0] = appended;//replace in command
            pid_t p = fork();//2 process
            if (p == 0) {//if your a kid
                 if(!(strcmp(cmd[i][0],"exit")) ) {//maybe your and exit                       
                    exit(0);//exit will be handled when dead process found     
                }
                if (!strcmp(cmd[i][0],"mode") ){//same for node
                    if(mode(cmd[i], 1)){
                        exit(1);
                    }
                    exit(0);        
                }
                else if (!strcmp(cmd[i][0],"jobs")){//show jobs then leave
                    showjobs(jobs, 1);
                    exit(0);
                } else if (!strcmp(cmd[i][0],"pause")) {//same for pause
                    if (cmd[i][1] && !(cmd[i][2])){
                        freeze(cmd[i][1], jobs);
                    } else {
                        printf("invalid parameters");
                    }
                       exit(0);
                }else if (!strcmp(cmd[i][0],"resume")) {//and same for resume
                    if (cmd[i][1] && !(cmd[i][2])){
                        resume(cmd[i][1], jobs);
                    } else {
                        printf("invalid parameters");
                    }
                    exit(0);
                } else if ((execv(cmd[i][0], cmd[i]) < 0)){//run command
                fprintf(stderr, "execv failed: %s\n", strerror(errno));//if thre is an error, let the user know
                
                }
            exit(0);//kill the child
            }
            else if (p>0) {//parent
            addJobNode(p, 1, cmd[i][0], &jobs);//just print your child
            //showjobs(jobs,1);   
            }//end parent
        }// end null check
    }//end for loop    
    return cmdmode;
}//end run par


int runcmd( char **cmd, node *paths){
       
    if (cmd[0]){//null check
        char *appended = malloc(sizeof(char)*1024);
        strcpy(appended,findcmd(cmd[0], paths));
        pid_t p = fork();
        if (p == 0) {
            /* in child */
            char *temp = cmd[0];
            cmd[0] = appended;//add path extention
            if ((execv(cmd[0], cmd) < 0)) {//and run
                fprintf(stderr, "execv failed: %s\n", strerror(errno));
                cmd[0]  = temp;
                exit(0);
                }
            }
         else if (p > 0) {
            /* in parent */
            int rstatus = 0;
            pid_t childp = wait(&rstatus);//wait for kid
            free(appended);
            assert(p == childp);//double check if right kid
            printf("Parent got carcass of child process %d, return val %d\n", childp, rstatus);
            } 
            else {
                /* fork had an error; bail out */
                fprintf(stderr, "fork failed: %s\n", strerror(errno));
                }


            }
    return 0;
}

int exitShell(struct rusage usage){
    //do timing and closing events//
    struct timeval endu, endsys;
    endu = usage.ru_utime;
    endsys = usage.ru_stime;
    printf("Program spent %ld.%ld seconds in user mode \n", endu.tv_sec, endu.tv_usec);
    printf("Program spent %ld.%ld seconds in kernal mode \n", endsys.tv_sec, endsys.tv_usec);
    printf("adios\n"); 
    exit(0);   
    return 0;
}
 
void listclear(node *list) {//clear out a list
        while (list != NULL) {
            node *tmp = list;
            list = list->next;
            free(tmp);
        }
    }    



node *makepath(FILE *f, node *path){//create the ll for a path variable
    char line[128];
    int len = 0;
    int i = 0;
    while (fgets(line, 127, f)) {
        len = strlen(line);
        if ((line[len-1])=='\n'){
            (line[len-1])='\0';
        }
        list_insert(line, &path); 
        i++;
    }
	if (i == 0){
        printf("no paths in file, be sure to enter full names");
    }
    return path;
}

void list_insert(char *name, node **head) {//insert into a ll
     node *newnode = malloc(sizeof( node));
     strncpy(newnode->name, name, 127);
     newnode->next = *head;
     *head = newnode;
} 

void addJobNode(pid_t pid, char state, char *opp, jobnode **head){//add a job
    //printf("adding a job");
    jobnode *temp = (*head);//head is the "anchor"
    while (temp->next != NULL){
        temp = temp->next;//move on down until you find anthe end
    }
    temp->next = malloc(sizeof( jobnode));//adding a new job node
    temp = temp->next;//you are know the end
    strncpy(temp->name, opp, 127);//copy info in...
    temp->id = pid;
    temp->state = state;
    temp->next = NULL;
    return;
    }
    

int showjobs(jobnode *j, int toprnt){//print jobs in ll, also returns a jobcount
//through the number of iterations it runs. because this list should always have an anchor node, real jobcount will be i - 1 (assuming correct implementation)
    int i = 0; 
    if(toprnt){
    printf("P id \t Command \t state\n");
    } 
    while ((j)){//run through ll
         if ((j->state) && toprnt ){
            printf("%d \t %s \t running\n", j->id, j->name);
         } else if (toprnt) {
            printf("%d \t %s \t paused\n", j->id, j->name);
        }
    j = j->next;
    i++;
    }
    return i;
}


     
         

void freeze(char* id, jobnode *j){//pause a proccess, we use pid b/c its a unique id
    int i = atoi(id);//conver id to an int
    while (strcmp((j->name),"#$#$")){//run through list
         if ((j->id) == i){//when there is a match, pause it
            j->state = 0;
            kill(j->id, SIGSTOP);
            return;
         } 
    j = j->next;
    }
    printf("Process %d not found",j->id);
    return;
}

void resume(char* id, jobnode *j){//same idea as freeze, now just resuming
    int i = atoi(id);
    while (strcmp((j->name),"#$#$")){
         if ((j->id) == i){
            kill(j->id, SIGCONT);
            return;
         } 
    j = j->next;
    }
    printf("Process %d not found",j->id);
    return;
}
    
void removejobnode(jobnode *j, pid_t p){
    //because we use an anchor, the first value of the list will never need to be removed bc its a dummy
    if (!(j->next)){//if at the end of the list, give up
        return;
    }  
    if (((j->next)->id) == (p)){//just point around it
        jobnode *temp = (j -> next);
        printf("removing, %s\n",temp->name);
        j->next= j->next->next;
        return;
    }
    return removejobnode(j,p);
}
    


int pollin(node *paths, jobnode **j, int cmdMode){
    pid_t dead = 0;
    char *prompt = "How can I help?\n> ";
    printf("%s", prompt);
    fflush(stdout);
    char buffer[1024];
    int bounce = 0;
    jobnode **temp = j;

while (1)
{
    temp = j;
    struct pollfd pfd = { 0, POLLIN }; // file descriptor is 0, I want to know when there are "IN" events pending
    int rv = poll(&pfd, 1, 1000);  
    int rstatus = 0;
    if (rv == 0) {
        //fprintf(stderr,"\%s/",(*temp)->name);
        bounce = 1;
        dead = waitpid(-1,&rstatus,WNOHANG);    
        if(dead>=1){
            while ( (*temp) && bounce){
                //fprintf(stderr,"\%s/",(*temp)->name);
                if (((*temp)->id)==dead){
                    bounce = 0;
                    fprintf(stderr,"child process %d ends (%s), returns the value: %d\n", dead, (*temp)->name, rstatus);
                    
		            if (!(strcmp((*temp)->name, "exit"))){//if the dead process was exit
                    	if (showjobs(*j,0)==1){//if there are no more active jobs
                        	removejobnode((*temp),dead);
                            return 2;//return the exit value
                   		}
                        printf("cant exit with jobs remaining");
                    }
                    
                    printf("%s\n",(*temp)->name);
                    if (!(strcmp(((*temp)->name), "mode"))){//if the dead process was mode
                        if (rstatus==0 ){// if the rstatus was zero, the result of the mode call was 0(ie sequential
                        
                            if (showjobs(*j,0)==1){//if there are no more active jobs
                        	    cmdMode = 0;//sets the value for seq
                   		    }
                            else{
                            printf("cant go sequential with unfinished processes remaining");      
                            }//else
                        }//rsatus
                        
                    }//if process was mode
                removejobnode(*j, dead);
                
                }//if we have a node mathching the dead childs id
                else{ (*temp) = (*temp)->next;}//otherwise keep looking
            }//if we have a dead child
        }//time out
    } else if (rv < 0) {
        fprintf(stderr, "fork failed: %s\n", strerror(errno));
        return -1; //send errorish return command
    } //end error handler 
    else {//getting input
        fgets(buffer, 1024, stdin);
        removeComment(buffer);//take out comments
        char***cmd = parsecmd(buffer);//break into a point to a pointer of strings
        if (cmdMode == 0){//run sequentially
            cmdMode = runSeq(cmd, paths, (*j));
            }//end run seq
        else{//run in parallel
            cmdMode = runPar(cmd, paths, (*j));
        }//end run par
        clean(cmd);//do some memory cleanup on command
        if (cmdMode == 2){
            return 2;//return exit value
        }
        
        printf("%s", prompt);//do it allover again
        fflush(stdout);
    }//end else

    //return 0;
        
    }//end while 1
   // return 0;
}//end pollin

