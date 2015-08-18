/* util.c */

#include "util.h"
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <unistd.h>

void
die( const char *msg, ... ) {
  va_list ap;
  va_start( ap, msg );
  fprintf( stderr, "Stopping: " );
  vfprintf( stderr, msg, ap );
  va_end( ap );
  exit( 1 );
}

void
noerr( int rc ) {
  if ( rc != 0 )
    die( "error %d", rc );
}

void
drop_privileges() {
    (void) seteuid(getuid());
}

void
elevate_privileges() {
    (void) seteuid(0);
}

/* vim:ts=2:sw=2:sts=2:et:ft=c 
 */
