#ifndef _COMM_H_
#define _COMM_H_  //ͷ�ļ���һ�㶼Ҫ��ӣ���֤����ͨ���������淶_�ļ���_H_

#define _FILE_OFFSET_BITS 64 //�����ļ��ر�����λ

#include <stdio.h>   //C���Ա�׼���������
/*
stdio.h
�ļ����ʺ���
fopen
freopen
fflush
fclose
����������/���
fread
fwrite
�Ǹ�ʽ������/���
fgetc/getc
fputc/putc
ungetc
fgets
fputs
��ʽ������/���
scanf/fscanf/sscanf
printf/fprintf/sprintf
perror
�ļ���λ
ftell
fseek
fgetpos
fsetpos
rewind
������
feof
ferror
�ļ�����
remove
rename
tmpfile
*/
#include <stdlib.h>   //standard library��׼��ͷ�ļ�
/*
1.void calloc(unsigned n,unsigned size);//����n����������ڴ������ռ�,ÿ��������Ĵ�СΪsize
2.void free(void* p);//�ͷ�p��ָ���ڴ���
3.void * malloc(unsigned size);//����size�ֽڵĴ洢��
4.void * realloc(void * p,unsigned size);//��p��ָ�����ѷ����ڴ����Ĵ�С��Ϊsize,size���Ա�ԭ������Ŀռ���С
5.int rand(void);//����0��32767����������(0��0x7fff֮��)
6.void abort(void);//�쳣��ֹһ������.
7. void exit(int state);//������ִֹ�У����ص��ù���
8. char* getenv(const char *name); //����һ��ָ�򻷾�������ָ��
9. int putenv(const char *name);//���ַ���name���ӵ�DOS����������
10.long labs(long num);//�����Ͳ����ľ���ֵ
11.double atof(char *str);//���ַ���ת����һ��˫������ֵ
12.int atoi(char *str);//���ַ���ת����һ������ֵ
13.long atol(char *str)//���ַ���ת����һ��������
14.char *ecvt(double value,int ndigit,int *dec,int *sign); //��������ת��Ϊ�ַ���
15.char *fcvt(double value,int ndigit,int *dec,int *sign);//�����������һ���ַ���
*/
#include <string.h>				//memset, strcmp, strncpy_s, ...
#include <malloc.h>				//malloc, free, 
#include <sys/types.h>  //����ϵͳ��������
/*
caddr_t ���ĵ�ַ
clock_t ��ʾϵͳʱ��
comp_t ѹ����ʱ�ӵδ�
dev_t �����豸��
fd_set �ļ�������
fpos_t �ļ�λ��
gid_t ����ֵID
ino_t i�ڵ���
off_t �����ļ���С��ƫ����
mode_t �ļ����ͣ��ļ�����ģʽ
pid_t ����ID�ͽ�����ID
ptrdiff_t ��һ�ִ��������ͣ����ڶ�����ָ��ִ�м�����������õĽ��
rlim_t ��Դ����
size_t ��ӳ�ڴ��ж���Ĵ�С�����ֽ�Ϊ��λ��
ssize_t �������ֽڼ����������ʾ�ĺ���ʹ��
time_t ����Ϊ��λ��ʱ
uid_t ��ֵ�û�ID
wchar_t �ܱ�ʾ���в�ͬ���ַ���
*/

#include <sys/socket.h>			//socket, bind, listen, accept, ...����Э���
/*
struct sockaddr{
sa_family_t   sa_family//��ַ��
char          sa_data[]//socket��ַ
}

struct msghdr{
void  *msg_name;  //��ѡ��ַ
socklen_t     msg_namelen;//��ַ��С
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

int     getpeername(int socket, struct sockaddr *address,socklen_t *address_len);//˫����������֮��,��ȡ�Է��׽��ֵ�ַ
int     getsockname(int socket, struct sockaddr *address,socklen_t *address_len);//�����׽��ֵ�ַ
int     getsockopt(int socket, int level, int option_name,void *option_value, socklen_t *option_len);//�����׽��ֵ�ѡ��
int     setsockopt(int socket, int level, int option_name,const void *option_value, socklen_t option_len);//�����׽���ѡ��
int     shutdown(int socket, int how); //�ر��׽��ֵĶ���д

*/
#include <arpa/inet.h>			//inet_addr, inet_ntoa, htonl, ...
/*

struct linger
{
int l_onoff;
int l_linger;
};
(1)l_onoff = 0;
l_linger����
���ַ�ʽ�£�������closesocket��ʱ�����̷��أ��ײ�Ὣδ����������ݷ�����ɺ����ͷ���Դ��Ҳ��
�����ŵ��˳���
(2)l_onoff����
l_linger = 0;
���ַ�ʽ�£��ڵ���closesocket��ʱ��ͬ�������̷��أ������ᷢ��δ������ɵ����ݣ�����ͨ��һ��REST��ǿ�ƵĹر�socket��������Ҳ����ǿ�Ƶ��˳���
(3)l_onoff����
l_linger > 0
���ַ�ʽ�£��ڵ���closesocket��ʱ�򲻻����̷��أ��ں˻��ӳ�һ��ʱ�䣬���ʱ�����l_linger��ֵ�������������ʱʱ�䵽��֮ǰ��������δ���͵�����(����FIN��)���õ���һ�˵�ȷ�ϣ�closesocket�᷵����ȷ��socket�������������˳�������closesocket��ֱ�ӷ��ش���ֵ��δ�������ݶ�ʧ��socket��������ǿ�����˳�����Ҫע���ʱ�����socket������������Ϊ�Ƕ����ͣ���closesocket��ֱ�ӷ���ֵ��

uint32_t htonl(uint32_t);  //32λ�����ֽ���ת��Ϊ�����ֽ��������ֽ����TCP/IPЭ����ô�����������ֽ������С������С��ַС�ֽ�
uint16_t htons(uint16_t);
uint32_t ntohl(uint32_t);  //�����෴
uint16_t ntohs(uint16_t);

const char* inet_ntop(int domain,const void* restrict addr,char * restrict str,socklen_t size);//�������Ƶ�ַת��Ϊ���ʮ�����ַ���
int inet_pton(int domain,const char * restrict str,void * restrict addr);//�����ʮ�����ַ���ת��Ϊ������


*/
#include <netinet/in.h>			//sockaddr_in��struct sockaddr, ...
/*
struct sockaddr_in
 
{
 
short sin_family;/*Address familyһ����˵AF_INET����ַ�壩PF_INET��Э���壩*/
 
unsigned short sin_port;/*Port number(����Ҫ�����������ݸ�ʽ,��ͨ���ֿ�����htons()����ת�����������ݸ�ʽ������)*/
 
struct in_addr sin_addr;/*IP address in network byte order��Internet address��*/
 
unsigned char sin_zero[8];/*Same size as struct sockaddrû��ʵ������,ֻ��Ϊ�ˡ���SOCKADDR�ṹ���ڴ��ж���*/
 
};

*/
#include <sys/ioctl.h>			//setsockopt
/*
setsockopt �����׽���ѡ��
*/
#include <unistd.h>				//close
/*
������ Unix ϵͳ��unistd.h ��������Ľӿ�ͨ�����Ǵ������ϵͳ���õķ�װ��Ӣ�wrapper functions������ fork��pipe �Լ����� I/O ԭ�read��write��close �ȵȣ�

*/
#include <pthread.h>			//pthread_creat, ...
/*
pthread_create()������һ���߳�
pthread_exit()����ֹ��ǰ�߳�
pthread_cancel()���ж�����һ���̵߳�����
pthread_join()��������ǰ���̣߳�ֱ������һ���߳����н���
pthread_attr_init()����ʼ���̵߳�����
pthread_attr_setdetachstate()����������״̬�����ԣ���������߳�����ֹʱ�Ƿ���Ա���ϣ�
pthread_attr_getdetachstate()����ȡ����״̬������
pthread_attr_destroy()��ɾ���̵߳�����
pthread_kill()�����̷߳���һ���ź�

ͬ������
pthread_mutex_init() ��ʼ��������
pthread_mutex_destroy() ɾ��������
pthread_mutex_lock()��ռ�л�����������������
pthread_mutex_trylock()����ͼռ�л�������������������������������������ʱ����ռ�и����������������ء�
pthread_mutex_unlock(): �ͷŻ�����
pthread_cond_init()����ʼ����������
pthread_cond_destroy()��������������
pthread_cond_signal(): ���ѵ�һ������pthread_cond_wait()������˯�ߵ��߳�
pthread_cond_wait(): �ȴ�����������������������
Thread-local storage��������Pthreads��������߳��������ݣ���
pthread_key_create(): �������ڱ�ʶ�������߳��ض����ݵļ�
pthread_setspecific(): Ϊָ���߳��ض����ݼ������߳��ض���
pthread_getspecific(): ��ȡ�����̵߳ļ��󶨣������ð󶨴洢�� value ָ���λ����
pthread_key_delete(): ���������߳��ض����ݼ�
pthread_attr_getschedparam();��ȡ�߳����ȼ�
pthread_attr_setschedparam();�����߳����ȼ�

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
#include <sys/time.h>  //������ƽ̨�й�
//#define NDEBUG
//#include <assert.h>
//#include <errno.h>
#define CONFIG	"config"//�����ļ���
#define LOGS	"pslogs.txt"//��־�ļ���
#define ETH	"eth"
#define XDEF	3  ngroups=sum�ܽڵ���/XDEF
#define GLEN	16   //����豸
#define MAXCONN 10   //���������
#define MAXSLEEP 8   //���˯��ʱ��
#define NMAXSTRIP 120//���ݷ�����
#define DPORT	29002  //�������˿�

#define SOCKBUFSZ (64*1024)  //�׽��ֻ�������С
#define BLKSZ	512   //���ݷֿ��С
#define MB	(1024*1024)  //M�ֽ�


//cluster node
typedef struct _cluster_node{   //��Ⱥ�ڵ�
	int num; //�ڵ���
	char devname[GLEN];  //�豸��
	char ip[GLEN];  //�ڵ�IP
	ushort port;  //�ڵ�port
}cnode;
//cluster config
typedef struct _config{  //��Ⱥ����
	int k,r,dk;  //RS�� k�����ݽڵ㣬r��У��ڵ�
	int ngroups;//���ݹ鵵��ˮ������Ŀ
	size_t dbsz;//���ݷֿ��С
	off_t stoff;//�����ݿ����ʼ��ַ
	cnode *p;//��Ⱥ����Ľڵ�
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
