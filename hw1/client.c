#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<sys/socket.h>
#include<arpa/inet.h>

int main(int argc, char* argv[]){
	int sockfd;
	char buf[100];
	char* s_num = "2018116045";
	struct sockaddr_in servaddr;

	if(argc < 3) {
		printf("usage:./client remoteAddress remotePort\n");
		return -1;
	}
	
	if((sockfd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0){
		perror("socket creation failed");
		return -1;
	}

	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(atoi(argv[1]));
	servaddr.sin_addr.s_addr = inet_addr(argv[2]);
	
	if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0 ){
		perror("connect error");
		return -1;
	}

	printf("%s\n", s_num);
	write(sockfd, s_num, strlen(s_num));

	memset(buf, 0, sizeof(buf));
	read(sockfd, buf, sizeof(buf));
	printf("%s\n", buf);

	close(sockfd);
	return 0;
}	
