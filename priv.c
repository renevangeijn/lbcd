#include <sys/types.h>
#include <pwd.h>
#include <grp.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include "errmsg.h"

void drop_privileges ( const char *username )
{
    struct passwd *pw;

    // Step 1: Resolve username to UID/GID
    pw = getpwnam ( username );

    if ( !pw )
    {
        fprintf ( stderr, ERR_PRIV01, username, strerror(errno) );
        exit ( EXIT_FAILURE );
    }
    uid_t target_uid = pw->pw_uid;
    gid_t target_gid = pw->pw_gid;

    // Step 2: Drop supplementary groups
    if ( initgroups ( username, target_gid ) == -1 )
    {
        perror ( ERR_PRIV02 );
        exit ( EXIT_FAILURE );
    }

    // Step 3: Set GID (RGID, EGID, SGID)
    if ( setgid ( target_gid ) == -1)
    {
        perror ( ERR_PRIV03 );
        exit ( EXIT_FAILURE );
    }

    // Step 4: Set UID (RUID, EUID, SUID)
    if ( setuid ( target_uid ) == -1 )
    {
        perror ( ERR_PRIV04 );
        exit ( EXIT_FAILURE );
    }

    // Step 5: Verify UID/GID switch
    if ( getuid() != target_uid || geteuid() != target_uid )
    {
        fprintf ( stderr, ERR_PRIV05, target_uid );
        exit ( EXIT_FAILURE );
    }
    if ( getgid() != target_gid || getegid() != target_gid )
    {
        fprintf ( stderr, ERR_PRIV06, target_gid );
        exit(EXIT_FAILURE);
    }

    // Step 7: Sanitize environment
    if ( clearenv() != 0 )
    {
        perror ( ERR_PRIV07 );
        exit ( EXIT_FAILURE );
    }
    setenv ( "PATH", "/usr/bin:/bin", 1 ); // Minimal safe PATH
}
