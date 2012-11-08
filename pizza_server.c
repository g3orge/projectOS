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
    short m_num;
    short p_num;
    short s_num;
    bool time;
    bool status1;
    bool status2;
} order_t;

void fatal(char *message) {
    /* A function to display an error message and then exit */
    /* Maybe put it in a logfile so it can be accessible from everywhere */
    fprintf(stderr, "\a!! - Fatal error - ");
    fprintf(stderr, "%s\n", message);
    exit(EXIT_FAILURE);
}

void handler(int sig_num) {
    /* A simple handler maybe for stats */
    fprintf(stderr,"\nCathing Terminate Signal");
    exit(EXIT_SUCCESS);
}

void cook(char t) {
    /* The waiting function for the cooking */
    if (t == 'm')
        usleep(TIME_MARGARITA);
    else if (t == 'p')
        usleep(TIME_PEPPERONI);
    else if (t == 's')
        usleep(TIME_SPECIAL);
    else
        fatal("Wrong input on cook function");
    /* The cook functions does not change the statuses of the orders */
}

void deliverer(char d) {
    /* The waiting function for the delivery */
    if (d == 's')
        usleep(T_KONTA);
    else if (d == 'l')
        usleep(T_MAKRIA);
    else
        fatal("Wrong input on delivery function");
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
    int size = LIMIT * sizeof(order_t) + N_DIANOMEIS + N_PSISTES; 

    /* ================ END OF DECLARATIONS ===================*/

    /* ctrl-C signal handler */
    signal(SIGINT, handler);

    /* Fork off the parent process to get into deamon mode */
    /* UNCOMMENT THIS TO WORK */
    /* pid = fork(); */
    if (pid == -1)
        fatal("Can't fork parent");
    if (pid > 0) {
        /* Stopping the parent proccess and just keeping the child */
        printf(">> Deamon mode <<\n");
        /* exit(EXIT_SUCCESS); */
    }

    /* Semaphore business */
    sem_t *mutex;
    mutex = sem_open(SEM_NAME, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR, 1);
    if (mutex == SEM_FAILED)
        fatal("could not create semaphore");

    /* Shared memory allocation */
    shm_id = shmget(SHM_KEY, size, 0600 | IPC_CREAT);
    if (shm_id == -1)
        fatal("in shared memory");
    bool* shm_start = shmat(shm_id, NULL, 0);
    if (shm_start == (bool *)-1)
        fatal("main could not attach to shared memory");
    /* pointer to the start of the list by shifting */
    order_t* order_list = (order_t*)shm_start + N_DIANOMEIS + N_PSISTES;
    /* initialization. here?? */
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

    /* Childs operate below */

    /* getting the pid to avoid the mess with the if-s */
    pid = getpid();
    /* The chosen cooker id. defaults to -1: not found */
    int chosen = -1;
    char pizza_type = 'n';
	
    /* configure shared memory for every child proccess  */
    shm_id = shmget(SHM_KEY, size , 0600);
    if (shm_id == -1)          
        fatal("in shared memory");
    bool* shm_begin = shmat(shm_id, NULL, 0);
    if (shm_begin == (bool *)-1)
        fatal("childs could not attach to shared memory");
    /* Configuring the pointers to shared memory */
    bool* cook_status = shm_begin;
    bool* deliverer_status = shm_begin + N_PSISTES;
    /* XXX: This raises error for redefinition
     * we need to change how we are passing the data to the children */
    order_t* order_list = (order_t*)shm_begin + N_DIANOMEIS + N_PSISTES;

    /* forking for the individual pizzas */
    while (1) {
        while (order_list->m_num != 0) {
            (order_list->m_num)--;
            /* Margarita type */
            pizza_type = 'm';
            pid = fork();
            if (pid == 0)
                break;
        }
        while (order_list->p_num != 0) {
            (order_list->p_num)--;
            pizza_type = 'p';
            pid = fork();
            if (pid == 0)
                break;
        }
        while (order_list->s_num != 0) {
            (order_list->s_num)--;
            pizza_type = 's';
            pid = fork();
            if (pid == 0)
                break;
        }
        /* This is to get out of the nested while-s. Other solution: goto */
        if (pid == 0)
            break;
    }
    /* after forking for the individual pizzas, exit */
    if (pid > 0)
        exit(EXIT_SUCCESS);

    /* FROM HERE INDIVIDUAL PIZZAS */
    sem_wait(&mutex);
    /* let's choose a cooker */
    for (i = 0; i < 10; i++) {
        if ( *(cook_status + i) == 0) {
            *(cook_status + i) = 1;
            chosen = i;
            break;
        }
    }
    sem_post(&mutex);

    /* ready for cooking */
    order_list->status2 = 1;
    cook(pizza_type);
    /* cooked */
    order_list->status1 = 1;
    order_list->status2 = 0;

    /* THE SAME FOR DELIVERY */

    /* detaching from shared memory */
    if (shmdt(shm_start) == -1)
        fatal("order could not detach from shared memory");	
	
    return 0;
}
