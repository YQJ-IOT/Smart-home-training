#include<stdio.h>
#include<stdlib.h>
#include <semaphore.h>
#include<sys/types.h>          
#include<sys/socket.h>
#include<netinet/in.h>
#include<netinet/tcp.h>
#include<pthread.h>
#include<string.h>
#include"pbulic.h"
#include<stdbool.h>
#include <sys/stat.h>
#include <fcntl.h>
#include<unistd.h>      
#include<termios.h>
#include<error.h>
#define CLI 5
//下面4个全局变量,存放设备数据温度,湿度,光照
int tmp=0;
int hum=0;
int led=0;
int light=0;
int set_opt(int fd,char nEvent,int nStop); //初始化串口
int sensor(int option); 	  	   //控制设备
int getdata();  			   //获取设备数据


//用来保护pic数据的操作
pthread_mutex_t picmutex;
pthread_mutex_t caijimutex;
//项目数据汇总
struct PBULIC pbulic_shuju_qt;//服务器接受的项目数据
struct PBULIC pbulic_shuju_caiji;//数据库数据


int ser_cmd_wd = 0;       //服务器获取温度指令101
int ser_cmd_sd = 0;	  //服务器获湿度指令101
int ser_cmd_light = 0;    //服务器获取光照指令101
int ser_cmd_led_on = 0;   //服务器打开led指令102
int ser_cmd_led_off = 0;  //服务器关闭led指令103
int ser_cmd_laba_on = 0;  //服务器打开喇叭指令104
int ser_cmd_laba_off = 0; //服务器关闭喇叭指令105
int ser_cmd_fensan_on = 0;//服务器打开风扇指令106
int ser_cmd_fensan_off = 0;//服务器关闭风扇指令107
//拍照信号量
sem_t sem_pz[2]; //sem_pz[0]表示拍照，sem_pz[1]表示拍照结束
//wd信号量
sem_t sem_wd[2]; //sem_wd[0]表示温度采集，sem_wd[1]表示温度采集结束
//sd信号量
sem_t sem_sd[2]; //sem_sd[0]表示湿度采集，sem_sd[1]表示湿度采集结束
//led信号量
sem_t sem_led[2]; //sem_led[0]表示拍照，sem_led[1]表示拍照结束
//laba信号量
sem_t sem_laba[2]; //sem_laba[0]表示拍照，sem_laba[1]表示拍照结束
//light信号量
sem_t sem_light[2]; //sem_light[0]表示拍照，sem_light[1]表示拍照结束
//laba信号量
sem_t sem_fensan[2]; //sem_fensan[0]表示拍照，sem_fensan[1]表示拍照结束
//图片缓冲区
char photobuff[500000];

//与客户端通信
void* clifunc(void *arg)
{
	//接收客户端的socket文件描述符,将指针里面的值取出来
	int sockfd = *(int *)arg;    
	size_t ret; //容量更大
	size_t hadget = 0;
	size_t hadsend = 0;
	//断开与main的联系
	if(0!=pthread_detach(pthread_self()))
	{
		perror("pthread_detach error");
	}
	else
	{
		printf("pthread_detach ok\n");
	}
	while(1)
	{
		//清空接收缓冲
		memset(&pbulic_shuju_qt,0,sizeof(struct PBULIC));
		//从客户端的socket文件读取
		if(0 == read(sockfd,(void *)(&pbulic_shuju_qt),sizeof(struct PBULIC)))
		{
			printf("服务器退出\n");
			break;
		}
		//服务器打印接受到的数据
		printf("----打印QT端发送来的信息----\n");
		printf("--pbulic_shuju_qt.type is %s\n",pbulic_shuju_qt.type);
		printf("--pbulic_shuju_qt.pz_cmd is %d\n",pbulic_shuju_qt.pz_cmd);
		printf("--pbulic_shuju_qt.wd_cmd is %d\n",pbulic_shuju_qt.wd_cmd);
		printf("--pbulic_shuju_qt.sd_cmd is %d\n",pbulic_shuju_qt.sd_cmd);
		printf("--pbulic_shuju_qt.ledon_cmd is %d\n",pbulic_shuju_qt.ledon_cmd);
		printf("--pbulic_shuju_qt.ledoff_cmd is %d\n",pbulic_shuju_qt.ledoff_cmd);
		printf("--pbulic_shuju_qt.labaon_cmd is %d\n",pbulic_shuju_qt.labaon_cmd);
		printf("--pbulic_shuju_qt.labaoff_cmd is %d\n",pbulic_shuju_qt.labaoff_cmd);
		printf("--pbulic_shuju_qt.light_cmd is %d\n",pbulic_shuju_qt.light_cmd);
		printf("--pbulic_shuju_qt.fensanon_cmd is %d\n",pbulic_shuju_qt.fensanon_cmd);
		printf("--pbulic_shuju_qt.fensanoff_cmd is %d\n",pbulic_shuju_qt.fensanoff_cmd);
		//接受到了QT数据
		//====================拍照指令========================
		if(pbulic_shuju_qt.pz_cmd == true)
		{
			//释放sem_pz[0]信号，色相头开始采集
			sem_post(&sem_pz[0]);
			//等待色相头采集结束，将数据传回给QT
			sem_wait(&sem_pz[1]);
			hadsend = 0;
			ret = 0;
			while(hadsend < 500000)
			{
				ret = write(sockfd,photobuff+hadsend,500000-hadsend);
				hadsend = hadsend+ret;
			}
			printf("----回传数据:%ud----\n",hadsend);
			//传递结束后，清空处理
			memset(&pbulic_shuju_caiji,0,sizeof(struct PBULIC));
		}
		//==================指令========================
		if(pbulic_shuju_qt.wd_cmd == true)
		{
			//释放sem_pz[0]信号，色相头开始采集
			sem_post(&sem_wd[0]);
			//等待色相头采集结束，将数据传回给QT
			sem_wait(&sem_wd[1]);
			//传回采集数据
			write(sockfd,&pbulic_shuju_caiji,sizeof(struct PBULIC));
			memset(&pbulic_shuju_caiji,0,sizeof(struct PBULIC));
		}
		//====================湿度指令========================
		if(pbulic_shuju_qt.sd_cmd == true)
		{
			//释放sem_pz[0]信号，色相头开始采集
			sem_post(&sem_sd[0]);
			//等待色相头采集结束，将数据传回给QT
			sem_wait(&sem_sd[1]);
			//传回采集数据
			write(sockfd,&pbulic_shuju_caiji,sizeof(struct PBULIC));
			memset(&pbulic_shuju_caiji,0,sizeof(struct PBULIC));
		}
		//====================光照指令========================
		if(pbulic_shuju_qt.light_cmd == true)
		{
			//释放sem_pz[0]信号，色相头开始采集
			sem_post(&sem_light[0]);
			//等待色相头采集结束，将数据传回给QT
			sem_wait(&sem_light[1]);
			//传回采集数据
			write(sockfd,&pbulic_shuju_caiji,sizeof(struct PBULIC));
			memset(&pbulic_shuju_caiji,0,sizeof(struct PBULIC));
		}
		//====================led指令========================
		if(pbulic_shuju_qt.ledon_cmd == true)
		{
			//释放sem_pz[0]信号，色相头开始采集
			sem_post(&sem_led[0]);
		}
		if(pbulic_shuju_qt.ledoff_cmd == true)
		{
			//释放sem_pz[0]信号，色相头开始采集
			sem_post(&sem_led[1]);
		}
		//====================laba指令========================
		if(pbulic_shuju_qt.labaon_cmd == true)
		{
			//释放sem_pz[0]信号，色相头开始采集
			sem_post(&sem_laba[0]);
		}
		if(pbulic_shuju_qt.labaoff_cmd == true)
		{
			//释放sem_pz[0]信号，色相头开始采集
			sem_post(&sem_laba[1]);
		}
		//====================风扇指令========================
		if(pbulic_shuju_qt.fensanon_cmd == true)
		{
			//释放sem_pz[0]信号，色相头开始采集
			sem_post(&sem_fensan[0]);
		}
		if(pbulic_shuju_qt.fensanoff_cmd == true)
		{
			//释放sem_pz[0]信号，色相头开始采集
			sem_post(&sem_fensan[1]);
		}
	}
	//关闭客户端的socket文件
	if(-1 == close(sockfd))
	{
		perror("close error");
	}
	else
	{
		printf("close sockfd ok\n");
		//客户端线程数-1
	}
	//退出线程
	pthread_exit(NULL);
}

//摄像头进程:调用摄像头可执行文件,读取拍摄的图片文件信息,放在photobuff缓存里面,利用socket发送给qt端
void* paizhaofunc(void *arg)
{
	printf("%s in\n",__FUNCTION__);
	//断开线程与main之间的联系
	if(0!=pthread_detach(pthread_self()))
	{
		perror("pthread_detach error");
	}
	else
	{
		printf("pthread_detach ok\n");
	}
	int file_fd;
	size_t file_room = 0;
	size_t ret;
	while(1)
	{
		printf("-->>\n");
		memset(photobuff,0,500000);
		sem_wait(&sem_pz[0]);
	        printf("拍照+1\n");
	        system("./se-xiang-tou");
		file_fd = open("text.jpg",O_EXCL|O_RDWR);
		ret = 0;
		file_room = 0;		
		pthread_mutex_lock(&picmutex);
		while(1)
		{
			ret = read(file_fd,&photobuff[file_room],1);
			file_room = file_room+ret;
			if(ret == 0)
			{
				break;
			}
		};
		printf("file_room is %zu\n",file_room);
		//打包采集数据
		memset(&pbulic_shuju_caiji,0,sizeof(struct PBULIC));
		sprintf(pbulic_shuju_caiji.type,"caiji");
		pbulic_shuju_caiji.pz_cmd = false;
		//打包数据结束，释放信号sem_pz[1],服务器将数据传给QT
		pthread_mutex_unlock(&picmutex);
		sem_post(&sem_pz[1]);
	}
	pthread_exit(NULL);
}



//温度采集线程
void* wdfunc(void *arg)
{
	printf("%s in\n",__FUNCTION__);
	//断开线程与main之间的联系
	if(0!=pthread_detach(pthread_self()))
	{
		perror("pthread_detach error");
	}
	else
	{
		printf("pthread_detach ok\n");
	}
	while(1)
	{
		//等待sem_wd[0]信号
		sem_wait(&sem_wd[0]);
		ser_cmd_wd = 101;
		pthread_mutex_lock(&caijimutex);
		sensor(ser_cmd_wd);
		printf("采集结束\n");
		pthread_mutex_unlock(&caijimutex);
		pbulic_shuju_caiji.wd_shuju = tmp;
		sem_post(&sem_wd[1]);
		//释放sem_wd[1]信号
		ser_cmd_wd = 0;
	}
	pthread_exit(NULL);
}

//湿度采集线程
void* sdfunc(void *arg)
{
	printf("%s in\n",__FUNCTION__);
	//断开线程与main之间的联系
	if(0!=pthread_detach(pthread_self()))
	{
		perror("pthread_detach error");
	}
	else
	{
		printf("pthread_detach ok\n");
	}
	while(1)
	{
		sem_wait(&sem_sd[0]);	
		ser_cmd_sd = 101;
		pthread_mutex_lock(&caijimutex);
		sensor(ser_cmd_sd);
		printf("采集结束\n");
		pthread_mutex_unlock(&caijimutex);
		pbulic_shuju_caiji.sd_shuju = hum;
		sem_post(&sem_sd[1]);
		ser_cmd_sd = 0;
	}
	pthread_exit(NULL);
}



//光照采集线程
void* ltfunc(void *arg)
{
	printf("%s in\n",__FUNCTION__);
	//断开线程与main之间的联系
	if(0!=pthread_detach(pthread_self()))
	{
		perror("pthread_detach error");
	}
	else
	{
		printf("pthread_detach ok\n");
	}
	while(1)
	{
		sem_wait(&sem_light[0]);
		ser_cmd_light = 101;
		pthread_mutex_lock(&caijimutex);
		sensor(ser_cmd_light);
		printf("采集结束\n");
		pthread_mutex_unlock(&caijimutex);
		pbulic_shuju_caiji.light = light;
		sem_post(&sem_light[1]);
		ser_cmd_light = 0;
	}
	pthread_exit(NULL);
}

//led开
void* ledonfunc(void *arg)
{
	printf("%s in\n",__FUNCTION__);
	//断开线程与main之间的联系
	if(0!=pthread_detach(pthread_self()))
	{
		perror("pthread_detach error");
	}
	else
	{
		printf("pthread_detach ok\n");
	}
	while(1)
	{	
		sem_wait(&sem_led[0]);
		ser_cmd_led_on = 102;
		pthread_mutex_lock(&caijimutex);
		sensor(ser_cmd_led_on);
		printf("采集结束\n");
		pthread_mutex_unlock(&caijimutex);
		ser_cmd_led_on = 0;
	}
	pthread_exit(NULL);
}

//led关
void* ledofffunc(void *arg)
{
	printf("%s in\n",__FUNCTION__);
	//断开线程与main之间的联系
	if(0!=pthread_detach(pthread_self()))
	{
		perror("pthread_detach error");
	}
	else
	{
		printf("pthread_detach ok\n");
	}
	while(1)
	{	
		sem_wait(&sem_led[1]);
		ser_cmd_led_off = 103;
		pthread_mutex_lock(&caijimutex);
		sensor(ser_cmd_led_off);
		printf("采集结束\n");
		pthread_mutex_unlock(&caijimutex);
		ser_cmd_led_off = 0;
	}
	pthread_exit(NULL);
}

//喇叭开
void* labaonfunc(void *arg)
{
	printf("%s in\n",__FUNCTION__);
	//断开线程与main之间的联系
	if(0!=pthread_detach(pthread_self()))
	{
		perror("pthread_detach error");
	}
	else
	{
		printf("pthread_detach ok\n");
	}
	while(1)
	{
		sem_wait(&sem_laba[0]);
		ser_cmd_laba_on = 104;
		pthread_mutex_lock(&caijimutex);
		sensor(ser_cmd_laba_on);
		printf("采集结束\n");
		pthread_mutex_unlock(&caijimutex);
		ser_cmd_laba_on = 0;
	}
	pthread_exit(NULL);
}

//喇叭关
void* labaofffunc(void *arg)
{
	printf("%s in\n",__FUNCTION__);
	//断开线程与main之间的联系
	if(0!=pthread_detach(pthread_self()))
	{
		perror("pthread_detach error");
	}
	else
	{
		printf("pthread_detach ok\n");
	}
	while(1)
	{
		sem_wait(&sem_laba[1]);
		ser_cmd_laba_off = 105;
		
		pthread_mutex_lock(&caijimutex);
		sensor(ser_cmd_laba_off);
		
		printf("采集结束\n");
		pthread_mutex_unlock(&caijimutex);
		ser_cmd_laba_off = 0;
	}
	pthread_exit(NULL);
}

//风尚开
void* fensanonfunc(void *arg)
{
	printf("%s in\n",__FUNCTION__);
	//断开线程与main之间的联系
	if(0!=pthread_detach(pthread_self()))
	{
		perror("pthread_detach error");
	}
	else
	{
		printf("pthread_detach ok\n");
	}
	while(1)
	{
		sem_wait(&sem_fensan[0]);
		ser_cmd_fensan_on = 106;

		pthread_mutex_lock(&caijimutex);
		sensor(ser_cmd_fensan_on);
		
		printf("采集结束\n");
		pthread_mutex_unlock(&caijimutex);
		
		ser_cmd_fensan_on = 0;
	}
	pthread_exit(NULL);
}
//风扇关
void* fensanofffunc(void *arg)
{
	printf("%s in\n",__FUNCTION__);
	//断开线程与main之间的联系
	if(0!=pthread_detach(pthread_self()))
	{
		perror("pthread_detach error");
	}
	else
	{
		printf("pthread_detach ok\n");
	}
	while(1)
	{
		sem_wait(&sem_fensan[1]);
		ser_cmd_fensan_off = 107;

		pthread_mutex_lock(&caijimutex);
		sensor(ser_cmd_fensan_off);
		
		printf("采集结束\n");
		pthread_mutex_unlock(&caijimutex);
		ser_cmd_fensan_off = 0;
	}
	pthread_exit(NULL);
}

//socket初始化
int sock_init()
{
	int sersockfd;
	struct sockaddr_in serdef; //服务端地址信息
	//生成服务器端文件
	if(-1==(sersockfd = socket(AF_INET,SOCK_STREAM,0)))
	{
		perror("socket error");
	}
	else
	{
		printf("socket return %d\n",sersockfd);
	}
	
	/*设置套接字的文件的属性 */
	int opt = 1;	/* 使能地址重新使用 */
	if(0!=setsockopt(sersockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)))
	{
		perror("setsockopt error");
	}
	//为服务器端文件绑定地址:ipv4+10000端口+本地地址
	//INADDR_ANY转换过来就是0.0.0.0，泛指本机的意思，也就是表示本机的所有IP
	serdef.sin_family = AF_INET;
	serdef.sin_port = htons(10000);
	serdef.sin_addr.s_addr = htonl(INADDR_ANY);
	if(-1 == bind(sersockfd,(struct sockaddr*)&serdef,sizeof(serdef)))
	{
		perror("bind error");
	}
	else
	{
		printf("bind ok\n");
	}
	//监听最多10个客户端
	if(-1 == listen(sersockfd,20))
	{
		perror("listen error");
	}
	else
	{
		printf("listen ok\n");
	}

	return sersockfd;
}
//信号量初始化
void sem_muext_init()
{
	pthread_mutex_init(&picmutex,NULL); 
	pthread_mutex_init(&caijimutex,NULL); 
	sem_init(&sem_pz[0],0,0);
	sem_init(&sem_pz[1],0,0);
	sem_init(&sem_wd[0],0,0);
	sem_init(&sem_wd[1],0,0);
	sem_init(&sem_sd[0],0,0);
	sem_init(&sem_sd[1],0,0);
	sem_init(&sem_led[0],0,0);
	sem_init(&sem_led[1],0,0);
	sem_init(&sem_laba[0],0,0);
	sem_init(&sem_laba[1],0,0);
	sem_init(&sem_light[0],0,0);
	sem_init(&sem_light[1],0,0);
	sem_init(&sem_fensan[0],0,0);
	sem_init(&sem_fensan[1],0,0);
}

//获取设备数据
int getdata(int fd)
{
    int bytes;
    unsigned int tag_value = 0xffffffbb;
    char buffer[36],recv[36];
    memset(buffer,0,sizeof(buffer));

	sleep(4);
	int i,j=0,flag=0;
    
	sleep(4);
   	while(flag==0){
		printf("............................\n");
		memset(buffer,0,sizeof(buffer));
		bytes = read(fd,buffer,sizeof(buffer));
		if(bytes==-1)
		{
			printf("ReadFailed.\n");
			//break;
		}
		else if(bytes==0)
		{
			printf("ReadNone!!!\n");
		}
		else
		{
			printf("Read:%dbytes.\n",bytes);
			for (i=0;i<sizeof(buffer);i++){
				if((unsigned int)buffer[i]==tag_value)
				flag=1;
				if(flag==1)
				{
					recv[j]=buffer[i];
					//printf("[%d]:%x\t",j,recv[j]);
					j=j+1;
					if(j==36)
					{
						tmp =(unsigned int)recv[5];
						hum =(unsigned int)recv[7];
						led =(unsigned int)recv[24];
						light =(unsigned int)((recv[20])&(0x0ff));
						printf("light == %d",light);
						memset(recv,0,sizeof(recv));
						j=0;flag=1;
					}
				}		
			} 
		}
		printf("\n");
	}
	close(fd);
}

//串口初始化
int set_opt(int fd,char nEvent,int nStop)
{
    struct termios newtio,oldtio;
    if(tcgetattr(fd,&oldtio) !=0){
        perror("SetupSerial 1");
        return -1;
    }
    bzero(&newtio,sizeof(newtio));
    newtio.c_cflag |= CLOCAL | CREAD;
    newtio.c_cflag &= ~CSIZE;
    newtio.c_cflag |= CS8;
	
    switch(nEvent)
    {   case 'O' :
           newtio.c_cflag |= PARENB;
           newtio.c_cflag |= PARODD;
           newtio.c_iflag |= (INPCK | ISTRIP);
           break;
        case 'E' :
           newtio.c_iflag |= (INPCK | ISTRIP);
           newtio.c_cflag |= PARENB;
           newtio.c_cflag &= ~PARODD;
           break;
        case 'N' :
           newtio.c_cflag &= ~PARENB;
           break;    
    }
	cfsetispeed(&newtio,B115200);
    cfsetospeed(&newtio,B115200);
	
    if(nStop == 1)/*设置停止位；若停止位为1，则清除CSTOPB，若停止位为2，则激活CSTOPB*/
    {
         newtio.c_cflag &=~ CSTOPB;
    }
    else if(nStop == 2)
    {
        newtio.c_cflag |= CSTOPB;
    }
	/*设置最少字符和等待时间，对于接收字符和等待时间没有特别的要求时*/
    newtio.c_cc[VTIME] =0;
    newtio.c_cc[VMIN]  =1;
    tcflush(fd,TCIFLUSH);/*tcflush清空终端未完成的输入/输出请求及数据；TCIFLUSH表示清空正收到的数据，且不读取出来 */
	/*激活配置使其生效*/
    if((tcsetattr(fd,TCSANOW,&newtio))!= 0)
    {
        perror("com set error");
        return -1;
    }
    printf("........tty init ok.......\n");
    return 0;
}

//控制设备
int sensor(int option){ //控制
	char buffer[5];
   	memset(buffer,0,sizeof(buffer));
    buffer[0]=0xdd;
	buffer[1]=0x09;
	buffer[2]=0x24;
	buffer[3]=0x00;
	int fd;
    char *uart3 ="/dev/ttyUSB0";
	if((fd = open(uart3,O_RDWR|O_NOCTTY))<0)
    {
		printf("open %s failed,trying once!\n",uart3); 	
		return -1;	
    } 
	printf("open %s success!!\n",uart3);
	set_opt(fd,'N',1);     
	switch(option){
	case 102:buffer[4]=0x00;break;//开灯
	case 103:buffer[4]=0x01;break;//关灯
	case 104:buffer[4]=0x02;break;//开风鸣器
	case 105:buffer[4]=0x03;break;//关风鸣器
	case 106:buffer[4]=0x04;break;//开风扇
	case 107:buffer[4]=0x08;break;//关风扇
	case 101:getdata(fd);break;
	}
	write(fd,buffer,sizeof(buffer));
	close(fd);
}

//主函数
void main()
{
	printf("%d\n",sizeof(size_t));
	sem_muext_init();
	int sersockfd;
	pthread_t pthread[11];  //创建线程对象数组

	pthread_create(&pthread[0],NULL,paizhaofunc,NULL);
	pthread_create(&pthread[1],NULL,wdfunc,NULL);
	pthread_create(&pthread[2],NULL,sdfunc,NULL);
	pthread_create(&pthread[3],NULL,ltfunc,NULL);
	pthread_create(&pthread[4],NULL,ledonfunc,NULL);
	pthread_create(&pthread[5],NULL,ledofffunc,NULL);
	pthread_create(&pthread[6],NULL,labaonfunc,NULL);
	pthread_create(&pthread[7],NULL,labaofffunc,NULL);
	pthread_create(&pthread[8],NULL,fensanonfunc,NULL);
	pthread_create(&pthread[9],NULL,fensanofffunc,NULL);


	sersockfd = sock_init();
	struct sockaddr_in clidef; //客户端地址信息
	//客户端地址信息:ipv4+任意端口+本地任意地址0.0.0.0
	clidef.sin_family = AF_INET;
	clidef.sin_port = htons(INADDR_ANY);
	clidef.sin_addr.s_addr = htonl(INADDR_ANY);
	int sockfd_new = 0;
	socklen_t addrlen = sizeof(clidef);
	while(1)
	{
		//接收客户端的connect,并分配一个客户端文件用于读写通信
		if( -1 == (sockfd_new = accept(sersockfd,(struct sockaddr*)&clidef,&addrlen)))
		{
			perror("accept error");
		}
		else
		{
			printf("sockfd_new is %d\n",sockfd_new);
			//分配线程1,clifunc1与客户端通信
			if(0!=pthread_create(&pthread[10],NULL,clifunc,(void*)&sockfd_new))
			{
				perror("pthread_create error\n");
			}
			else
			{
				printf("pthread_create ok\n");
			}
		}
	}
	//关闭客户端文件
	if(-1 == close(sersockfd))
	{
		perror("close error");
	}
	else
	{
		printf("close ok\n");
	}
	//释放互斥锁
	pthread_mutex_destroy(&picmutex);
}
