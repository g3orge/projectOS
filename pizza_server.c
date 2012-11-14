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
int shm_id, shm_id2;

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
        usleep(TIME_MARGARITA);
    else if (t == 'p')
        usleep(TIME_PEPPERONI);
    else if (t == 's')
        usleep(TIME_SPECIAL);
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
    signal(SIGALRM, SIG_IGN);

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

    /* process for coca cola handling */
    pid = fork();
    if (pid == 0 )
        goto cocacola;		

    /* endless loop to get new connections */
    while (1) {
        addr_len = sizeof(struct sockaddr_un);
        /* getting new connections from the client socket */
        new_conn = accept(sd, (struct sockaddr *) &client_addr, &addr_len);
        read(new_conn, &incoming, sizeof(incoming));
        /* required variable for shared memory allocation */
        incoming.exists = true;
        /* close connection with this client */
        close(new_conn);

        pid = fork();
        if (pid == 0)
            break;
    }

    /* Children operate below */

    /* variables for elapsed time counting */
    struct timeval begin, end;
    gettimeofday(&begin, NULL);
    /* initialization for the coca cola process to handle */
    incoming.start_sec = begin.tv_sec; 
    incoming.start_usec = begin.tv_usec;
    /* new pid for the order sub-proccess */
    pid_t pid_order;
    char pizza_type = 'n';
    /* ignoring the unnecessary signals */
    signal(SIGCHLD, SIG_IGN);

    /* configure shared memory for every child proccess  */
    shm_id = shmget(SHM_KEY, size , 0600);
    if (shm_id == -1)          
        fatal("in shared memory (in children)");
    char* shm_begin = shmat(shm_id, NULL, 0);
    if (shm_begin == (char *)-1)                                                          
        fatal("children could not attach to shared memory");
    order_t* order_list = (order_t*)shm_begin;

    /* get the next empty position in the shared memory */
    sem_wait(mutex);
    while (order_list->exists != false)
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
        gettimeofday(&end,NULL);
        FILE *fd;
        fd = fopen("logfile", "a");
        fprintf(fd, "[%d] -^- elapsed time: %ld seconds and %ld microseconds \n",
                getpid(), (end.tv_sec -begin.tv_sec ),(end.tv_usec - begin.tv_usec));
        fclose(fd);

        /* delete the order */
        order_list->exists = false;

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


cocacola:
    /* this process periodically scans the shared memory to find overdelayed orders
     * configure shared memory for this process   */
    shm_id2 = shmget(SHM_KEY, size , 0600);
    if (shm_id2 == -1)          
        fatal("in shared memory (coca cola handling)");
    char* shm_begin2 = shmat(shm_id2, NULL, 0);
    if (shm_begin2 == (char *)-1)
        fatal("coca cola  could not attach to shared memory");

    /* Configuring the pointers to shared memory */
    order_t* order_list2 = (order_t*)shm_begin2;
    /* set variable in order to test elapsed time of order */
    struct timeval test;
    /* scan the order lists */
    for(;;) {
        order_list2 = (order_t*)shm_begin2;
        /* get current test time */	
        gettimeofday(&test, NULL );
        /* long int test_time = test.tv_sec*1000000 + test.tv_usec; */

        int j = 0; 
        while (order_list2->exists == true) {
	    j++;     
	    /* substract orders start time from current test time in order to get the elapsed time */
	    if ((test.tv_sec - order_list2->start_sec)*10^6 + 
		(test.tv_usec - order_list2->start_usec) >= 3000) {
	    	
                FILE *coke;
                coke = fopen("logfile" , "a"); 
                fprintf(coke, "### coca cola for order [%d] elapsed time : [%ld] seconds, [%ld] microseconds \n",
			j,test.tv_sec -order_list2->start_sec,test.tv_usec - order_list2->start_usec);
                fclose(coke);
            }

            order_list2++;
        }
        usleep(T_VERYLONG);
    }
    /* detaching from shared memory */
    if (shmdt(shm_begin2) == -1)
        fatal("could not detach from shared memory");
}
