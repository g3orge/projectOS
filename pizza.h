/* Header file */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h> 
#include <unistd.h>
#include <fcntl.h>
/* Pizza default values (time in microseconds) */ 
#define TIME_MARGARITA 10000
#define TIME_PEPPERONI 12000
#define TIME_SPECIAL 15000
#define T_KONTA 5000
#define T_MAKRIA 10000
#define T_VERYLONG 50000
#define N_DIANOMEIS 10
#define N_PSISTES 10
#define N_MAXPIZZA 3
/* change this if server can't connect to shared memory */
#define SHM_KEY 8843
/* random names for semaphores */
#define SEM_NAME1 "ppizzaguys"
#define SEM_NAME2 "ppveinafhu"
#define SEM_NAME3 "haeriglurf"
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
    /* to have the pid of the process that handles it */
    pid_t mypid;
} order_t;
