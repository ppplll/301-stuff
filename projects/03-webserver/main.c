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

typedef struct threadargs{
    pthread_mutex_t *mutex;
    pthread_cond_t *worker;   
    pthread_cond_t *producer;
    int *nextin;
    int *nextout;
    int *requests;
    int numthreads;
    int socketnumber;
    struct sockaddr_in saddr;
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
    int charleft = 32000; //track unwritten file
    tofile[0] = '\0';
    while((charleft != 0) && (fgets(buff,charleft, fd))){
        charleft -= strlen(buff);
        strcat(tofile,buff);
    }
    return;
}
        

void addRs(char *tofile, char *fromfile,int len, FILE * fd){
    int i = 0;
    int linelen = 0;
    tofile[0] = '\0';
    for(;i<len;i++){
        linelen++;
        if (fromfile[i] == '\n'){
            strncat(tofile,fromfile,(linelen-1));
            linelen = 0;
            strcat(tofile,"\r\n");
        }
    }
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
    addRs(content,fileholder,strlen(fileholder), fd);
    char line3[50];
    sprintf(line3,l3temp,strlen(content));
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
	fputs(ctime(&now),log);
	fputs(" \"GET ",log);
	fputs(filename,log);
	fputs(" ",log);	
	if (error404)
		fputs("404 ",log);
	else
		fputs("200 ",log);
		
	fputs(buffer,log);
	fputs("\n",log);
	
	
	fclose(log);
    return;	
}


void handlerequest(int socket, struct sockaddr_in saddr){
    char filename[1024];
    //char dupe[1024];
    char *info;
    int error404 = 0;
    struct stat statresult;
    int fofo = 0;
    //int sum = socket*socket;
    if (getrequest(socket, filename, 1024)){
        printf("failure\n");
    }
    else {
        fofo = stat(filename, &statresult);
        //also stat filename w/o slash
        if(!(fofo)){//if the file exists, start scripting
            info = twohundred(filename, statresult);
        }
        else{
            fofo = stat(&(filename[1]), &statresult);
            if(!(fofo)){//if the file exists, start scripting
                info = twohundred(&(filename[1]), statresult);
            }
            else{
                info = fourofour();
                error404 = 1;
            }
        }
        senddata(socket, info, strlen(info)+1);
        logit( saddr, filename, info , error404);
        close(socket);
        free(info);

    }
    //printf("i have it under control ;)");
    return;
}


void *dowork(void *threadarg){    
    threadargs **blah = (threadargs ** )(threadarg);
    threadargs *sockets = *blah;
    pthread_mutex_t mutex = *((sockets[0]).mutex);
    while(1){
        pthread_mutex_lock(&mutex);
        int nextout = *(sockets[0].nextout);
        int numthreads = (sockets[nextout].numthreads);
        pthread_cond_t worker = *(sockets[nextout].worker);
        int socket = sockets[nextout].socketnumber;
        int requests = *(sockets[nextout].requests);
        while (requests == 0){
            pthread_cond_wait(&worker, &mutex); 
        }
        fprintf(stderr,"Socket: %d\n", socket);
        if (socket == -9999){
            pthread_mutex_unlock(&mutex); 
            return NULL;
        }
        handlerequest(socket, sockets[nextout].saddr);
        nextout = ( nextout + 1 ) % numthreads; // find numthreads from arg that is passed
        requests--;
        *(sockets[0].nextout) = nextout;
        *(sockets[0].requests) = requests;
        pthread_cond_t producer = *(sockets[nextout].producer);
        pthread_cond_signal(&producer);
        pthread_mutex_unlock(&mutex);
        }
    return NULL;
}


void runserver(int numthreads, unsigned short serverport) {

    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
    pthread_cond_t worker = PTHREAD_COND_INITIALIZER;
    pthread_cond_t producer = PTHREAD_COND_INITIALIZER;
    int nextin = 0;
    int nextout = 0;
    int requests = 0;
    //////////////////////////////////////////////////
    pthread_t workers[numthreads];//pool of workers
    threadargs targs[numthreads]; //pointer to ints, will use to pass socket numbers
    threadargs temparg;
    temparg.mutex = &mutex;
    temparg.worker = &worker;
    temparg.producer = &producer;
    temparg.nextin = &nextin;
    temparg.nextout = &nextout;
    temparg.requests = &requests;
    temparg.numthreads = numthreads;
    temparg.socketnumber = 0;
    struct sockaddr_in holdaddr;
    temparg.saddr = holdaddr;
    int i = 0;
    for(;i<numthreads;i++){
        (targs[i]) = temparg;        
    }
    i = 0;
    for(; i<numthreads;i++){
        pthread_create(&workers[i], NULL, dowork, &targs);
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
            pthread_mutex_lock(&mutex);            
            while (requests == numthreads){
                pthread_cond_wait(&producer, &mutex);
            }
            (targs[nextin]).socketnumber = new_sock;
            (targs[nextin]).saddr = client_address;
            nextin = (nextin +1) % numthreads;
            requests++;
            pthread_cond_signal(&worker);
            pthread_mutex_unlock(&mutex);
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
    for(;i<requests;i++){
        pthread_cond_signal(&worker);//clear out the rest of the requests
    } 
    (targs[nextout]).socketnumber = -9999;//alert threads that we're closing
    i = 0;
    requests = numthreads+1;
    for(; i <numthreads;i++){//all threads should have exited
        pthread_cond_signal(&worker);
    }
    i = 0;
    for(; i <numthreads;i++){//pick up the scraps
        pthread_join(workers[i], NULL);
    }
    
    
    
    
    fprintf(stderr, "Server shutting down.\n");
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

