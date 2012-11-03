/* Client programm
 * George Papanikolaou - Prokopis Gryllos
 * Operating Systems Project 2012 - Pizza Delivery
 * There is absolutely no warranty
 */

#include <stdio.h>
#include <stdlib.h
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>

#define MAXORDER 10

/* Struct for the pizza order */
typedef struct {
    short int d_num;
    short int p_num;
    short int s_num;
    char *distance;
  
} order_t;

order_t make_order(){
    order_t order;
    printf("\nmake your oder\n");
    printf("\nHow many daisy pizzas do you want? :");
    order.d_num = getchar();
    printf("\nHow many peperoni pizzas do you want? :");
    order.p_num = getchar();
    printf("\nHow many special pizzas do you want? :");
    order.s_num = getchar();
    printf("\n Where do you live? ( 't_l' for a long distance 't_s' for short distance)");
}

void print_order(order_t  order){
  
    printf("Your order \n %d daisy pizzas", order.d_num);
    printf("\n %d peperoni pizzas", order.p_num); 
    printf("\n %d special pizzas", order.s_num); 
    printf("\n %p distance", order.distance);      
}

void order_format(){
    printf("\nOrder format :$ client <arg1> <arg2> <arg3> <arg4> <arg5> ");
    printf("\n<arg1> : the number of daisy pizzas you want to order ");
    printf("\n<arg2> : the number of peperoni pizzas you want to order ");
    printf("\n<arg3> : the number of special pizzas you want to order ");
    printf("\n<arg4> : your distance from the pizza server ");
    printf(" ( 't_l' for a long distance 't_s' for short distance) ");
    printf("\n your order should have the exact format as shown above ");
    printf("\n PRESS ANY KEY TO EXIT");
    int exit = getchar();
}

order_t random_order(){
  /* this function creates a random order */
}

int main(int argc, char **argv) {
    /* create order_t object */
    order_t order;
    
    /* auxiliary variables */
    char confirm;
    short int  counter=0;
    
    /* parsing arguments */
    if ( argc == 0 ) {
      order = make_order();
    }
    else if ( argc < 5 ) {
      if (( argc == 1 ) && ( argv[1] == 'rand' ))
	order = random_order();
      else {
      printf("Some fields of the order are missing\n");
      /* TODO: print out the missing fields */
      order_format(); /* print the correct oder format */
      exit(EXIT_FAILURE);
      }
    }
    else if ( argc > 5 ) {
      printf("Wrong input format\n");
      order_format();
      exit(EXIT_FAILURE);
    }
    /* right case */
    else {
      /*TODO: check for wrong input data type */
      order.d_num = atoi(argv[1]);
      order.p_num = atoi(argv[2]);
      order.s_num = atoi(argv[3]);
      order.distance = argv[4];
    }
      
      /* confirmation of the order */
   for (int i = 0; i<3 ; i++ ) {
       
      /* print out the order */
      print_order();
     
      do { 
	printf("\n confirm? [y/n] : ");
	confirm = getchar();
	if ( confirm == 'y' ) {
	  break;
	}
	else if ( confirm == 'n' ) {
	  order = make_order();
	}
      } while (( confirm != 'y' ) && ( confirm !='n'));
   }
    
    
    /* TODO: declare sent variable */
    /* create clients socket descriptor */
    int client_sd; 
    /* struct for server socket address */
    struct sockaddr_un serv_addr;
    /* create clients endpoint */
    if (client_sd = socket( PF_UNIX, SOCK_STREAM, 0 )== -1 )
      fatal("while creating clients socket"); 
    /* socket internal information --- Maybe: AF_LOCAL */
    servaddr.sun_family = AF_UNIX;
    /* Zero all fields of servaddr. */
    bzero( &servaddr, sizeof( serv_addr ) ); 
    /* Define the name of this socket. */
    strcpy( servaddr.sun_path, PATH ); 
    /* Connect the client's and the server's endpoint. */
    connect(client_sd, (struct sockaddr *) &serv_addr, sizeof(serv_addr));
    /*sent information to server */
    write( client_sd , &send , sizeof( char ) );
    /*close connection*/
    close(client_sd);
    
    return 0;
}
