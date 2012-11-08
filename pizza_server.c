/*
 * George Papanikolaou - Prokopis Gryllos
 * Operating Systems Project 2012 - Pizza Delivery
 * There is absolutely no warranty
 *
 * status1 --- status2:
 *    0           0   : Pending
 *    0           1   : Cooking
 *    1           0   : Cooked
 *    1           1   : Delivering
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <semaphore.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/un.h>
/* Pizza default values */ 
#define TIME_MARGARITA 10
#define TIME_PEPPERONI 12
#define TIME_SPECIAL 15
#define T_KONTA 5
#define T_MAKRIA 10
#define T_VERYLONG 50
#define N_DIANOMEIS 10
#define N_PSISTES 10
#define N_MAXPIZZA 3
/* System values: key(maybe a key_t?), pizza limit, listen queue */
#define SHM_KEY 7942
#define SEM_NAME "onoma"
#define LIMIT 200
#define QUEUE 5
#define PATH "socketfile"

/* Declaration of boolean type */
typedef enum { false, true } bool;

/* Struct for the pizza order */
typedef struct {
    int m_num;
    short p_num;
    short s_num;
    bool time;
    bool status1;
    bool status2;
} order_t;

/* A function to display an error message and then exit */
void fatal(char *message) {
    fprintf(stderr, "\a!! - Fatal error - ");
    fprintf(stderr, "%s\n", message);
    exit(EXIT_FAILURE);
}

/* An error checked malloc() wrapper function */
void *ec_malloc(unsigned int size) {
    void *ptr;
    ptr = malloc(size);
    if(ptr == NULL)
        fatal("in ec_malloc() on memory allocation");
    return ptr;
}

void handler(int sig_num) {
    /* handling the Signal of the client SIGUSR1 (10) */
}

void cook() {
    /* Cooker function */
}

void deliverer() {
    /* Delivery function */
}

int main() {
    pid_t pid;
    /* socket file descriptors */
    int sd, new_conn;
    /* shared memory id and an index for loops */
    int shm_id, i;
    /* temporary place for incoming data */
    order_t incoming;
    /* unix socket address declarations and lengths */
    struct sockaddr_un server_addr, client_addr;
    socklen_t addr_len;
    
    /* Shared memory size: order number limit + status buffers */
    int size = LIMIT * sizeof(order_t) + 20; 

    /* ================ END OF DECLARATIONS ===================*/

    /* Fork off the parent process to get into deamon mode */
    /* UNCOMMENT THIS and line 109 TO WORK */
    /* pid = fork(); */
    if (pid == -1)
        fatal("Can't fork parent");
    if (pid > 0) {
        /* Stopping the parent proccess and just keeping the child */
        printf(">> Deamon mode <<\n");
        /* exit(EXIT_SUCCESS); */
    }

    /* Semaphore business */
    sem_t *sem;
    sem = sem_open(SEM_NAME, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR, 1);
    if (sem == SEM_FAILED)
        fatal("could not create semaphore");

    /* Shared memory allocation */
    shm_id = shmget(SHM_KEY, size, 0600 | IPC_CREAT);
    if (shm_id == -1)
        fatal("in shared memory");
    bool* shm_start = shmat(shm_id, NULL, 0);
    if (shm_start == (bool *)-1)
        fatal("main could not attach to shared memory");
    /* Setting the pointers to shared memory */
    bool* cook_status = shm_start;
    bool* deliverer_status = shm_start + 10;
    order_t* order_list = (order_t*)shm_start + 20;
    for (i = 0; i < 20; i++) {
        *(shm_start + i) = false;
    }

    /* Socket business */
    sd = socket(PF_UNIX, SOCK_STREAM, 0);
    if (sd == -1)
        fatal("while creating server's socket");
    unlink(PATH);
    /* socket internal information --- Maybe: AF_LOCAL */
    server_addr.sun_family = AF_UNIX;
    strcpy(server_addr.sun_path, PATH);
    /* bind function call with typecasted arguments of server address */
    if (bind (sd, (struct sockaddr *) &server_addr, sizeof(server_addr)) == -1)
        fatal("while binding");
    if (listen(sd, QUEUE) == -1)
        fatal("while listening");

    /* endless loop to get new connections */
    while (1) {
        printf("Inside While (Debugging)\n");
        addr_len = sizeof(struct sockaddr_un);
        /* getting new connections from the client socket */
        new_conn = accept(sd, (struct sockaddr *) &client_addr, &addr_len);
        read(new_conn, &incoming, sizeof(incoming));
        /* close connection with this client */
        close(new_conn);
        printf("\n%d\n",incoming.m_num);
        *order_list = incoming;
        printf("%d\n",order_list->m_num);
        /* (--order_list)->m_num */

        /* getting the pointer to point to the new address */
        order_list++;
        /* pid = fork(); */
        if (pid == 0)
            break;
    }

    /* Child operates below */
	
    /* configure shared memory for every child proccess  */
    shm_id = shmget(SHM_KEY, size , 0600);
    if (shm_id == -1)          
        fatal("in shared memory");
    /* Do we need to attach ??? */
    /* bool* shm_start = shmat(shm_id, NULL, 0); */
    /* if (shm_start == (bool *)-1) */
        /* fatal("childs could not attach to shared memory"); */

    /* detaching from shared memory */
    if (shmdt(shm_start) == -1)
        fatal("order could not detach from shared memory");	
	
    /* terminate */
}
