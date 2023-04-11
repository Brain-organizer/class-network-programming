#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>

int get_sock_ready_to_acpt(char *my_addr, char *port);
int set_sock_reusable(int sockfd);
void send_something_to_client(int clisock);
void print_from_client(int clisock);

int main(int argc, char *argv[]){
    int sockfd, cSockfd;
    struct sockaddr_in cliaddr;
    socklen_t len;

    if(argc != 2){
        fprintf(stderr, "wrong Usage!\n");
        exit(1);
    }

    sockfd = get_sock_ready_to_acpt(NULL, argv[1]);

    len = sizeof(cliaddr);
    if((cSockfd = accept(sockfd, (struct sockaddr *)&cliaddr, &len))==-1){
        perror("accept failed");
        exit(1);
    }

    send_something_to_client(cSockfd);
    
    if(shutdown(cSockfd, SHUT_WR)==-1){
        perror("half close failed");
        exit(1);
    }
    //입력버퍼를 닫았으므로 마지막으로 EOF가 전달되고, 따라서 client는 read시에 0을 반환받음. 

    print_from_client(cSockfd);

    close(cSockfd);
    close(sockfd);
}

int get_sock_ready_to_acpt(char *my_addr, char *port){
    int sockfd;
    struct sockaddr_in servaddr;

    if ((sockfd = socket(PF_INET, SOCK_STREAM, 0)) < 0){
			perror("socket creation failed");
			exit(1);
	}

    if(set_sock_reusable(sockfd))
        perror("set socket failed");
	
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(atoi(port));
    if(my_addr==NULL)
	    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    else
        servaddr.sin_addr.s_addr = inet_addr(my_addr);

	if(bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
		perror("bind failed");
		exit(1);
	}

	if(listen(sockfd, 5) < 0) {
		perror("socket failed");
		exit(1);
	}

    return sockfd;
}

int set_sock_reusable(int sockfd){
    int enable = 1;
	return setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int));
}

void send_something_to_client(int clisock){
    int textfd, rlen, wlen, tmplen;
    char buf[1024];

    if((textfd =open("text.txt", O_RDONLY)) == -1){
        perror("file open failed");
        exit(1);
    }
    
    while ((rlen = read(textfd, buf, sizeof(buf))) > 0){//EOF 만나면 0 반환됨.
        wlen = 0;
        while (wlen < rlen){
            if((tmplen = write(clisock, buf+wlen, rlen-wlen)) == -1){
                perror("writting to socket failed");
                exit(1);
            }
            wlen += tmplen;
        }
    }

    if(rlen == -1){
        perror("reading from file failed");
        exit(1);
    }

    if(close(textfd) == -1){
        perror("file close failed");
        exit(1);
    }
}

void print_from_client(int clisock){
    int rlen, wlen, tmplen;
    char buf[1024];

    while((rlen = read(clisock, buf, sizeof(buf))) > 0){//상대방의 입력버퍼가 닫기면 EOF가 날아오고, 따라서 0 반환됨.
        wlen = 0;
        while (wlen < rlen){
            if((tmplen = write(1, buf+wlen, rlen-wlen)) == -1){
                perror("writting to socket failed");
                exit(1);
            }
            wlen += tmplen;
        }
    }
    printf("\n");

    if(rlen == -1){
        perror("reading from socket failed");
        exit(1);
    }
}