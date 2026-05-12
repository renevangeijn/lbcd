#define PIDFILE "/tmp/lbcd.pid"

int print_help ();
void signal_callback_handler ( int signum );
void printperror ( char *msg );
void sigchld_handler ( int sig );
int itoa ( int value, char *sp, int radix );
int isodatetime ( char *retstr );

