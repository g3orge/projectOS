/* Header file */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h> 
#include <unistd.h>
#include <fcntl.h>
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
/* change this if server can't connect to shared memory */
#define SHM_KEY 8843
#define SEM_NAME1 "mageires"
#define SEM_NAME2 "pizzaguys"
#define SEM_NAME3 "onoma"
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
