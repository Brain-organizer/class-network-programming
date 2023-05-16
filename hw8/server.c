#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <sys/uio.h>

#define BUF_SIZE 1024
#define MAX_CLNT 256

void * handle_clnt(void * arg);
void send_msg(char * msg, int len);
void error_handling(char * msg);
int make_msg(struct iovec* vec, char *buf);
int calculate(char, int*, char*);
int operate(int, int, char);

int clnt_cnt=0;
int clnt_socks[MAX_CLNT];
pthread_mutex_t mutx;

int main(int argc, char *argv[])
{
	int serv_sock, clnt_sock;
	struct sockaddr_in serv_adr, clnt_adr;
	int clnt_adr_sz;
	pthread_t t_id;
	if(argc!=2) {
		printf("Usage : %s <port>\n", argv[0]);
		exit(1);
	}
  
	pthread_mutex_init(&mutx, NULL);
	serv_sock=socket(PF_INET, SOCK_STREAM, 0);

	memset(&serv_adr, 0, sizeof(serv_adr));
	serv_adr.sin_family=AF_INET; 
	serv_adr.sin_addr.s_addr=htonl(INADDR_ANY);
	serv_adr.sin_port=htons(atoi(argv[1]));
	
	if(bind(serv_sock, (struct sockaddr*) &serv_adr, sizeof(serv_adr))==-1)
		error_handling("bind() error");
	if(listen(serv_sock, 5)==-1)
		error_handling("listen() error");
	
	while(1)
	{
		clnt_adr_sz=sizeof(clnt_adr);
		clnt_sock=accept(serv_sock, (struct sockaddr*)&clnt_adr,&clnt_adr_sz);
		
		pthread_mutex_lock(&mutx);
		clnt_socks[clnt_cnt++]=clnt_sock;
		pthread_mutex_unlock(&mutx);
	
		pthread_create(&t_id, NULL, handle_clnt, (void*)&clnt_sock);
		pthread_detach(t_id);
		printf("Connected client Port: %d \n", ntohs(clnt_adr.sin_port));
	}
	close(serv_sock);
	return 0;
}
	
void * handle_clnt(void * arg)
{
	int clnt_sock=*((int*)arg);
	int str_len=0, i;
	char msg[BUF_SIZE];
    struct iovec vec[2];
    char name[5];
    char opinfo[BUF_SIZE];

    vec[0].iov_base = name;
    vec[0].iov_len = 4;
    vec[1].iov_base = opinfo;
    vec[1].iov_len = BUF_SIZE;
	
	while(readv(clnt_sock, vec, 2)>0){
        name[4] = 0;
        str_len = make_msg(vec,msg);
		send_msg(msg, str_len);
    }
	
	pthread_mutex_lock(&mutx);
	for(i=0; i<clnt_cnt; i++)   // remove disconnected client
	{
		if(clnt_sock==clnt_socks[i])
		{
			while(i++<clnt_cnt-1)
				clnt_socks[i]=clnt_socks[i+1];
			break;
		}
	}
	clnt_cnt--;
	pthread_mutex_unlock(&mutx);

    printf("Closed client\n");
	close(clnt_sock);
	return NULL;
}
void send_msg(char * msg, int len)   // send to all
{
	int i;
	pthread_mutex_lock(&mutx);
	for(i=0; i<clnt_cnt; i++)
		write(clnt_socks[i], msg, len);
	pthread_mutex_unlock(&mutx);
}
int make_msg(struct iovec *vec, char *buf){
    char opCount, operators[128];
	int operands[128], result;
	int byte = 0;
    
    byte += sprintf(buf+byte, "[%s] ", (char *)vec[0].iov_base);

    memset(operands, 0, sizeof(operands));
    memset(operators, 0, sizeof(operators));

    opCount = ((char *)vec[1].iov_base)[0];

    for(int i=0; i<opCount; i++){
        operands[i] = ((int *)(vec[1].iov_base+1))[i];
    }

    for(int i=0; i<opCount-1; i++){
        operators[i] = ((char*)(vec[1].iov_base+1+4*opCount))[i];
    }

    for(int i=0; i<opCount-1; i++){
        byte += sprintf(buf+byte,"%d",operands[i]);
        
        buf[byte] = operators[i];
        byte += 1;
    }

    byte += sprintf(buf+byte, "%d", operands[opCount-1]);
    
    result = calculate(opCount, operands, operators);

    byte += sprintf(buf+byte, "=%d\n", result);

    return byte;
}
int calculate(char opcount, int* operands, char *operators){
	int result = operands[0];
	
	for(int i = 1; i<opcount; i++)
		result = operate(result, operands[i], operators[i-1]);
	
	return result;
}
int operate(int n, int m, char opr){
	switch(opr){
		case '*':
			return n*m;
		case '+':
			return n+m;
		case '-':
			return n-m;
	}
}
void error_handling(char * msg)
{
	fputs(msg, stderr);
	fputc('\n', stderr);
	exit(1);
}