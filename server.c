#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/select.h>
#include <arpa/inet.h>

#define BACKLOG 10 // Number of connections allowed on the incoming queue
#define BUFFER 256

int get_listener_socket(const char *port); // Create and bind a listening socket

             
void run_chat_loop(int listener); // Main loop for chatting with clients
            
void handle_new_connection(int listener, fd_set *master, int *fdmax); // Handle new client connections

void handle_client_message(int sender_fd, fd_set *master, int listener, int *fdmax); // Handle messages from clients

void *get_in_addr(struct sockaddr *sa); // Get sockaddr , IPv4 or IPv6:


int main(int argc , char **argv){
    puts("Starting server...");
    if(argc != 2){
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        exit(0);
    }
    int sockfd = get_listener_socket(argv[1]);
    if (sockfd < 0){
        fprintf(stderr , "Error getting listening socket\n");
        exit(1);
    }

    run_chat_loop(sockfd);
   
    
    return 0;

}





// get a listening socket , bind it , and start listening
int get_listener_socket(const char *port) {
    int listener;
    int status;
    int yes = 1;
    struct addrinfo hints, *res, *p;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if ((status = getaddrinfo(NULL, port, &hints, &res)) != 0) {
        fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
        return -1;
    }

    for (p = res; p != NULL; p = p->ai_next) {
        listener = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (listener < 0) {
            continue;
        }

        setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

        if (bind(listener, p->ai_addr, p->ai_addrlen) < 0) {
            close(listener);
            continue;
        }
        break;
    }

    freeaddrinfo(res);

    if (p == NULL) {
        return -1;
    }

    if (listen(listener, BACKLOG) == -1) {
        close(listener);
        return -1;
    }

    return listener;
}




// main loop for chatting with clients
void run_chat_loop(int listener){
    fd_set master; // master file descriptor list
    fd_set read_fds; // temp file descriptor list for select()
    int fdmax;
    int i;

    // clear the master and temp sets
    FD_ZERO(&master);
    FD_ZERO(&read_fds);

    // add the listener to the master set
    FD_SET(listener , &master);
    fdmax = listener; // so far , it's this one

    printf("Server is running on socket %d...\n", listener);

    while (1)
    {
       read_fds = master; // copy it

       if(select(fdmax+1,&read_fds,NULL,NULL,NULL ) == -1){
              perror("select");
              exit(4);
       }
         // run through the existing connections looking for data to read
         for(i = 0 ; i <= fdmax;i++){
            if(FD_ISSET(i , &read_fds)){

                if (i == listener){
                    handle_new_connection(listener , &master , &fdmax);
                }
                else{
                    handle_client_message(i, &master, listener, &fdmax);

                }
            }
         }
    }
    
}

void handle_new_connection(int listener, fd_set *master, int *fdmax) {
    struct sockaddr_storage remoteaddr; 
    socklen_t addrlen = sizeof remoteaddr;
    int newfd;
    char remoteIP[INET6_ADDRSTRLEN];

    newfd = accept(listener, (struct sockaddr *)&remoteaddr, &addrlen);
   
    
    if (newfd < 0) {
        perror("accept");
    } else {
        // add to master set
        FD_SET(newfd, master);
        if (newfd > *fdmax) {
            *fdmax = newfd;
        }
        // print connection info 
        printf("New connection from %s on socket %d\n",
            inet_ntop(remoteaddr.ss_family,
                get_in_addr((struct sockaddr*)&remoteaddr),
                remoteIP, INET6_ADDRSTRLEN),
            newfd);
    }
}

void handle_client_message(int sender_fd, fd_set *master, int listener, int *fdmax) {
    char buf[BUFFER]; // buffer for client data
    int nbytes; // q
    // `
    if ((nbytes = recv(sender_fd, buf, sizeof buf, 0)) <= 0) {
        if (nbytes == 0) {
            printf("Socket %d hung up\n", sender_fd);  // connection closed
        } else {
            perror("recv");
        }
        close(sender_fd);
        FD_CLR(sender_fd, master);
    } else {
        for (int j = 0; j <= *fdmax; j++) {
            if (FD_ISSET(j, master)) {
                if (j != listener && j != sender_fd) {
                    buf[nbytes] = '\0'; // Null-terminate the received data
                    if (send(j, buf, nbytes, 0) == -1) {
                        perror("send");
                    }
                }
            }
        }
    }
}
// get sockaddr , IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa) {
    // IPv4
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    // IPv6
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}