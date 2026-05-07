#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <mysql/mysql.h>
#include "errmsg.h"

int check_mysql ( char *mysql_socket )
{
   unsigned int load = 0;

   MYSQL *con = mysql_init ( NULL );

   if ( con == NULL )
   {
      fprintf ( stderr, ERR_MYSQL01, mysql_error ( con ));
      return 1;
   }

   if ( !mysql_real_connect ( con,
               NULL,                 /* server hostname or IP address */
               NULL,                 /* mysql user */
               NULL,                 /* password */
               "information_schema", /* default database to use, NULL for none */
               0,                    /* port number, 0 for default */
               mysql_socket,         /* socket file or named pipe name */
               CLIENT_FOUND_ROWS     /* connection flags */ ))
   {
      fprintf ( stderr, ERR_MYSQL02, mysql_error ( con ));
      mysql_close ( con );
      return 1;
   }

   const char *query = "SELECT round(( (select count(*) from information_schema.processlist) / (select GLOBAL_VALUE from SYSTEM_VARIABLES where variable_name = 'MAX_CONNECTIONS') )*100)";

   if ( mysql_query ( con, query ))
   {
      fprintf ( stderr, ERR_MYSQL02, mysql_error ( con ));
      mysql_close ( con );
      return 1;
   }
   
   MYSQL_RES *result = mysql_store_result ( con );
   if ( !result )
   {
      fprintf ( stderr, ERR_MYSQL03, mysql_error ( con ));
      return 1;
   }
   MYSQL_ROW row;

   // I have only 1 field to retrieve.
   row = mysql_fetch_row ( result );
   load = (uintptr_t) atoi(row[0]);

   mysql_free_result ( result );
   
   mysql_close ( con );
   return load;
}

