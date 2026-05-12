#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <netdb.h>
#include <sys/wait.h>
#include "errmsg.h"
#include "priv.h"
#include "util.h"
#include "database.h"

#define LBCD_PORT 64000
#define LBCD_USER "mysql"
#define LBCD_SOCKET "/var/lib/mysql/mysql.sock"
#define BACKLOG 10
#define BUFFER_SIZE 8192

int main ( int argc, char *argv[] ) 
{
   int sockfd, new_fd;
   struct addrinfo hints, *res, *p;
   struct sockaddr_storage client_addr;
   int yes = 1;
   int status = 0;
   socklen_t client_len;
   char buffer[BUFFER_SIZE] = {0};
   ssize_t bytes_received = 0;
   char *db_user, *db_socket;
   int db_port;
   int opt, semdbuser, semdbsock = 0;
   int load = 0;
   char buf[10];
   time_t rawtime;
   struct tm * timeinfo;
   const char *htmlmsg = "HTTP/1.1 %d %s\r\nContent-Type: text/html; charset=UTF-8\r\n\r\n<!DOCTYPE html>%d\%\n";

   while (( opt = getopt ( argc, argv, "u:p:s:h" )) != -1 )
   {
      switch ( opt )
      {
         case 'h':
              print_help();
              exit (0);
              break;
         case 'u':
              db_user = (char *) malloc ( strlen ( optarg ));
              strncpy ( db_user, optarg, strlen ( optarg ));
              semdbuser = 1;
              break;
         case 's':
              db_socket = (char *) malloc ( strlen ( optarg ));
              strncpy ( db_socket, optarg, strlen ( optarg ));
              semdbsock = 1;
              break;
         case 'p':
              db_port = atoi ( optarg );
              break;
      }
   }

   signal ( SIGINT, signal_callback_handler );
   signal ( SIGHUP, signal_callback_handler );
   signal ( SIGTERM, signal_callback_handler );

   if ( db_port <= 0 )
      db_port = LBCD_PORT;

   /* Default user */
   if ( semdbuser == 0 )
   {
      db_user = (char *) malloc ( strlen ( LBCD_USER ) );
      strncpy ( db_user, LBCD_USER, strlen ( LBCD_USER ) );
   }

   // Check if we're root (required to switch users)
   if ( getuid() != 0 )
   {
      fprintf ( stderr, ERR_PRIVROOT );
      exit ( EXIT_FAILURE );
   }
   // Drop privileges to target user
   drop_privileges ( db_user );

   /* Default socket */
   if ( semdbsock == 0 )
   {
      db_socket = (char *) malloc ( strlen ( LBCD_SOCKET ));
      strncpy ( db_socket, LBCD_SOCKET, strlen ( LBCD_SOCKET ) );
      db_socket[strlen(LBCD_SOCKET)] = '\0';
   }

   if ( db_port > 1000000 )
      db_port = LBCD_PORT;
   if ( itoa ( db_port, buf, 10 ) == 0 )
      itoa ( LBCD_PORT, buf, 10 );  

   char s_now[19];
   isodatetime ( s_now );
   fprintf ( stderr, "LBCD starting as user %s on port %d at %s.\r\n", db_user, db_port, s_now );
   // Can free now, process is running as db_user
   free ( db_user );

   memset ( &hints, 0, sizeof (hints) );
   hints.ai_family = AF_UNSPEC;
   hints.ai_socktype = SOCK_STREAM;
   hints.ai_flags = AI_PASSIVE;
   
   status = getaddrinfo ( NULL, buf, &hints, &res );
   if ( status == -1 ) 
      printperror ( ERR_ADDRINFO );
  
   for ( p = res; p != NULL; p = p->ai_next ) 
   {
      sockfd = socket ( p->ai_family, p->ai_socktype, p->ai_protocol );
      if ( sockfd == -1 ) 
      {
         perror ( ERR_SOCKET );
         continue;
      }

      if ( setsockopt ( sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes) ) == -1) 
         printperror ( ERR_SOCKOPT );

      if ( bind ( sockfd, p->ai_addr, p->ai_addrlen ) == -1 ) 
      {
         perror ( ERR_SOCKBIND );
         continue;
      }
      break;
   }
   freeaddrinfo ( res );

   if ( p == NULL ) 
      printperror ( ERR_BINDFAIL );

   if ( listen ( sockfd, BACKLOG ) == -1) 
      printperror ( ERR_LISTEN );

   struct sigaction sa;
   sa.sa_handler = sigchld_handler;
   sigemptyset ( &sa.sa_mask );
   sa.sa_flags = SA_RESTART;
   if ( sigaction ( SIGCHLD, &sa, NULL ) == -1 ) 
      printperror ( ERR_SIGACTION );

   /*  Write the pidfile */
   FILE *mainpidf = fopen ( PIDFILE, "w+" );
   long mainpid;
   mainpid = (long) getpid();
   fprintf ( mainpidf, "%ld", mainpid );
   fprintf ( stderr, "Started with pid %ld\r\n", mainpid );
   fclose ( mainpidf );

   while (1) 
   {
      client_len = sizeof ( client_addr );
      new_fd = accept ( sockfd, (struct sockaddr *) &client_addr, &client_len );
      if ( new_fd == -1 ) 
      {
         perror ( ERR_ACCEPT );
         continue;
      }

      pid_t pid = fork();

      if ( pid < 0 ) 
      {
         perror ( ERR_FORKFAILED );
         close ( new_fd );
         continue;
      }
      else 
      if ( pid == 0 ) 
      {
         close ( sockfd );

         bytes_received = read ( new_fd, buffer, sizeof ( buffer ) - 1 ); 

         if ( bytes_received > 0 ) 
         {
            buffer[bytes_received] = '\0'; 
//          printf( "Received HTTPS Request: \n%s\n", buffer );
            char *response = (char *) malloc ( 1024 );

            load = check_mysql ( db_socket );
            if ( load > 0 )
            {
               sprintf ( response, htmlmsg, 200, "OK", load ) ;
            }
            else
            {
               sprintf ( response, htmlmsg, 500, "Internal Server Error", 0 )  ;
            }

            write ( new_fd, response, strlen ( response ) );
            free ( response );
         }

         close ( new_fd );
         exit ( EXIT_SUCCESS );
      }
      else 
      {
         close ( new_fd );
      }
   }
   return 0;
}
