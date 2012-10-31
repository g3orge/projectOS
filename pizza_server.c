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

#define TIME_MARGARITA 10
#define TIME_PEPPERONI 12
#define TIME_SPECIAL 15
#define T_KONTA 5
#define T_MAKRIA 10
#define T_VERYLONG 50
#define N_DIANOMEIS 10
#define N_PSISTES 10
#define N_MAXPIZZA 3

#define SHM_KEY "PiePizza"
/* TODO: shared memory segment size */
#define SHM_SIZE 4*1024
#define QUEUE 5
#define PATH "socketfile"

/* Declaration of boolean type */
typedef enum { false, true } bool;

/* Struct for the pizza order */
typedef struct {
    int m_num;
    short p_num;
    short s_num;
    int time;
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
    int sd, new_conn, shm_id, i;
    int ord_len = sizeof(order_t);
    char temp[ord_len];
    /* unix socket address declarations and lengths */
    sockaddr_un server_addr, client_addr;
    socklet_t addr_len;

    /* Fork off the parent process to get into deamon mode */
    pid = fork();
    if (pid == -1)
        fatal("Can't fork parent");
    if (pid > 0) {
        /* Stopping the parent proccess and just keeping the child */
        printf(">> Deamon mode <<\n");
        exit(EXIT_SUCCESS);
    }

    /* Shared memory allocation */
    shm_id = shmget(SHM_KEY, SHM_SIZE, 0600 | IPC_CREAT);
    if (shm_id == -1)
        fatal("in shared memory");
    int* shm_start = shmat(shm_id, NULL, 0);
    if (shm_start == (char *)-1)
        fatal("main could not attach to shared memory");
    /* Setting the pointers to memory */
    bool* cook_status = shm_start;
    bool* deliverer_status = shm_start + 10;
    order_t* order_list = shm_start + 20;
    for (i = 0; i < 20; i++) {
        *(shm_start + i) = false;
    }

    /* Socket business */
    sd = socket(PF_UNIX, SOCK_STREAM, 0);
    if (sd == -1)
        fatal("while creating socket");
    unlink(PATH);
    /* socket internal information --- Maybe: AF_LOCAL */
    server_addr.sun_family = AF_UNIX;
    strcpy(server_addr.sun_path, PATH);
    /* bind function call with typecasted arguments of server address */
    if (bind (sd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
        fatal("while binding");
    if (listen(sd, QUEUE) == -1)
        fatal("while listening");
    /* endless loop to get new connections */
    while (1) {
        addr_len = sizeof(struct sockaddr_un);
        new_conn = accept(sd, (struct sockaddr *)&client_addr, &addr_len);
        read(new_conn, &temp, sizeof(temp));
        close(new_conn);
        pid = fork()
        if (pid == 0)
            break;
    }

    /* Child operates below */
}
