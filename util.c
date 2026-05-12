#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>
#include <time.h>
#include "util.h"
#include "errmsg.h"

int print_help ()
{
   printf ( "\nLoad Balancer Check usage \n" );
   printf ( "\n" );
   printf ( "-h          This screen.\n" );
   printf ( "-u <user>   The user this daemon runs as. Default is mysql.\n" );
   printf ( "-s <socket> The socket MariaDB or MySQL uses to connect. Default is /var/lib/mysql/mysql.sock.\n" );
   printf ( "-p <port>   This port nr this daemon will use. Default is 64000.\n" );
   printf ( "\n\n" );
}

int isodatetime ( char *retstr )
{
   time_t now = time ( NULL );  
   struct tm *t = localtime ( &now );
  
   /* return YYYY-MM-DDTHH:MM:SS */
   sprintf ( retstr, "%d-%02d-%02dT%02d:%02d:%02d", 
                     t->tm_year + 1900, 
                     t->tm_mon + 1, 
                     t->tm_mday, 
                     t->tm_hour, 
                     t->tm_min, 
                     t->tm_sec );
   return ( 0 );
}

void signal_callback_handler ( int signum )
{
   char s_now[19];

   isodatetime ( s_now );
   fprintf ( stderr, "Server killed (%d) at %s\r\n", signum, s_now );

   /* Remove the pidfile */
   unlink ( PIDFILE );
   exit (1);
}

void printperror ( char *msg )
{
   perror ( msg );
   exit ( EXIT_FAILURE );
}

void sigchld_handler ( int sig )
{
   (void) sig;
   while ( waitpid ( -1, NULL, WNOHANG ) > 0 );
}

/* int to string */
int itoa ( int value, char *sp, int radix )
{
    char tmp[16];
    char *tp = tmp;
    int i;
    unsigned v;

    int sign = ( radix == 10 && value < 0 );
    if ( sign )
        v = -value;
    else
        v = (unsigned)value;

    while ( v || tp == tmp )
    {
        i = v % radix;
        v /= radix;
        if ( i < 10 )
          *tp++ = i + '0';
        else
          *tp++ = i + 'a' - 10;
    }

    int len = tp - tmp;

    if ( sign )
    {
        *sp++ = '-';
        len++;
    }
    while ( tp > tmp )
      *sp++ = *--tp;

    return len;
}

