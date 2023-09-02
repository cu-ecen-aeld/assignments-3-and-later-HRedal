//////////////////
// Good stuff to read:        https://beej.us/guide/bgnet/html/#system-calls-or-bust
// Qemu documentation:        https://qemu.readthedocs.io/en/latest/system/devices/net.html
// Builtroot documentation:   https://buildroot.org/downloads/manual/manual.html#configure
// Init scripts documentatio: http://man7.org/linux/man-pages/man8/start-stop-daemon.8.html
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>

#include <errno.h>

#include <syslog.h>

#include <signal.h>
#include <pthread.h>

/*
int getaddrinfo(const char *node,     // e.g. "www.example.com" or IP
                const char *service,  // e.g. "http" or port number
                const struct addrinfo *hints,
                struct addrinfo **res);*/
                
#define   PORT        "9000"
#define   MAXDATASIZE 1024
#define   BACKLOG     10

bool caught_sigint  = false;
bool caught_sigterm = false;
bool endGracefully  = false;

pthread_mutex_t criticalRegionSignal;
pthread_mutex_t criticalRegionChild;

static void signal_handler(int signal_number) {
  sigset_t previousMask;   
  sigset_t currentMask;
  sigprocmask(SIG_SETMASK, NULL, &previousMask);
  
  sigfillset(&currentMask);
  sigprocmask(SIG_SETMASK, &currentMask, NULL);
  
  pthread_mutex_lock(&criticalRegionSignal);
  
  if (signal_number == SIGINT) {
    caught_sigint = true;
  } else if (signal_number == SIGTERM) {
     caught_sigterm = true;
  }
  
  pthread_mutex_unlock(&criticalRegionSignal);
  
  sigprocmask(SIG_SETMASK, &previousMask, NULL);
}

char* fileToWrite = "/var/tmp/aesdsocketdata";

int main(int argc, char *argv[]) {
   int        			status;
   struct addrinfo 		hints;
   struct addrinfo*		servinfo;  // will point to the results
   socklen_t  			addr_size;

   struct sockaddr_in		their_addr;
   int 			my_socket;
   int 			new_socket;
   int 			numbytes;
   int				numbytesSent;
   int				indexInitBuffer;
   char 			buffer[MAXDATASIZE];
   int 			yes=1;
   
   bool                        signalRaised = false;
   int                         returnCode   = 0;
   int                         stringLength = 0;
   
   returnCode = pthread_mutex_init(&criticalRegionSignal, NULL);
   if (returnCode != 0) {
      fprintf(stderr, "Error: %d (%s) when initializing signal mutex.", errno, strerror(errno));
      exit(-1);
   }

   returnCode = pthread_mutex_init(&criticalRegionChild, NULL);
   if (returnCode != 0) {
      fprintf(stderr, "Error: %d (%s) when initializing child mutex.", errno, strerror(errno));
     exit(-1);
   }
   
   struct sigaction            new_action;
   memset(&new_action, 0, sizeof(struct sigaction));
   new_action.sa_handler = signal_handler;
   FILE*                       filePointer = NULL; 
   
   if (sigaction(SIGTERM, &new_action, NULL) != 0) {
      fprintf(stderr, "Error: %d (%s) registering for SIGTERM", errno, strerror(errno));
      exit(-1);
   }

   if (sigaction(SIGINT, &new_action, NULL) != 0) {
      fprintf(stderr, "Error: %d (%s) registering for SIGINT", errno, strerror(errno));
      exit(-1);
   }
   
   pid_t theChildPid;
   
   theChildPid = fork();
   if (theChildPid == 0) {
      printf("Waiting for a signal\n");
      pause();
      
      pthread_mutex_lock(&criticalRegionSignal);

      if (caught_sigint) {
        fprintf(stderr, "Caught signal, exiting: SIGINT!\n");
        exit(-1);
      }
        
      if (caught_sigterm) {
        fprintf(stderr, "Caught signal, exiting: SIGTERM!\n");
        exit(-1);
      }
      
      pthread_mutex_unlock(&criticalRegionSignal);
      
      exit(0);
   }
   else if (theChildPid > 0) {
     openlog(argv[0], LOG_PID|LOG_CONS, LOG_LOCAL0	);
     syslog(LOG_USER, "Syslog has been setup for aesdsocket.log\n");


	//char yes='1'; // Solaris people use this

	// lose the pesky "Address already in use" error message
	/*
	if (setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1) {
	    perror("setsockopt");
	    exit(-1);
	} */

     memset(&hints, 0, sizeof(hints)); // make sure the struct is empty
     hints.ai_family 	= AF_UNSPEC;		// don't care IPv4 or IPv6
     hints.ai_socktype = SOCK_STREAM; 	// TCP stream sockets
     hints.ai_flags 	= AI_PASSIVE;     	// fill in my IP for me

     if ((status = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
       fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
       exit(-1);
     }

     // make a socket
     my_socket = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);
     if (my_socket == -1) {
       fprintf(stderr, "socket call error: %s\n", gai_strerror(errno));
       exit(-1);
     }

     // bind it to the port we passed in to getaddrinfo();
     status = bind(my_socket, servinfo->ai_addr, servinfo->ai_addrlen);
     if (status == -1) {
       fprintf(stderr, "bind error: %s\n", gai_strerror(errno));
       exit(-1);
     }

     status = listen(my_socket, BACKLOG);
     if (status == -1) {
       fprintf(stderr, "listen error: %s\n", gai_strerror(errno));
       exit(-1);
     }

     filePointer = fopen(fileToWrite, "w+");
     if (filePointer == NULL) {
      fprintf(stderr, "Not possible to open the file to writedata received: %s\n", gai_strerror(errno));
      exit(-1);
     }

     endGracefully = false;
     while (true) {
        pthread_mutex_lock(&criticalRegionChild);
        if (endGracefully == true) {
          close(my_socket);
          fclose(filePointer);
          remove(fileToWrite);
          exit(-1);
        }
        pthread_mutex_unlock(&criticalRegionChild);

        
        printf("Accepting new connection:");
        addr_size = sizeof(their_addr);
  	new_socket = accept(my_socket, (struct sockaddr*)&their_addr, &addr_size);

	if (new_socket == -1) {
	    fprintf(stderr, "socket accept error: %s\n", gai_strerror(errno));
	    exit(-1);
	}
	printf("Connection accepted");

	char *ipv4Address_str = inet_ntoa(((struct sockaddr_in*) &their_addr)->sin_addr);
	
	syslog(LOG_INFO, "Accepted connection from %s\n", ipv4Address_str);
	if ((numbytes = recv(new_socket, buffer, MAXDATASIZE-1, 0)) == -1) {
	   fprintf(stderr, "Error receiving data on the socket %s\n", gai_strerror(errno));
	   exit(-1);
	}
	
	// Removing the \n character
	buffer[strcspn(buffer, "\r\n")] = 0;

	if (numbytes < MAXDATASIZE) {
	    buffer[numbytes] = '\0';
	}

	if (filePointer != NULL) {
	   syslog(LOG_USER, "Storing contents in file: %s\n", buffer);
	   fprintf(filePointer, "%s\n", buffer);
	}
	
	fseek(filePointer, 0, SEEK_SET);
	
	while (1) {
	  if (!fgets(buffer, sizeof(buffer), filePointer))
            break;
          // buffer[strcspn(buffer, "\n")] = 0;
            
          stringLength    = strlen(buffer);

          numbytesSent    = 0;
          indexInitBuffer = 0;
	  numbytesSent = send(new_socket, &buffer[indexInitBuffer], stringLength, 0);
	  while ((stringLength != -1) && (stringLength != numbytesSent)) {
	    stringLength    -= numbytesSent;
	    indexInitBuffer += numbytesSent;
	    numbytesSent = send(new_socket, &buffer[indexInitBuffer], stringLength, 0);
	  }
	}

	syslog(LOG_INFO, "Closed connection from %s\n", ipv4Address_str);
	close(new_socket);
     }

     close(my_socket);

     freeaddrinfo(servinfo); // free the linked-list
   }
   
   return 0;
}

