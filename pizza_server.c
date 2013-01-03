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

/* ========== Globals ========== */
/* the main list for the orders in global scope */
order_t order_list[LIMIT];
/* mutexes (and static initialization) */
pthread_mutex_t list_mutex     = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t cook_mutex     = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t delivery_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t fake_mutex     = PTHREAD_MUTEX_INITIALIZER;
/* condition variables (and static initialization) */
pthread_cond_t cook_cond     = PTHREAD_COND_INITIALIZER;
pthread_cond_t delivery_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t fake_cond     = PTHREAD_COND_INITIALIZER;
/* global counters */
short cookers = N_PSISTES;
short delivery_guys = N_DIANOMEIS;

/* ========== Functions ========== */
/* A function to display an error message and then exit */
void fatal(char *message)
{
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

/* The function to log messages from everywhere */
void pizza_log(char *message)
{
	FILE *fd;
	time_t raw_time;
	time(&raw_time);
	fd = fopen("logfile", "a");
	fprintf(fd, "[%d] --- %s --- %s",
				pthread_self(), message, ctime(&raw_time));
	fclose(fd);
}

/* Thread function for cooking individual pizzas */
void* cook(void* pizza_type)
{
	pizza_log("ready to get cooked");

	/* required typecast */
	char* type = (char*)(pizza_type);

	/* using condition variable to get the cooker */
	pthread_mutex_lock(&cook_mutex);
	if (cookers == 0) 
		pthread_cond_wait(&cook_cond, &cook_mutex);
	/* take the cooker */
	cookers--;
	pthread_mutex_unlock(&cook_mutex);

	/* we have the cooker. Actually cook (wait) */
	if (*type == 'm')
		sleep(TIME_MARGARITA);
	else if (*type == 'p')
		sleep(TIME_PEPPERONI);
	else if (*type == 's')
		sleep(TIME_SPECIAL);
	else
		fatal("Wrong input on cook function");

	pizza_log("cooked");

	/* give the cooker back */
	pthread_mutex_lock(&cook_mutex);
	cookers++;
	pthread_cond_signal(&cook_cond);
	pthread_mutex_unlock(&cook_mutex);

	/* bail out to join with parent thread */
	pthread_exit(0);
}

/* Coca-Cola handling function (to a single independent thread) */
void* coca_cola(void* arg)
{
	/* we are gonna need to work without alarm() since it's signal-based
	 * and signals are per-process and we don't want to stop the whole
	 * program when the coca_cola handler kicks in. */

	/* set variable in order to test elapsed time of order */
	struct timeval test;
	/* arbitary variable */
	short k;

	/* endless loop to check for delays */
	while(true) {
		/* get current test time */
		gettimeofday(&test, NULL);

		/* parse all the available space */
		for (k = 0; k <= LIMIT; k++) {
			/* the order has to exists */
			if (order_list[k].exists == true) {
				int temp_sec = order_list[k].start_sec;
				int temp_usec = order_list[k].start_usec;
				/* substracting the internal order time (temp variables) from
				 * the current time to find the elapsed time per order */
				if ((test.tv_sec - temp_sec) * 1000000 +
					(test.tv_usec - temp_usec) >= T_VERYLONG) {
					/* delayed order found. Log it */
					/* TODO: Report delay to client? */
					FILE *coke;
					coke = fopen("coke", "a");
					fprintf(coke,"### coca cola for [%d] order. Elapsed time: %ld seconds and %ld microseconds\n",
							order_list[k].myid,
							(test.tv_sec - temp_sec),
							(test.tv_usec - temp_usec));
					fclose(coke);
				}
			} /* first 'if' closes here */
		}
		/* wait between the checks */
		sleep(T_VERYLONG);
	}
}

/* Thread function for complete individual order handling */
void* order_handling(void* incoming)
{
	/* variables for elapsed time counting */
	struct timeval begin, end;
	gettimeofday(&begin, NULL);
	/* required typecast */
	order_t* incoming_order = (order_t*)(incoming);
	/* initialization for the coca cola process */
	incoming_order->start_sec = begin.tv_sec;
	incoming_order->start_usec = begin.tv_usec;

	/* Try to find a place in the order_list array */
	short local = 0;
	/* synchronization with mutex */
	pthread_mutex_lock(&list_mutex);
	while (order_list[local].exists == true)
		local++;
	order_list[local] = *incoming_order;
	pthread_mutex_unlock(&list_mutex);
	/* done */

	/* order id */
	order_list[local].myid = pthread_self();

	/* summing up the amount of pizzas in the delivery */
	short sum = order_list[local].m_num + order_list[local].p_num + order_list[local].s_num;
	if (sum > N_MAXPIZZA) 
		fatal("Very big order");
	/* sub-thread ids */
	pthread_t sub_id[sum];
	/* for pizza identification on the cook() function */
	char pizza_type = 'n';

	/* distribute pizzas in sub-threads */
	/* sub-thread counter */
	short j=0;
	while (order_list[local].m_num != 0) {
		(order_list[local].m_num)--;
		/* Margarita type */
		pizza_type = 'm';
		if (pthread_create(&sub_id[j], NULL, &cook, &pizza_type) != 0)
			fatal("Failed to create 'cooking' thread");
		/* increment the special counter */
		j++;
	}	
	while (order_list[local].p_num != 0) {
		(order_list[local].p_num)--;
		/* Pepperoni */
		pizza_type = 'p';
		if (pthread_create(&sub_id[j], NULL, &cook, &pizza_type) != 0)
			fatal("Failed to create 'cooking' thread");
		/* increment the special counter */
		j++;
	}
	while (order_list[local].s_num != 0) {
		(order_list[local].s_num)--;
		/* Special */
		pizza_type = 's';
		if (pthread_create(&sub_id[j], NULL, &cook, &pizza_type) != 0)
			fatal("Failed to create 'cooking' thread");
		/* increment the special counter */
		j++;
	}

	/* set "cooking" status */
	order_list[local].status2 = true;

	/* wait for pizzas to get ready */
	while (j > 0) {
		/* return value */
		short ret;
		/* j-1 to bypass 0-base indexing */
		if (pthread_join(sub_id[j-1], &ret) != 0)
			fatal("Thread failed to join");
		j--;
	}
	
	/* set "cooked" status */
	order_list[local].status1 = true;
	order_list[local].status2 = false;

	pizza_log("ready for delivery");

	/* get the delivery guy */
	pthread_mutex_lock(&delivery_mutex);
	if (delivery_guys == 0) 
		/* if noone is available, wait */
		pthread_cond_wait(&delivery_cond, &delivery_mutex);
	/* take the guy */
	delivery_guys--;
	pthread_mutex_unlock(&delivery_mutex);

	/* actually delivering */
	/* using regular sleep() function since it works fine thread-based */
	if (order_list[local].time == false)
		sleep(T_KONTA);
	else if (order_list[local].time == true)
		sleep(T_MAKRIA);
	else
		fatal("Wrong input on delivery function");

	pthread_mutex_lock(&delivery_mutex);
	/* and give him back */
	delivery_guys++;
	pthread_cond_signal(&delivery_cond);
	pthread_mutex_unlock(&delivery_mutex);
	/* ...done */

	/* log time */
	gettimeofday(&end,NULL);
	FILE *fd;
	fd = fopen("logfile", "a");
	fprintf(fd,"[%d] -^- Done. elapsed time: %ld seconds and %ld microseconds\n",
				pthread_self(),
				(end.tv_sec - begin.tv_sec),
				(end.tv_usec - begin.tv_usec));
	fclose(fd);

	/* delete the order */
	order_list[local].exists = false;

	/* exit successfully */
	pthread_exit(0);
}

/* ========== Main ========== */
int main()
{
	/* thread type declarations */
	pthread_t id[LIMIT];
	/* thread for coca-cola handling */
	pthread_t colas;
	/* temporary place for incoming data */
	order_t incoming;
	/* thread id counter (long?) */
	int i;
	/* socket file descriptors */
	int sd, new_conn;

	/* unix socket address declarations and lengths */
	struct sockaddr_un server_addr, client_addr;
	socklen_t addr_len;

	/* ============= END OF DECLARATIONS ================ */

	/* Fork off the parent process to get into deamon mode (background) */
	int pid = fork();
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

	if (pthread_create(&colas, NULL, &coca_cola, NULL) != 0 )
		fatal("Failed to create coca-colas handling thread");
	
	/* endless loop to get new connections */
	i=0;
	while (true) {
		/* get order via socket */
		addr_len = sizeof(struct sockaddr_un);
		/* getting new connections from the client socket */
		new_conn = accept(sd, (struct sockaddr *) &client_addr, &addr_len);
		read(new_conn, &incoming, sizeof(incoming));
		/* required variable for memory organization */
		incoming.exists = true;
		/* close connection with this client */
		close(new_conn);

		/* Threading */
		if (pthread_create(&id[i], NULL, &order_handling, &incoming) != 0)
			fatal("Failed to create basic order handling thread");

		/* required action for counter (increment or zero) */
		if (i < LIMIT)
			i++;
		else {
			i=0;
			while (order_list[i].exists == true)
				i++;
		}
	}
}
