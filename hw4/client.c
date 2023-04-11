#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <string.h>
#include <arpa/inet.h>
#include <netdb.h>

void host_info(char *hostname);
void print_host(struct hostent *hostp);
void communicate_with_server(char *port, char *servip);
void connect_to_server(int sockfd, char *port, char *servip);
void readandwrite(int fromfd, int tofd);

int main(int argc, char *argv[]){
    if (argc == 2) 
        host_info(argv[1]);
    else if (argc == 3){
        communicate_with_server(argv[1], argv[2]);
    }
}

void host_info(char * hostname){
    struct hostent * hostp;

    if((hostp = gethostbyname(hostname)) == NULL){
        perror("gethostbyname() error");
        exit(1);
    }

    printf("gethostbyname()\n");
    print_host(hostp);


    if((hostp = gethostbyaddr((char*)(hostp->h_addr_list[0]), hostp->h_length, hostp->h_addrtype)) == NULL){
        perror("gethostbyaddr() error");
        exit(1);
    }

    printf("\ngethostbyaddr()\n");
    print_host(hostp);
}

void print_host(struct hostent *hostp){

    printf("Official name: %s \n", hostp->h_name);

    for(int i=0; hostp->h_aliases[i]; i++){
        printf("Aliases %d: %s \n", i, hostp->h_aliases[i]);
    }

    printf("Address type: %s \n", (hostp->h_addrtype==AF_INET)? "AF_INET" : "AF_INET6");

    for(int i=0; hostp->h_addr_list[i]; i++){
        printf("IP addr %d: %s \n", i, inet_ntoa(*(struct in_addr*)hostp->h_addr_list[i]));
    }
}

void communicate_with_server(char *port, char *servip){
    int sockfd, copyfd; 
    int socktype;
    int socktlen;
    char buf[1024];

    if((sockfd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0){
		perror("socket creation failed");
		exit(1);
	}

    socktlen = sizeof(socktype);
    if(getsockopt(sockfd, SOL_SOCKET, SO_TYPE, &socktype, &socktlen) == -1){
        perror("getsockopt() error");
        exit(1);
    }
    printf("This socket type is : %d(%d)\n", socktype, SOCK_STREAM);

    connect_to_server(sockfd, port, servip);

    copyfd = creat("copy.txt", 0666);

    readandwrite(sockfd, copyfd);
    printf("Received file data\n");
    close(copyfd);

    copyfd = open("copy.txt",O_RDONLY);
    readandwrite(copyfd, sockfd);
    close(copyfd);
    
    close(sockfd);
}

void connect_to_server(int sockfd, char *port, char *servip){
    struct sockaddr_in servaddr;

    servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(atoi(port));
	servaddr.sin_addr.s_addr = inet_addr(servip);
	
	if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0 ){
		perror("connect error");
		exit(1);
	}
}

void readandwrite(int fromfd, int tofd){
    int textfd, rlen, wlen, tmplen;
    char buf[1024];

    while ((rlen = read(fromfd, buf, sizeof(buf))) > 0){
        wlen = 0;
        while (wlen < rlen){
            if((tmplen = write(tofd, buf+wlen, rlen-wlen)) == -1){
                perror("writting to socket failed");
                exit(1);
            }
            wlen += tmplen;
        }
    }

    if(rlen == -1){
        perror("reading from socket failed");
        exit(1);
    }
}