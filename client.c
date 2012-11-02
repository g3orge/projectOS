/*
 * George Papanikolaou - Prokopis Gryllos
 * Operating Systems Project 2012 - Pizza Delivery
 * There is absolutely no warranty
 */


#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>

int main(int argc, char **argv) {
  /*TODO: arange filestream for order
  /* argument managing 
  /*TODO: declare sent variable 
  */
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
  
}
