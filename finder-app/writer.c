#include <stdio.h>
#include <syslog.h>

int main(int argc, char *argv[])
{
  char* theFileName  	= 0L;
  char* theWordToWrite = 0L;
  FILE* theFile 	= 0L;
  
  openlog(argv[0], LOG_PID|LOG_CONS, 0);
  syslog(LOG_USER, "Syslog has been setup for myapp.log\n");
  
  if ( argc < 2 )
  {
    syslog(LOG_ERR, "Low number of arguments. The usage of the script is: writer <writefile> <writestr>]\n");
    closelog();
    return 1;
  }
  else
  {
    theFileName    = argv[1];
    theWordToWrite = argv[2];
  }
  
  if ((theFile = fopen(theFileName, "a"))!=0l)
  {
    syslog(LOG_DEBUG, "Writing %s to %s\n", theFileName, theWordToWrite);
    fprintf(theFile, theWordToWrite, "\n%s\n");
    fclose(theFile);
  }
  else
  {
    syslog(LOG_ERR, "Not possible to open the file [%s] passed by argument.\n", theFileName);
    closelog();
    return 1;
  }
  
  closelog();
  return 0;
}

