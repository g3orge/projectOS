/* Pizza server 2012 Operating Systems Project
 * Header file */

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/un.h> 
#include <unistd.h>
#include <fcntl.h>
/* Pizza default values (time in seconds) */ 
#define TIME_MARGARITA 1
#define TIME_PEPPERONI 2
#define TIME_SPECIAL 3
#define T_KONTA 3
#define T_MAKRIA 5
#define T_VERYLONG 20
#define N_DIANOMEIS 10
#define N_PSISTES 10
/* pizza-in-every-order limit */
#define N_MAXPIZZA 3
/* pizza limit, listen queue */
#define LIMIT 200
#define QUEUE 5
#define PATH "/tmp/pizza-server1547809"

/* Declaration of boolean type */
typedef enum { false, true } bool;

/* Struct for the pizza order */
typedef struct {
    bool exists;
    /* sums of pizza kinds */
    short m_num;
    short p_num;
    short s_num;
    /* for distance [short|long] */
    bool time;
    bool status1;
    bool status2;
    /* time variables */
    int start_sec;
    int start_usec;
	/* we need an order id */
	pthread_t myid;
} order_t;
