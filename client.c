#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <pthread.h>

#define BUFFER 256
#define NICKNAMESIZE 32

void *get_in_addr(struct sockaddr *sa); // Get sockaddr , IPv4 or IPv6 

void *receive_messages(void *resv); // Thread function to receive messages from server

int main(int argc , char **argv){
    int sockfd , numbytes;
    char buf[BUFFER];
    char nickname[NICKNAMESIZE];
    struct addrinfo hints, *servinfo, *p;
    int rv;
    char s[INET6_ADDRSTRLEN];
    memset(&hints , 0 , sizeof hints);
    memset(&nickname , 0 , sizeof nickname);
    memset(&buf , 0 , sizeof buf);
    memset(&servinfo , 0 , sizeof servinfo);
    memset(&p , 0 , sizeof p);

    if(argc != 3){
        fprintf(stderr, "Usage: %s <hostname> <port>\n", argv[0]);
        exit(0);
    }
    // Set up address info
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

   if((rv = getaddrinfo(argv[1],argv[2], &hints , &servinfo)) != 0){
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }
    // Loop through all results and connect to the first we can
    for(p = servinfo; p != NULL ; p = p->ai_next){
        if((sockfd = socket(p->ai_family , p->ai_socktype , p->ai_protocol)) == -1){
            perror("client: socket");
            continue;
        }

        if (connect(sockfd , p->ai_addr , p->ai_addrlen) == -1) {
            close(sockfd);
            perror("client: connect");
            continue;
        }
        break; // if we get here , we must have connected successfully
    }

    if(p == NULL){
        fprintf(stderr, "client: failed to connect\n");
        return 2;
    }
    freeaddrinfo(servinfo); // all done with this structure

    // Get nickname from user
    printf("Enter your nickname: ");
    fgets(nickname, NICKNAMESIZE, stdin);
    nickname[strcspn(nickname, "\n")] = 0; // Remove newline

 
    

    // Create a thread to receive messages from server
    pthread_t recv_thread;
    if(pthread_create(&recv_thread , NULL , receive_messages , (void *) (intptr_t) sockfd) != 0){
        perror("pthread_create");
        return 1;
    }
         inet_ntop(p->ai_family , get_in_addr((struct sockaddr *)p->ai_addr) , s , sizeof s);
           printf("client: connecting to %s\n", s);
    
    // Main loop to send messages
    for(;;){
        char massage[BUFFER - NICKNAMESIZE - 1];
        printf("> ");
        fgets(massage , sizeof massage , stdin);
        massage[strcspn(massage, "\n")] = 0; // Remove newline
        // Prepare the message with nickname
        char full_massage[BUFFER];
        snprintf(full_massage , sizeof full_massage , "%s: %s", nickname , massage); // Format message
        if(send(sockfd , full_massage , strlen(full_massage), 0) == -1){
            perror("send");
        }  
        memset(&massage , 0 , sizeof massage);
        memset(&full_massage , 0 , sizeof full_massage);
    }
    pthread_join(recv_thread , NULL);
    close(sockfd);
    return 0;
    
}

// Thread function to receive messages from server
void *receive_messages(void *resv){
    int sockfd = (intptr_t) resv;
    char buf[BUFFER];
    int numbytes;

    for(;;){
        if((numbytes = recv(sockfd , buf , sizeof buf - 1 , 0)) == -1){
            perror("recv");
            exit(1);
        }

        if(numbytes == 0){
            printf("Connection closed by server\n");
            close(sockfd);
            exit(0);
        }
        buf[numbytes] = '\0'; // Null-terminate the received data
        printf("%s\n", buf); // Print the received message
    }
    return NULL;
}

void *get_in_addr(struct sockaddr *sa){
    if(sa->sa_family == AF_INET){
        return &(((struct sockaddr_in *)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}