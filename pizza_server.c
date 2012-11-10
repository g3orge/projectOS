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

#include "pizza.h"
#include <time.h>
#include <signal.h>
#include <semaphore.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/shm.h>

/* Global variables */
/* necessary signal counter */
int counter = 0;
int shm_id;

void fatal(char *message) {
    /* A function to display an error message and then exit */
    fprintf(stderr, "\a!! - Fatal error - ");
    fprintf(stderr, "%s\n", message);
    /* also writing to a logfile */
    FILE *fd;
    time_t raw_time;
    time(&raw_time);
    fd = fopen("logfile", "a");
    fprintf(fd, "!!! Fatal error: %s --- %s", message, ctime(&raw_time));
    fclose(fd);
    exit(EXIT_FAILURE);
}

void handler(int sig_num) {
    /* Decrement the counter, this is for the order handling
     * function to see how many pizzas are ready */
    counter--;
}

void term_hand (int sig_num) {
    /* The handler to terminater the server */
    shmctl(shm_id, IPC_RMID, NULL);
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

void delivery(bool d) {
    /* The waiting function for the delivery */
    if (d == false)
        usleep(T_KONTA);
    else if (d == true)
        usleep(T_MAKRIA);
    else
        fatal("Wrong input on delivery function");
}

int main() {
    pid_t pid;
    int i;
    /* socket file descriptors */
    int sd, new_conn;
    /* temporary place for incoming data */
    order_t incoming;
    /* unix socket address declarations and lengths */
    struct sockaddr_un server_addr, client_addr;
    socklen_t addr_len;
    
    /* Shared memory size: order number limit + status buffers */
    int size = LIMIT * sizeof(order_t);

    /* ================ END OF DECLARATIONS ===================*/
    /* Ctrl-C signal handler */
    signal(SIGINT, term_hand);

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
    sem_t *cooks , *deliverers;
    cooks = sem_open(SEM_NAME, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR, 1);
    if (cooks == SEM_FAILED)
        fatal("could not create semaphore");
    deliverers = sem_open(SEM_NAME, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR, 1);
    if (deliverers == SEM_FAILED)
        fatal("could not create semaphore");
    /* semaphore initialization */
    if (sem_init(cooks, 1, N_PSISTES) == -1)
        fatal("could not initialize semaphore");
    if (sem_init(deliverers, 1, N_DIANOMEIS) == -1)
        fatal("could not initialize semaphore");

    /* Shared memory allocation */
    shm_id = shmget(SHM_KEY, size, 0600 | IPC_CREAT);
    if (shm_id == -1)
        fatal("in shared memory (parent)");
    /* we don't need to attach here */

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
        /* required variable for shared memory allocation */
        incoming.exists = 1;
        /* close connection with this client */
        close(new_conn);
        printf("%d\n",incoming.m_num);

        /* pid = fork(); */
        if (pid == 0)
            break;
    }

    /* Children operate below */

    /* new pid for the order sub-proccess */
    pid_t pid_order;
    char pizza_type = 'n';
	
    /* configure shared memory for every child proccess  */
    shm_id = shmget(SHM_KEY, size , 0600);
    if (shm_id == -1)          
        fatal("in shared memory (in children)");
    bool* shm_begin = shmat(shm_id, NULL, 0);
    if (shm_begin == (bool *)-1)
        fatal("children could not attach to shared memory");
    /* Configuring the pointers to shared memory */
    order_t* order_list = (order_t*)shm_begin;

    /* get the next empty position */
    while (order_list->exists != 0)
        order_list++;

    *order_list = incoming;

    /* pizza sum */
    counter = order_list->m_num + order_list->p_num + order_list->s_num;
    if (counter == 0)
        fatal("No pizza");

    while (order_list->m_num != 0) {
        (order_list->m_num)--;
        /* Margarita type */
        pizza_type = 'm';
        pid_order = fork();
        if (pid_order == 0)
            break;
    }
    while (order_list->p_num != 0) {
        (order_list->p_num)--;
        pizza_type = 'p';
        pid_order = fork();
        if (pid_order == 0)
            break;
    }
    while (order_list->s_num != 0) {
        (order_list->s_num)--;
        pizza_type = 's';
        pid_order = fork();
        if (pid_order == 0)
            break;
    }

    if (pid_order > 0) {
        /* Code for complete order handling */
        signal(SIGUSR1, handler);

        /* set "cooking" status */
        order_list->status2 = 1;

        while (counter != 0) { /* wait */ }
        /* set "cooked" status */
        order_list->status1 = 1;
        order_list->status2 = 0;

        /* DELIVERY */
        sem_wait(deliverers);
        delivery(order_list->time);
        /* setting "delivering" status */
        order_list->status2 = 1;
        /* Done. Give back the deliverer */
        sem_post(deliverers);

        /* detaching from shared memory */
        if (shmdt(shm_begin) == -1)
            fatal("order could not detach from shared memory");
	
        exit(EXIT_SUCCESS);
    }

    /* FROM HERE INDIVIDUAL PIZZAS */

    /* cooking */
    sem_wait(cooks);
    cook(pizza_type);
    pid_order = getppid();
    /* pizza ready, send the signal */
    kill(pid_order, SIGUSR1);
    /* Done. Semaphore up */
    sem_post(cooks);

    return 0;
}
