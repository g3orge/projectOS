/* Client program
 * George Papanikolaou - Prokopis Gryllos
 * Operating Systems Project 2012 - Pizza Delivery
 * There is absolutely no warranty
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>	

#define MAXORDER 5
#define PATH "socketfile"

/* Declaration of boolean type */
typedef enum { false, true } bool;

/* function to clear the input buffer */
int clear_input_buffer(void) {
    int ch;
    while (((ch = getchar()) != EOF) && (ch != '\n')) /* void */;
    return ch;
}

/* Struct for the pizza order */
typedef struct {
    int m_num;
    int p_num;
    int s_num;
    bool distance;
} order_t;

/* function that print	s the correct order format */
void order_format() {
    printf("\nOrder format :$ client <arg1> <arg2> <arg3> <arg4> <arg5> ");
    printf("\n<arg1> : the number of daisy pizzas you want to order ");
    printf("\n<arg2> : the number of peperoni pizzas you want to order ");
    printf("\n<arg3> : the number of special pizzas you want to order ");
    printf("\n<arg4> : your distance from the pizza server ");
    printf(" ( 't_l' for a long distance 't_s' for short distance) ");
    printf("\n\nYour order should have the exact format as shown above\n\n");

}

/* function that prints order details */
void print_order(order_t  order) {

    printf("\n Your order \n %i daisy pizzas", order.m_num);
    printf("\n %i peperoni pizzas", order.p_num); 
    printf("\n %i special pizzas", order.s_num); 
    if (order.distance == true )
        printf("\n distance: long distance");
    else 
        printf("\n distance: short distance");
}

/* a function to return bool type */
bool torf(int i) {
    bool distance;
    if (i == 0)
        distance = false;
    else
        distance = true;
    return distance;
}

/* A function to display an error message and then exit */
void fatal(int mode,char *message) {
    if (mode == 0) {
        fprintf(stderr, "\a!! - Fatal error - ");
        fprintf(stderr, "%s\n", message);
        exit(EXIT_FAILURE);
    }
    else if (mode == 1) {
        fprintf(stderr, "\a!! - Fatal error - ");
        fprintf(stderr, "%s\n", message);
        order_format(); /* print the correct order format */
        exit(EXIT_FAILURE);
    }

}

/* This is a system function to generate random numbers where needed */
int RandomInteger(int low, int high) {  
    int k;
    int dif=high-low+1; 
    k = low + rand() % dif; 
    return k;  
}


/* function for interactively create a new order */
order_t make_order(void) {
    char choise;
    order_t order; 
    printf("\nMake your oder");
    printf("\nHow many margarita pizzas do you want? :");
    clear_input_buffer();
    scanf("%d",&order.m_num);
    printf("\nHow many peperoni pizzas do you want? :");
    clear_input_buffer();
    scanf("%d",&order.p_num);
    printf("\nHow many special pizzas do you want? :");
    clear_input_buffer();
    scanf("%d",&order.s_num);
    do {
        clear_input_buffer();
        printf("\nWhere do you live? ( 'l' for a long distance 't' for short distance):");
        choise = getchar();
    } while (( choise !=  'l' ) && ( choise != 's' ));
    if ( choise == 'l' )
        order.distance = torf(1);
    else if ( choise == 't' )
        order.distance = torf(0);
    return order;
}

/* this function creates a random order */
order_t random_order(void) {
    order_t order;
    order.m_num = RandomInteger(0,1);
    order.p_num = RandomInteger(0,1);
    order.s_num = RandomInteger(0,1);
    order.distance = torf(RandomInteger(0,1));

    return order;
}

int main(int argc, char **argv) {

    srand((int)time(0)); 
    /* create order_t object */
    order_t order;

    /* auxiliary variables */
    char confirm;
    int i;

    int client_sd; 
    struct sockaddr_un serv_addr;

    /* parsing arguments */
    if ( argc == 1 ) {
        order = make_order();
    }
    else if ( argc < 5 ) {
        if (( argc == 2 ) && (strcmp(argv[1], "rand")==0))
            order = random_order();
        else 
            fatal(1,"Some fields of the order are missing\n");

    }
    else if ( argc > 5 ) {
        fatal(1,"Wrong input format\n");
    }
    /* right case */
    else {
        if (( argv[4] != "l" ) && ( argv[4] != "s"))
            fatal(1,"Wrong input format");
        else {
            order.m_num = atoi(argv[1]);
            order.p_num = atoi(argv[2]);
            order.s_num = atoi(argv[3]);
            if ( argv[4] == "l" )
                order.distance = torf(1);
            else
                order.distance = torf(0);
        }
    }

    /* confirmation of the order */
    if ((order.m_num + order.p_num + order.s_num) > MAXORDER)
        fatal(0,"Very big order, maximum acceptable order = 5 pizzas");

    for(;;) {
        /* print out the order */
        print_order(order);

        do { 
            printf("\n confirm? [y/n] : ");
            clear_input_buffer();
            confirm = getchar();
            if ( confirm == 'y' ) 
                break;
            else if ( confirm == 'n' ) 
                order = make_order();
        } while (( confirm != 'y' ) && ( confirm !='n'));

        if (confirm=='y')
            break;
    }

    client_sd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (client_sd == -1)
        fatal(0,"while creating clients socket"); 

    /* socket info */
    serv_addr.sun_family = AF_UNIX;
    strcpy(serv_addr.sun_path, PATH); 

    /* Connect the client's and the server's endpoint. */
    if (connect(client_sd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) == -1)
        fatal(0,"while connecting to server");

    /* sent information to server */
    write(client_sd, &order, sizeof(order_t));
    /*close connection*/
    close(client_sd);

    return 0;
}
