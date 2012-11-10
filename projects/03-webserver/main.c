#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>
#include <string.h>
#include <signal.h>
#include <sys/stat.h>
#include <arpa/inet.h>

#include "network.h"

typedef struct __threadargs{
    pthread_mutex_t *mutex;
    pthread_cond_t *worker;   
    pthread_cond_t *producer;
    int *nextin;
    int *nextout;
    int *requests;
    int numthreads;
    int (**socketnumber);
    struct sockaddr_in (**saddr);
} threadargs;

// global variable; can't be avoided because
// of asynchronous signal interaction
int still_running = TRUE;
void signal_handler(int sig) {
    still_running = FALSE;
}


 


void usage(const char *progname) {
    fprintf(stderr, "usage: %s [-p port] [-t numthreads]\n", progname);
    fprintf(stderr, "\tport number defaults to 3000 if not specified.\n");
    fprintf(stderr, "\tnumber of threads is 1 by default.\n");
    exit(0);
}

/*
int countlines(char * file, int len){
    int linecount;
    int i = 0;
    for (;i<len; i++){
        if (file[i] == '\n'){
            linecount++;
        }
    }
    return linecount;
}
*/

void takeIn(char* tofile, FILE *fd){
    char buff[32000]; //excessive, but what if there are no newlines?
    int charleft = 31999; //track unwritten file
    int len;
    tofile[0] = '\0';
    while((fgets(buff,charleft, fd))){
        len = strlen(buff);
        charleft -= len;
        if (charleft < len){
            tofile[31999-charleft] = '\0';
            break;
        }
        strcat(tofile,buff);
    }
    
    return;
}
        

void addRs(char *tofile, char *fromfile){
    char *holder;
    holder = strtok(fromfile,"\n");
    tofile[0] = '\0';
    strcat(tofile,holder);
    strncat(tofile,"\r\n", 2);
    while((holder = (strtok(NULL,"\n")))){
        strcat(tofile,holder);
        strncat(tofile,"\r\n", 2);
    }
    strncat(tofile,"\r\n", 2);
    return;
}





char *twohundred(char *filename, struct stat statresult){
    char firstline[] = "HTTP/1.1 200 OK\r\n";
    char secondline[]= "Content-type: text/plain\r\n";
    char l3temp [50];
    strcpy(l3temp, "Content length: %d\r\n");
    char line4[] = "\r\n"; 
    FILE * fd = 0;
    fd = fopen(filename,"r");
    char fileholder[32000];
    takeIn(fileholder, fd);
    fclose(fd);
    char content[64000];
    fprintf(stderr,"%s",fileholder);
    addRs(content,fileholder);
    char line3[50];
    sprintf(line3,l3temp,(strlen(content)));
    char *returnval;
    returnval = (char*) malloc(sizeof(char)*64250);
    strcpy(returnval,firstline);
    strcat(returnval,secondline);
    strcat(returnval,line3);
    strcat(returnval,line4);
    strcat(returnval,content);
    strcat(returnval, "\r\n");
    return returnval;
}

    

    
    
    

char *fourofour(){
    char holder[] = "HTTP/1.1 404 Not Found\r\n\r\n";
    char *returnval = malloc(sizeof(char)*31);
    strcpy(returnval, holder);
    return returnval;
}
    
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void logit( struct sockaddr_in client_address, char * filename, char * responce, int error404)
{
	FILE * log;
	log=fopen("weblog.txt","a");
	
	time_t now;
   	time(&now);

	int length=strlen(responce);
	char *buffer2 = "%d";
    char buffer[128];
	sprintf(buffer,buffer2,length);

	fputs(inet_ntoa(client_address.sin_addr),log);
	fputs(" ",log);
    char temp1[128];
    sprintf(temp1,buffer2,ntohs(client_address.sin_port));
    fputs(temp1,log);
	fputs(" ",log);
    char *temp3 = ctime(&now);
    temp3[strlen(temp3)-1] = '\0';
	fputs(temp3,log);
	fputs(" \"GET ",log);
	fputs(filename,log);
	fputs("\" ",log);	
	if (error404)
		fputs("404 ",log);
	else
		fputs("200 ",log);
		
	fputs(buffer,log);
    fprintf(stderr,"abc: %s\n",responce);
	fputs("\n",log);
	fclose(log);
    return;	
}


void handlerequest(int socket, struct sockaddr_in saddr){
    char filename[1024];
    char *info;
    int error404 = 0;
    struct stat statresult;
    if (getrequest(socket, filename, 1023)){
        printf("failure\n");
    }
    else {
        if(filename[0]!='/'){//no slash
            if(!(stat(filename, &statresult))){   
                info = twohundred(filename, statresult);
            }
            else{
                info = fourofour();
                error404=1;
            }
        }
        else{
            if(!(stat(&(filename[1]), &statresult))){//if the file exist
                info = twohundred(&(filename[1]), statresult);
            }
            else{
                info = fourofour();
                error404 = 1;
            }
        }
        senddata(socket, info, strlen(info)+1);
        //fprintf(stderr,"%d", strlen(info));
        logit( saddr, filename, info , error404);
        close(socket);
        free(info);

    }
    //printf("i have it under control ;)");
    return;
}


void *dowork(void *threadarg){    
    threadargs *contain = (threadargs *) threadarg;
    pthread_mutex_t *mutex = contain->mutex;
    while(1){
        pthread_mutex_lock(mutex);
        int numthreads = (contain->numthreads);
        pthread_cond_t *worker = (contain->worker);
        int *requests = (contain->requests);
        while ((*requests) == 0){
            pthread_cond_wait(worker, mutex); 
        }
        int nextout = *(contain->nextout);
        int socket = (*(contain->socketnumber))[nextout];
        fprintf(stderr,"Socket: %d\n", socket);
        if (socket == -9999){
            pthread_mutex_unlock(mutex); 
            return NULL;
        }
        handlerequest(socket, (*(contain->saddr))[nextout]);
        nextout = ( nextout + 1 ) % numthreads; // find numthreads from arg that is passed
        (*(contain->socketnumber))[nextout] = nextout;
        (*requests) = (*requests)-1;
        pthread_cond_t *producer = (contain->producer);
        pthread_cond_signal(producer);
        pthread_mutex_unlock(mutex);
        }
    return NULL;
}


void runserver(int numthreads, unsigned short serverport) {

    pthread_mutex_t *mutex = malloc(sizeof(pthread_mutex_t));
	pthread_mutex_init(mutex,NULL);//USYNC_THREAD
    pthread_cond_t *worker = malloc(sizeof(pthread_cond_t));
	pthread_cond_init(worker, NULL);
    pthread_cond_t *producer = malloc(sizeof(pthread_cond_t));
	pthread_cond_init(producer, NULL);
    int *nextin = malloc(sizeof(int));
	*nextin = 0;
    int *nextout = malloc(sizeof(int));
	*nextout = 0;
    int *requests = malloc(sizeof(int));
	*requests = 0;
    //////////////////////////////////////////////////
    pthread_t workers[numthreads];//pool of workers
    int *sockets = malloc((sizeof(int))*numthreads);
    struct sockaddr_in *addrs = malloc(sizeof(struct sockaddr_in)*numthreads); //pointer to ints, will use to pass socket numbers
    threadargs temparg;
    temparg.mutex = mutex;
    temparg.worker = worker;
    temparg.producer = producer;
    temparg.nextin = nextin;
    temparg.nextout = nextout;
    temparg.requests = requests;
    temparg.numthreads = numthreads;
    struct sockaddr_in holdaddr;
    int i = 0;
    for(;i<numthreads;i++){
        sockets[i] = 0;
        addrs[i].sin_family = 0;
		addrs[i].sin_port = 0;
		addrs[i].sin_addr.s_addr = 0;
		int j = 0;
		for (;j<8;j++){
			addrs[i].sin_zero[j]='\0';
		}               
    }
    temparg.socketnumber = &sockets;
    temparg.saddr = &addrs;
    i = 0;
    for(; i<numthreads;i++){
        pthread_create(&workers[i], NULL, dowork, (void*)&temparg);
    }
    // create your pool of threads here

    //////////////////////////////////////////////////
    
    
    int main_socket = prepare_server_socket(serverport);
    //set up constant part of threadarguments//
    
    if (main_socket < 0) {
        exit(-1);
    }
    signal(SIGINT, signal_handler);

    struct sockaddr_in client_address;
    socklen_t addr_len;

    fprintf(stderr, "Server listening on port %d.  Going into request loop.\n", serverport);
    while (still_running) {
        struct pollfd pfd = {main_socket, POLLIN};
        int prv = poll(&pfd, 1, 10000);

        if (prv == 0) {
            continue;
        } else if (prv < 0) {
            PRINT_ERROR("poll");
            still_running = FALSE;
            continue;
        }
        
        addr_len = sizeof(client_address);
        memset(&client_address, 0, addr_len);

        int new_sock = accept(main_socket, (struct sockaddr *)&client_address, &addr_len);
        if (new_sock > 0) {
            
            fprintf(stderr, "Got connection from %s:%d\n", inet_ntoa(client_address.sin_addr), ntohs(client_address.sin_port));
            
           
            ////////////////////////////////////////////////////////
            pthread_mutex_lock(mutex);            
            while ((*requests) == numthreads){
                pthread_cond_wait(producer, mutex);
            }
            sockets[(*nextin)] = new_sock;
            addrs[(*nextin)] = client_address;
            *nextin = ((*nextin) +1) % numthreads;
            *requests = (*requests) +1;
            pthread_cond_signal(worker);
            pthread_mutex_unlock(mutex);
           /* You got a new connection.  Hand the connection off
            * to one of the threads in the pool to process the
            * request.
            *
            * Don't forget to close the socket (in the worker thread)
            * when you're done.
            */
           ////////////////////////////////////////////////////////


        }
    }
    //closeing threads -- eventually will be refactored into a function
    for(;i<(*requests);i++){
        pthread_cond_signal(worker);//clear out the rest of the requests
    } 
    sockets[(*nextout)] = -9999;//alert threads that we're closing
    i = 0;
    (*requests) = numthreads+1;
    for(; i <numthreads;i++){//all threads should have exited
        pthread_cond_signal(worker);
    }
    i = 0;
    for(; i <numthreads;i++){//pick up the scraps
        pthread_join(workers[i], NULL);
    }
    
    
    
    
    fprintf(stderr, "Server shutting down.\n");
	pthread_mutex_destroy(mutex);
	pthread_cond_destroy(worker);
	pthread_cond_destroy(producer);
    close(main_socket);
}


int main(int argc, char **argv) {
    unsigned short port = 3000;
    int num_threads = 1;

    int c;
    while (-1 != (c = getopt(argc, argv, "hp:"))) {
        switch(c) {
            case 'p':
                port = atoi(optarg);
                if (port < 1024) {
                    usage(argv[0]);
                }
                break;

            case 't':
                num_threads = atoi(optarg);
                if (num_threads < 1) {
                    usage(argv[0]);
                }
                break;
            case 'h':
            default:
                usage(argv[0]);
                break;
        }
    }

    runserver(10, port);
    
    fprintf(stderr, "Server done.\n");
    exit(0);
}

