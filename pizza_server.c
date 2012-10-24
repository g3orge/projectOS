/*
 * George Papanikolaou - Prokopis Gryllos
 * Operating Systems Project 2012 - Pizza Delivery
 * There is absolutely no warranty
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>

#define TIME_MARGARITA 10
#define TIME_PEPPERONI 12
#define TIME_SPECIAL 15
#define T_KONTA 5
#define T_MAKRIA 10
#define T_VERYLONG 50
#define N_DIANOMEIS 10
#define N_PSISTES 10
#define N_MAXPIZZA 3

/* Declaration of boolean type */
typedef enum { false, true } bool;

/* Struct for the pizza order */
struct order {
    short m_num = 0;
    short p_num = 0;
    short s_num = 0;
    int time = 0;
    bool status1=0;
    bool status2=0;
};
typedef struct order order_t;

/* A function to display an error message and then exit */
void fatal(char *message) {
    fprintf(stderr, "\a!! - Fatal error - ");
    fprintf(stderr, "%s\n", message);
    exit(EXIT_FAILURE);
}

/* An error checked malloc() wrapper function */
void *ec_malloc(unsigned int size) {
    void *ptr;
    ptr = malloc(size);
    if(ptr == NULL)
        fatal("in ec_malloc() on memory allocation");
    return ptr;
}

void handler(int sig_num) {
    /* handling the Signal of the client SIGUSR1 (10) */
}

void cook() {
    /* Cooker function */
}

void deliverer() {
    /* Delivery function */
}

int main() {
    /* Our process ID */
    pid_t pid;

    /* Fork off the parent process */
    pid = fork();

    if (pid == -1)
        fatal("Can't fork parent");
    if (pid > 0) {
        /* Stopping the parent proccess and just keeping the child */
        printf(">> Deamon mode <<\n");
        exit(EXIT_SUCCESS);
    }

    /* Child operates here */
}
