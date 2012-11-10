/* Header file */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
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
#define SHM_KEY 7943
#define SEM_NAME "onoma"
#define LIMIT 200
#define QUEUE 5
#define PATH "/tmp/pizza-server1547809"

/* Declaration of boolean type */
typedef enum { false, true } bool;

/* Struct for the pizza order */
typedef struct {
    bool exists;
    short m_num;
    short p_num;
    short s_num;
    bool time;
    bool status1;
    bool status2;
} order_t;
