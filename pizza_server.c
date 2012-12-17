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
 * false = 0
 * true  = 1
 */

#include "pizza.h"
#include <pthread.h>
#include <time.h>
#include <signal.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <sys/ipc.h>

/* the main list for the orders in global scope */
order_t order_list[LIMIT];

/* A function to display an error message and then exit */
void fatal(char *message) {
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

/* The function to log messages from children */
void log(char *message) {
    FILE *fd;
    time_t raw_time;
    time(&raw_time);
    fd = fopen("logfile", "a");
    fprintf(fd, "[%d] --- %s --- %s", (int)getpid(), message, ctime(&raw_time));
    fclose(fd);
}

/* The handler to terminater the server */
void term_hand(int sig_num) {
    /* TODO: cancel (and join) all the threads */
    exit(EXIT_SUCCESS);
}

/* The waiting function for the delivery */
void delivering(bool d) {
    if (d == false)
        usleep(T_KONTA);
    else if (d == true)
        usleep(T_MAKRIA);
    else
        fatal("Wrong input on delivery function");
}

/* The waiting function for the cooking */
void cooking(char t) {
    if (t == 'm')
        usleep(TIME_MARGARITA);
    else if (t == 'p')
        usleep(TIME_PEPPERONI);
    else if (t == 's')
        usleep(TIME_SPECIAL);
    else
        fatal("Wrong input on cook function");
}

/* Thread function for complete individual order handling */
void* order_handling(void* incoming) {
    /* variables for elapsed time counting */
    struct timeval begin, end;
    gettimeofday(&begin, NULL);
    /* initialization for the coca cola process */
    incoming.start_sec = begin.tv_sec; 
    incoming.start_usec = begin.tv_usec;

    /* Try to find a place in the order_list array */
    short local = 0;
    /* TODO: we need semaphores here */
    while (order_list[local].exists == true)
        local++;
    order_list[local] = incoming;

    /* new thread id for the sub-threads (max = pizza counter) */
    pthread_t sub_id[counter];
    char pizza_type = 'n';

    /* distribute pizzas in sub-threads */
    short j=0;
    while (order_list[local].m_num != 0) {
        (order_list[local].m_num)--;
        /* Margarita type */
        pizza_type = 'm';
        pthread_create(&sub_id[j], NULL, &cook, &pizza_type);
        /* increment the special counter */
        j++;
    }	
    while (order_list[local].p_num != 0) {
        (order_list[local].p_num)--;
        /* Pepperoni */
        pizza_type = 'p';
        pthread_create(&sub_id[j], NULL, &cook, &pizza_type);
        /* increment the special counter */
        j++;
    }
    while (order_list[local].s_num != 0) {
        (order_list[local].s_num)--;
        /* Special */
        pizza_type = 's';
        pthread_create(&sub_id[j], NULL, &cook, &pizza_type);
        /* increment the special counter */
        j++;
    }

    /* set "cooking" status */
    order_list[local].status2 = true;

    for (j; j>0; j--) {
        if (pthread_join(sub_id[j], NULL) != 0)
            fatal("Threads failed to join");
    }
    
    /* set "cooked" status */
    order_list[local].status1 = true;
    order_list[local].status2 = false;

    log("ready for delivery");

    /* TODO: Delivery */

    /* Print time */
    gettimeofday(&end,NULL);
    FILE *fd;
    fd = fopen("logfile", "a");
    fprintf(fd, "[%d] -^- elapsed time: %ld seconds and %ld microseconds \n",
          getpid(), (end.tv_sec -begin.tv_sec ),(end.tv_usec - begin.tv_usec));
    fclose(fd);

    /* delete the order */
    order_list[local].exists = false;

    /* exit successfully */
    pthread_exit(0);
}

/* Thread function for cooking individual pizzas */
void* cook(void* pizza_type) {
    log("ready to get cooked");

    /* TODO: cooking with cooking function */

    pthread_exit(0);
}

int main() {
    /* thread type declarations */
    pthread_t id[LIMIT];
    /* temporary place for incoming data */
    order_t incoming;
    /* thread id counter */
    int i;
    /* socket file descriptors */
    int sd, new_conn;

    /* unix socket address declarations and lengths */
    struct sockaddr_un server_addr, client_addr;
    socklen_t addr_len;

    /* ================ END OF DECLARATIONS ===================*/
    /* signal handlers */
    signal(SIGINT, term_hand);

    /* Fork off the parent process to get into deamon mode (background) */
    pid = fork();
    if (pid == -1)
        fatal("Can't fork parent");
    if (pid > 0) {
        /* Stopping the parent proccess and just keeping the child */
        printf(">> Deamon mode <<\n");
        printf("Use 'make kill' to kill the server\n");
        exit(EXIT_SUCCESS);
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

    /* TODO: coca-cola */

    /* endless loop to get new connections */
    i=0;
    while (true) {
        /* get order via socket */
        addr_len = sizeof(struct sockaddr_un);
        /* getting new connections from the client socket */
        new_conn = accept(sd, (struct sockaddr *) &client_addr, &addr_len);
        read(new_conn, &incoming, sizeof(incoming));
        /* required variable for shared memory allocation */
        incoming.exists = true;
        /* close connection with this client */
        close(new_conn);

        /* Threading */
        pthread_create(&id[i], NULL, &order_handling, &incoming);

        /* required action to counter (increment or zero) */
        /* ( the LIMIth+1 order must not take the place of 1st order if 1st order is not finished ) */
        if (i < LIMIT)
            i++;
        else 
            i=0;
        
    }


cocacola:
    /* code for giving away coca-colas in case of delay */

    /* configure shared memory for this process */
    shm_id2 = shmget(SHM_KEY, size , 0600);
    if (shm_id2 == -1)          
        fatal("in shared memory (coca cola handling)");
    char* shm_begin2 = shmat(shm_id2, NULL, 0);
    if (shm_begin2 == (char *)-1)
        fatal("coca-cola could not attach to shared memory");

    /* Configuring the pointers to shared memory */
    order_t* order_list2 = (order_t*)shm_begin2;
    /* set variable in order to test elapsed time of order */
    struct timeval test;

    /* scan the order lists */
    for(;;) {
        order_list2 = (order_t*)shm_begin2;
        /* get current test time */	
        gettimeofday(&test, NULL);
        /* long int test_time = test.tv_sec*1000000 + test.tv_usec; */

        int j = 0;
        while (order_list2->exists == true) {
            j++;
            /* substract orders start time from current test time to get the elapsed time */
            if ( (test.tv_sec - order_list2->start_sec)* 1000000 +
                    (test.tv_usec - order_list2->start_usec) >= 3000) {
                /* open file to write into */
                FILE *coke;
                coke = fopen("logfile", "a");

                fprintf(coke, "[%d] ### coca cola for this order. Elapsed time: (%ld seconds and %ld microseconds)\n",
                        order_list2->mypid , (test.tv_sec - order_list2->start_sec), (test.tv_usec - order_list2->start_usec));
                fclose(coke);
            }
            order_list2++;
        }
        /* wait the proper time */
        usleep(T_VERYLONG);
    }
    /* detaching from shared memory */
    if (shmdt(shm_begin2) == -1)
        fatal("could not detach from shared memory");
}
