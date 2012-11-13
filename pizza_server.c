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

void log(char *message) {
    /* the function to log messages from children */
    FILE *fd;
    time_t raw_time;
    time(&raw_time);
    fd = fopen("logfile", "a");
    fprintf(fd, "[%d] --- %s --- %s", (int)getpid(), message, ctime(&raw_time));
    fclose(fd);
}

void cokehandler(int sig_num) {
    /* The SIGALARM handler to arrange the coke sendings */
    
    FILE *fd;
    fd = fopen("logfile", "a");
    fprintf(fd,"[%d] aaaa---aaaa \n",(int)getpid());
    fclose(fd);
}    
void zombiehandler(int sig_num) {
    /* The SIGCHLD handler to reap zombies */
    int status;
    wait(&status);
}

void term_hand (int sig_num) {
    /* The handler to terminater the server */
    shmctl(shm_id, IPC_RMID, NULL);
    sem_unlink(SEM_NAME1);
    sem_unlink(SEM_NAME2);
    sem_unlink(SEM_NAME3);
    exit(EXIT_SUCCESS);
}

void cook(char t) {
    /* The waiting function for the cooking */
    if (t == 'm')
        sleep(1);
    else if (t == 'p')
        sleep(2);
    else if (t == 's')
        sleep(3);
    else
        fatal("Wrong input on cook function");
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
    /* signal handlers */
    signal(SIGINT, term_hand);
    signal(SIGCHLD, zombiehandler);
    //signal(SIGALRM, cokehandler);

    /* Fork off the parent process to get into deamon mode */
    pid = fork();
    if (pid == -1)
        fatal("Can't fork parent");
    if (pid > 0) {
        /* Stopping the parent proccess and just keeping the child */
        printf(">> Deamon mode <<\n");
        exit(EXIT_SUCCESS);
    }

    /* Semaphore business */
    sem_t *cooks , *deliverers, *mutex;
    cooks = sem_open(SEM_NAME1, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR, 1);
    if (cooks == SEM_FAILED)
        fatal("could not create semaphore");
    deliverers = sem_open(SEM_NAME2, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR, 1);
    if (deliverers == SEM_FAILED)
        fatal("could not create semaphore");
    mutex = sem_open(SEM_NAME3, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR, 1);
    if (mutex == SEM_FAILED)
        fatal("could not create semaphore");
    /* semaphore initialization */
    if (sem_init(cooks, 1, N_PSISTES) == -1)
        fatal("could not initialize semaphore");
    if (sem_init(deliverers, 1, N_DIANOMEIS) == -1)
        fatal("could not initialize semaphore");
    if (sem_init(mutex, 1, 1) == -1)
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
        addr_len = sizeof(struct sockaddr_un);
        /* getting new connections from the client socket */
        new_conn = accept(sd, (struct sockaddr *) &client_addr, &addr_len);
        read(new_conn, &incoming, sizeof(incoming));
        /* required variable for shared memory allocation */
        incoming.exists = 1;
        /* close connection with this client */
        close(new_conn);

        pid = fork();
        if (pid == 0)
            break;
    }

    /* Children operate below */
    /* variables for elapsed time counting */
    struct timeval begin, end;

    /* new pid for the order sub-proccess */
    pid_t pid_order;
    char pizza_type = 'n';
    gettimeofday(&begin, NULL);		
    /* ignoring the unnecessary signals */
    signal(SIGCHLD, SIG_IGN);
    signal(SIGALRM, cokehandler);

    /* configure shared memory for every child proccess  */
    shm_id = shmget(SHM_KEY, size , 0600);
    if (shm_id == -1)          
        fatal("in shared memory (in children)");
    bool* shm_begin = shmat(shm_id, NULL, 0);
    if (shm_begin == (bool *)-1)
        fatal("children could not attach to shared memory");
    /* Configuring the pointers to shared memory */
    order_t* order_list = (order_t*)shm_begin;

    /* get the next empty position in the shared memory */
    sem_wait(mutex);
    while (order_list->exists != 0)
        order_list++;
    /* putting into the memory */
    *order_list = incoming;
    sem_post(mutex);

    log("order in shared memory");

    /* pizza sum */
    counter = order_list->m_num + order_list->p_num + order_list->s_num;
    if (counter == 0)
        fatal("No pizza");

    /* distribute pizzas in individual forks */
    while (order_list->m_num != 0) {
        (order_list->m_num)--;
        /* Margarita type */
        pizza_type = 'm';
        pid_order = fork();
        if (pid_order == 0)
            goto cooking;
    }	
    while (order_list->p_num != 0) {
        (order_list->p_num)--;
        pizza_type = 'p';
        pid_order = fork();
        if (pid_order == 0)
            goto cooking;
    }
    while (order_list->s_num != 0) {
        (order_list->s_num)--;
        pizza_type = 's';
        pid_order = fork();
        if (pid_order == 0)
            goto cooking;
    }

    if (pid_order > 0) {
        /* Code for complete order handling */

        /* set "cooking" status */
        order_list->status2 = 1;

        while (counter != 0) {
            int status;
            wait(&status);
            counter--;
        }
        /* set "cooked" status */
        order_list->status1 = 1;
        order_list->status2 = 0;

        log("ready to delivery");
        /* DELIVERY */
        sem_wait(deliverers);
        delivery(order_list->time);
        /* setting "delivering" status */
        order_list->status2 = 1;
        /* Done. Give back the deliverer */
        sem_post(deliverers);
        log("delivered");

        /* delete the order */
        order_list->exists = 0;
	gettimeofday(&end, NULL);
	FILE *fd;
        fd = fopen("logfile", "a");
	fprintf(fd,"[%d] --- elapsed time : %ld microseconds\n",(int)getpid(),
	((end.tv_sec * 1000000 + end.tv_usec) - (begin.tv_sec * 1000000 + begin.tv_usec)));	
     	fclose(fd);
        /* detaching from shared memory */
        if (shmdt(shm_begin) == -1)
            fatal("order could not detach from shared memory");

        /* closing semaphores */
        sem_close(cooks);
        sem_close(deliverers);
        sem_close(mutex);

        _exit(EXIT_SUCCESS);
    }

cooking:
    /* FROM HERE INDIVIDUAL PIZZAS */
    log("ready to get cooked");

    sem_wait(cooks);
    /* cooking */
    cook(pizza_type);
    /* Done. Semaphore up */
    sem_post(cooks);
    log("cooked");

    /* detaching from shared memory */
    if (shmdt(shm_begin) == -1)
        fatal("order could not detach from shared memory");

    /* closing semaphores */
    sem_close(cooks);
    sem_close(deliverers);
    sem_close(mutex);

    _exit(EXIT_SUCCESS);
}
