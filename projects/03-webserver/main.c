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


// global variable; can't be avoided because
// of asynchronous signal interaction
int still_running = TRUE;
void signal_handler(int sig) {
    still_running = FALSE;
}

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t worker = PTHREAD_COND_INITIALIZER;
pthread_cond_t producer = PTHREAD_COND_INITIALIZER;
int nextin = 0;
int nextout = 0;
int requests = 0;




void usage(const char *progname) {
    fprintf(stderr, "usage: %s [-p port] [-t numthreads]\n", progname);
    fprintf(stderr, "\tport number defaults to 3000 if not specified.\n");
    fprintf(stderr, "\tnumber of threads is 1 by default.\n");
    exit(0);
}


void handlerequest(int socket){
    char *filename[1024];
    struct stat statresult
    int fofo = 0
    if (getrequest(socket, filename, 1024)){
        printf("failure\n");
    }
    else {
        fofo = stat(filename, &statresult);
        if(!(fofo)){//if the file exists, start scripting
            //twohundred
        }
        else{//heading for a 200 error
            //check error condition (EFAULT)
            //fourofour()
        
        
    
    
    printf("i have it under control ;)");
    return;
}


void *dowork(void *threadarg){
    int **sockets = (int** )threadarg;
    int numthreads = (sizeof(sockets))/(sizeof(int *));
    
    while(1){
        pthread_mutex_lock(&mutex);
        
        while (requests == 0){
            /*if (!still_running){
                //allow an exit hatch when done running.
                // putting the if first should allow for any unprocessed requests to be handled before the close 
                return NULL;
            }*/
            pthread_cond_wait(&worker, &mutex); 
        }
        int socket = *(sockets[nextout]);
        //printf("Socket: %d\n", socket);
        if (socket == -9999){
            pthread_mutex_unlock(&mutex); 
            return NULL;
        }
        handlerequest(socket);
        nextout = ( nextout + 1 ) % numthreads; // find numthreads from arg that is passed
        requests--;
        pthread_cond_signal(&producer);
        pthread_mutex_unlock(&mutex);
        }
    return NULL;
}


void runserver(int numthreads, unsigned short serverport) {


    //////////////////////////////////////////////////
    pthread_t workers[numthreads];//pool of workers
    int *socketnums[numthreads]; //pointer to ints, will use to pass socket numbers
    int i = 0;
    for(;i<numthreads;i++){
        int zero = 0;
        int *tmp = &zero;
        (socketnums[i]) = tmp;
        pthread_create(&workers[i], NULL, dowork, socketnums);
        
    }
    i = 0;
    // create your pool of threads here

    //////////////////////////////////////////////////
    
    
    int main_socket = prepare_server_socket(serverport);
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
            *(socketnums[nextin]) = new_sock;
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
    *(socketnums[nextout]) = -9999;//alert threads that we're closing
    i = 0;
    requests = numthreads+1;
    for(; i <numthreads;i++){//all threads should have exited
        pthread_cond_signal(&worker);
    }
    i = 0;
    for(; i <numthreads;i++){//pick up the scraps
        pthread_join(workers[i], NULL);
    }
    
    
    
        
    //temporary
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
