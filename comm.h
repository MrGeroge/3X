#ifndef _COMM_H_
#define _COMM_H_  //头文件中一般都要添加，保证编译通过，命名规范_文件名_H_

#define _FILE_OFFSET_BITS 64 //定义文件关闭设置位

#include <stdio.h>   //C语言标准输入输出库
/*
stdio.h
文件访问函数
fopen
freopen
fflush
fclose
二进制输入/输出
fread
fwrite
非格式化输入/输出
fgetc/getc
fputc/putc
ungetc
fgets
fputs
格式化输入/输出
scanf/fscanf/sscanf
printf/fprintf/sprintf
perror
文件定位
ftell
fseek
fgetpos
fsetpos
rewind
错误处理
feof
ferror
文件操作
remove
rename
tmpfile
*/
#include <stdlib.h>   //standard library标准库头文件
/*
1.void calloc(unsigned n,unsigned size);//分配n个数据项的内存连续空间,每个数据项的大小为size
2.void free(void* p);//释放p所指的内存区
3.void * malloc(unsigned size);//分配size字节的存储区
4.void * realloc(void * p,unsigned size);//将p所指出的已分配内存区的大小改为size,size可以比原来分配的空间大或小
5.int rand(void);//产生0到32767间的随机整数(0到0x7fff之间)
6.void abort(void);//异常终止一个进程.
7. void exit(int state);//程序中止执行，返回调用过程
8. char* getenv(const char *name); //返回一个指向环境变量的指针
9. int putenv(const char *name);//将字符串name增加到DOS环境变量中
10.long labs(long num);//求长整型参数的绝对值
11.double atof(char *str);//将字符串转换成一个双精度数值
12.int atoi(char *str);//将字符串转换成一个整数值
13.long atol(char *str)//将字符串转换成一个长整数
14.char *ecvt(double value,int ndigit,int *dec,int *sign); //将浮点数转换为字符串
15.char *fcvt(double value,int ndigit,int *dec,int *sign);//将浮点数变成一个字符串
*/
#include <string.h>				//memset, strcmp, strncpy_s, ...
#include <malloc.h>				//malloc, free, 
#include <sys/types.h>  //基本系统数据类型
/*
caddr_t 核心地址
clock_t 表示系统时间
comp_t 压缩的时钟滴答
dev_t 用于设备号
fd_set 文件描述集
fpos_t 文件位置
gid_t 数组值ID
ino_t i节点编号
off_t 用于文件大小和偏移量
mode_t 文件类型，文件创建模式
pid_t 进程ID和进程组ID
ptrdiff_t 是一种带符号整型，用于对两个指针执行减法运算后所得的结果
rlim_t 资源限制
size_t 反映内存中对象的大小（以字节为单位）
ssize_t 供返回字节计数或错误提示的函数使用
time_t 以秒为单位计时
uid_t 数值用户ID
wchar_t 能表示所有不同的字符码
*/

#include <sys/socket.h>			//socket, bind, listen, accept, ...网络协议簇
/*
struct sockaddr{
sa_family_t   sa_family//地址粗
char          sa_data[]//socket地址
}

struct msghdr{
void  *msg_name;  //可选地址
socklen_t     msg_namelen;//地址大小
struct iovec *msg_iov;// scatter/gather array
int           msg_iovlen;// members in msg_iov
void         *msg_control;//ancillary data, see below
socklen_t     msg_controllen;//ancillary data buffer len
int           msg_flags;//flags on received message
}

struct cmsghdr{
socklen_t     cmsg_len        data byte count, including the cmsghdr
int           cmsg_level      originating protocol
int           cmsg_type       protocol-specific type
}

int     getpeername(int socket, struct sockaddr *address,socklen_t *address_len);//双方建立连接之后,获取对方套接字地址
int     getsockname(int socket, struct sockaddr *address,socklen_t *address_len);//返回套接字地址
int     getsockopt(int socket, int level, int option_name,void *option_value, socklen_t *option_len);//返回套接字的选项
int     setsockopt(int socket, int level, int option_name,const void *option_value, socklen_t option_len);//设置套接字选项
int     shutdown(int socket, int how); //关闭套接字的读或写

*/
#include <arpa/inet.h>			//inet_addr, inet_ntoa, htonl, ...
/*

struct linger
{
int l_onoff;
int l_linger;
};
(1)l_onoff = 0;
l_linger忽略
这种方式下，就是在closesocket的时候立刻返回，底层会将未发送完的数据发送完成后再释放资源，也就
是优雅的退出。
(2)l_onoff非零
l_linger = 0;
这种方式下，在调用closesocket的时候同样会立刻返回，但不会发送未发送完成的数据，而是通过一个REST包强制的关闭socket描述符，也就是强制的退出。
(3)l_onoff非零
l_linger > 0
这种方式下，在调用closesocket的时候不会立刻返回，内核会延迟一段时间，这个时间就由l_linger得值来决定。如果超时时间到达之前，发送完未发送的数据(包括FIN包)并得到另一端的确认，closesocket会返回正确，socket描述符优雅性退出。否则，closesocket会直接返回错误值，未发送数据丢失，socket描述符被强制性退出。需要注意的时，如果socket描述符被设置为非堵塞型，则closesocket会直接返回值。

uint32_t htonl(uint32_t);  //32位主机字节序转化为网络字节序，网络字节序表TCP/IP协议采用大端排序，主机字节序采用小端排序小地址小字节
uint16_t htons(uint16_t);
uint32_t ntohl(uint32_t);  //过程相反
uint16_t ntohs(uint16_t);

const char* inet_ntop(int domain,const void* restrict addr,char * restrict str,socklen_t size);//将二进制地址转化为点分十进制字符串
int inet_pton(int domain,const char * restrict str,void * restrict addr);//将点分十进制字符串转换为二进制


*/
#include <netinet/in.h>			//sockaddr_in，struct sockaddr, ...
/*
struct sockaddr_in
 
{
 
short sin_family;/*Address family一般来说AF_INET（地址族）PF_INET（协议族）*/
 
unsigned short sin_port;/*Port number(必须要采用网络数据格式,普通数字可以用htons()函数转换成网络数据格式的数字)*/
 
struct in_addr sin_addr;/*IP address in network byte order（Internet address）*/
 
unsigned char sin_zero[8];/*Same size as struct sockaddr没有实际意义,只是为了　跟SOCKADDR结构在内存中对齐*/
 
};

*/
#include <sys/ioctl.h>			//setsockopt
/*
setsockopt 设置套接字选项
*/
#include <unistd.h>				//close
/*
对于类 Unix 系统，unistd.h 中所定义的接口通常都是大量针对系统调用的封装（英语：wrapper functions），如 fork、pipe 以及各种 I/O 原语（read、write、close 等等）

*/
#include <pthread.h>			//pthread_creat, ...
/*
pthread_create()：创建一个线程
pthread_exit()：终止当前线程
pthread_cancel()：中断另外一个线程的运行
pthread_join()：阻塞当前的线程，直到另外一个线程运行结束
pthread_attr_init()：初始化线程的属性
pthread_attr_setdetachstate()：设置脱离状态的属性（决定这个线程在终止时是否可以被结合）
pthread_attr_getdetachstate()：获取脱离状态的属性
pthread_attr_destroy()：删除线程的属性
pthread_kill()：向线程发送一个信号

同步函数
pthread_mutex_init() 初始化互斥锁
pthread_mutex_destroy() 删除互斥锁
pthread_mutex_lock()：占有互斥锁（阻塞操作）
pthread_mutex_trylock()：试图占有互斥锁（不阻塞操作）。即，当互斥锁空闲时，将占有该锁；否则，立即返回。
pthread_mutex_unlock(): 释放互斥锁
pthread_cond_init()：初始化条件变量
pthread_cond_destroy()：销毁条件变量
pthread_cond_signal(): 唤醒第一个调用pthread_cond_wait()而进入睡眠的线程
pthread_cond_wait(): 等待条件变量的特殊条件发生
Thread-local storage（或者以Pthreads术语，称作线程特有数据）：
pthread_key_create(): 分配用于标识进程中线程特定数据的键
pthread_setspecific(): 为指定线程特定数据键设置线程特定绑定
pthread_getspecific(): 获取调用线程的键绑定，并将该绑定存储在 value 指向的位置中
pthread_key_delete(): 销毁现有线程特定数据键
pthread_attr_getschedparam();获取线程优先级
pthread_attr_setschedparam();设置线程优先级

*/
#include <fcntl.h>				//open,read,write
#include <net/if.h>				//struct ifreq,struct ifconf
/*
struct ifreq
{
#define IFHWADDRLEN 6
 union
 {
  char ifrn_name[IFNAMSIZ];  
 } ifr_ifrn;
 
 union {
  struct sockaddr ifru_addr;
  struct sockaddr ifru_dstaddr;
  struct sockaddr ifru_broadaddr;
  struct sockaddr ifru_netmask;
  struct  sockaddr ifru_hwaddr;
  short ifru_flags;
  int ifru_ivalue;
  int ifru_mtu;
  struct  ifmap ifru_map;
  char ifru_slave[IFNAMSIZ]; 
  char ifru_newname[IFNAMSIZ];
  void __user * ifru_data;
  struct if_settings ifru_settings;
 } ifr_ifru;
};

struct ifconf 

{
    int    ifc_len;            /* size of buffer    */
    union 
    {
        char *ifcu_buf;                        /* input from user->kernel*/
        struct ifreq *ifcu_req;        /* return from kernel->user*/
    } ifc_ifcu;
};


*/
#include <sys/time.h>  //代码与平台有关
//#define NDEBUG
//#include <assert.h>
//#include <errno.h>
#define CONFIG	"config"//配置文件名
#define LOGS	"pslogs.txt"//日志文件名
#define ETH	"eth"
#define XDEF	3  ngroups=sum总节点数/XDEF
#define GLEN	16   //最多设备
#define MAXCONN 10   //最大连接数
#define MAXSLEEP 8   //最大睡眠时间
#define NMAXSTRIP 120//数据分条数
#define DPORT	29002  //服务器端口

#define SOCKBUFSZ (64*1024)  //套接字缓冲区大小
#define BLKSZ	512   //数据分块大小
#define MB	(1024*1024)  //M字节


//cluster node
typedef struct _cluster_node{   //集群节点
	int num; //节点编号
	char devname[GLEN];  //设备名
	char ip[GLEN];  //节点IP
	ushort port;  //节点port
}cnode;
//cluster config
typedef struct _config{  //集群配置
	int k,r,dk;  //RS码 k个数据节点，r个校验节点
	int ngroups;//数据归档流水线数数目
	size_t dbsz;//数据分块大小
	off_t stoff;//放数据块的起始地址
	cnode *p;//集群管理的节点
}cfg;

//

//functions declaration
cfg* readConfig(const char *name);
void freeConfig(cfg *pc);
int getHostIPAddr(char *ip,int len);
cnode* getSpecNode(const cfg *pc,int num);
int connectTry(char *ip,ushort port);
void xorParity(char *d1,char * d2,int len);

size_t diskIO(int fd,off_t offset,size_t nbytes,char *buf,int mode);
size_t sockIO(int sockfd,char *buf,size_t len,int mode);

#endif
