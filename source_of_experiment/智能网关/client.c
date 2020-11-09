#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>

//网络初始化
int net_int(char *client_ip);
//串口初始化以及测试
int set_opt(int,int,int,char,int);
int uart_check(char *uart,char *uart_buffer);

int main(int argc, char **argv)
{
	int cfd;
	int recbyte,nByte;
	char buffer[1024] = {0};
	pid_t pid;
	
	int fd,wr_static,i=10;
	char *uart3 = "/dev/ttySAC3";
	//char *uart_buffer = "hello world!\n";
	char *uart_buffer = "hello world!\r\n";
	
	printf("Hello,welcome to client!\r\n");

	if(argc != 2)
	{
		printf("usage: echo ip\n");
		return -1;
	}
	if((cfd=net_int(argv[1]))==0){
		return -1;
	}
	if((fd = uart_check(uart3,uart_buffer))==-1){
		return -1;
	}
	
	pid = fork();
	if(pid == -1){
		printf("fork failed\n");
		return 1;
	}
	else if(pid){
		while(1){
			if(-1 == (recbyte = read(cfd, buffer, 1024)))
			{
				printf("read data fail !\r\n");
				return -1;
			}

			printf("read ok\r\nREC:\r\n");
			buffer[recbyte]='\0';
			printf("%s\r\n",buffer);
			
			if(strlen(buffer)>0){
				wr_static = write(fd,buffer, strlen(buffer));
				memset(buffer, 0, 1024);
			}
			usleep(50000);
		}
	}
	else{
		while(1){
			while((nByte = read(fd, buffer, 512))>0){
				//串口回显
				write(fd,buffer,nByte);
				//发送
				send(cfd, buffer, nByte, 0);
				nByte = 0;	
			}
			usleep(50000);
		}
	}
	

	close(cfd);
	close(fd);
	
	return 0;

}


//网络初始化
int net_int(char *client_ip){
	int cfd;
	struct sockaddr_in s_add, c_add;
	unsigned short portnum = 0x8888;
	
	cfd = socket(AF_INET, SOCK_STREAM, 0);
	if(-1 == cfd)
	{
		printf("socket fail ! \r\n");
		return -1;
	}

	printf("socket ok !\r\n");

	bzero(&s_add,sizeof(struct sockaddr_in));
	s_add.sin_family=AF_INET;
	s_add.sin_addr.s_addr= inet_addr(client_ip);
	s_add.sin_port=htons(portnum);
	printf("s_addr = %#x ,port : %#x\r\n",s_add.sin_addr.s_addr,s_add.sin_port);

	if(-1 == connect(cfd,(struct sockaddr *)(&s_add), sizeof(struct sockaddr)))
	{
		printf("connect fail !\r\n");
		return -1;
	}

	printf("connect ok !\r\n");
	return cfd;
}



//串口部分初始化
int set_opt(int fd,int nSpeed, int nBits, char nEvent, int nStop)
{
	struct termios newtio,oldtio;
	if  ( tcgetattr( fd,&oldtio)  !=  0) { 
		perror("SetupSerial 1");
		return -1;
	}
	bzero( &newtio, sizeof( newtio ) );
	newtio.c_cflag  |=  CLOCAL | CREAD;
	newtio.c_cflag &= ~CSIZE;

	switch( nBits )
	{
	case 7:
		newtio.c_cflag |= CS7;
		break;
	case 8:
		newtio.c_cflag |= CS8;
		break;
	}

	switch( nEvent )
	{
	case 'O':
		newtio.c_cflag |= PARENB;
		newtio.c_cflag |= PARODD;
		newtio.c_iflag |= (INPCK | ISTRIP);
		break;
	case 'E': 
		newtio.c_iflag |= (INPCK | ISTRIP);
		newtio.c_cflag |= PARENB;
		newtio.c_cflag &= ~PARODD;
		break;
	case 'N':  
		newtio.c_cflag &= ~PARENB;
		break;
	}

	switch( nSpeed )
	{
	case 2400:
		cfsetispeed(&newtio, B2400);
		cfsetospeed(&newtio, B2400);
		break;
	case 4800:
		cfsetispeed(&newtio, B4800);
		cfsetospeed(&newtio, B4800);
		break;
	case 9600:
		cfsetispeed(&newtio, B9600);
		cfsetospeed(&newtio, B9600);
		break;
	case 115200:
		cfsetispeed(&newtio, B115200);
		cfsetospeed(&newtio, B115200);
		break;
	case 460800:
		cfsetispeed(&newtio, B460800);
		cfsetospeed(&newtio, B460800);
		break;
	default:
		cfsetispeed(&newtio, B9600);
		cfsetospeed(&newtio, B9600);
		break;
	}
	if( nStop == 1 )
		newtio.c_cflag &=  ~CSTOPB;
	else if ( nStop == 2 )
	newtio.c_cflag |=  CSTOPB;
	newtio.c_cc[VTIME]  = 0;
	newtio.c_cc[VMIN] = 0;
	tcflush(fd,TCIFLUSH);
	if((tcsetattr(fd,TCSANOW,&newtio))!=0)
	{
		perror("com set error");
		return -1;
	}
//	printf("set done!\n\r");
	return 0;
}

int uart_check(char *uart,char *uart_buffer){
	int fd;
	int wr_static;
	if((fd = open(uart, O_RDWR|O_NOCTTY|O_NDELAY))<0){
		printf("open %s is failed",uart);
		return -1;
	}
	else{
		set_opt(fd, 115200, 8, 'N', 1); 
		wr_static = write(fd,uart_buffer, strlen(uart_buffer));
		
	}
	return fd;
}