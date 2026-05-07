// main.c
#define ERR_FORKFAILED  "LBCD [001] Fork failed."
#define ERR_ADDRINFO    "LBCD [008] Failed to get address information."
#define ERR_SOCKET      "LBCD [009] Cannot create socket."
#define ERR_SOCKOPT     "LBCD [010] Cannot set scoket options."
#define ERR_SOCKBIND    "LBCD [011] Cannot bind socket."
#define ERR_BINDFAIL    "LBCD [012] Failed to bind socket."
#define ERR_SIGACTION   "LBCD [013] Cannot change sigaction."
#define ERR_TLSSETUP    "LBCD [014] Failed to setup TLS context."
#define ERR_LISTEN      "LBCD [015] Failed to listen on port."
#define ERR_ACCEPT      "LBCD [016] Failed to accept a connection on the socket."
#define ERR_PRIVROOT    "LBCD [020] This program must be run as root.\n"

// database.c
#define ERR_MYSQL01     "LBCD [100] %s.\n"
#define ERR_MYSQL02     "LBCD [101] %s.\n"
#define ERR_MYSQL03     "LBCD [102] Could not retrieve results. %s\n"

// Priv.c
#define ERR_PRIV01      "LBCD: [200] User %s not found: %s.\n"
#define ERR_PRIV02      "LBCD: [201] initgroups failed."
#define ERR_PRIV03      "LBCD: [202] setgid failed." 
#define ERR_PRIV04      "LBCD: [203] setuid failed."
#define ERR_PRIV05      "LBCD: [204] Failed to set UID to %d.\n"
#define ERR_PRIV06      "LBCD: [205] Failed to set GID to %d.\n"
#define ERR_PRIV07      "LBCD: [206] clearenv failed."

