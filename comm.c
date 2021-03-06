#include "comm.h"

//////////////////////////////////////////////////////////////////////////

cfg* readConfig(const char *name){ //根据配置文件->配置对象
	int i,n;//i=64
	FILE *fp;  //配置文件指针
	cnode *pn;  //pn是集群节点数组
	off_t stb;//stb是读数据块的起始位置
	cfg *pc=NULL;   //配置对象
	if(NULL == (fp=fopen(name,"r"))){  //fopen以只读方式打开文件
		printf("Can't open file %s.\n",name);
		return NULL;
	}
	pc=(cfg *)malloc(sizeof(cfg));  //动态申请cfg对象
	if(!pc)return NULL;
	fscanf(fp,"%d %d %d %ld %d %d",&(pc->k),&(pc->r),&(pc->dk),&stb,&i,&(pc->ngroups)); //fscanf扫描文件读取参数,ngroups=12
	n=pc->k+pc->r+pc->dk;  //集群中所有节点个数
	pc->stoff=stb*BLKSZ;//放数据的起始地址
	pc->dbsz=((size_t)i)*MB;//数据分块大小64MB
	pc->p=(cnode *)malloc(n*sizeof(cnode));  //创建集群节点
	memset(pc->p,0,n*sizeof(cnode)); //清空集群节点
	for(i=0;i<n;i++){
		pn=pc->p+i;
		pn->num=i+1;  //集群节点序号
		pn->port=DPORT; //集群节点服务器端口号
		fscanf(fp,"%s %s",pn->ip,pn->devname); //保存节点IP与设备名
	}
	fclose(fp);
	return pc;   //返回集群配置对象
}
void freeConfig(cfg *pc){
	if(pc){
		if(pc->p)free(pc->p); //释放节点对象
		free(pc);
	}
}
//get ip address of ethx network card 
int getHostIPAddr(char *ip,int len){  //返回本地IP地址
	int i=0,sockfd=-1,tlen,r=0;
	if(!ip)return 1;  //若传入节点IP为NULL
	struct sockaddr_in *sip;
	/*
	struct sockaddr_in
{
 
short sin_family; //Address family涓�鑸潵璇碅F_INET锛堝湴鍧�鏃忥級PF_INET锛堝崗璁棌锛�
 
unsigned short sin_port;//Port number(蹇呴』瑕侀噰鐢ㄧ綉缁滄暟鎹牸寮�,鏅�氭暟瀛楀彲浠ョ敤htons()鍑芥暟杞崲鎴愮綉缁滄暟鎹牸寮忕殑鏁板瓧)
 
struct in_addr sin_addr;//IP address in network byte order锛圛nternet address锛�
 
unsigned char sin_zero[8];//Same size as struct sockaddr娌℃湁瀹為檯鎰忎箟,鍙槸涓轰簡銆�璺烻OCKADDR缁撴瀯鍦ㄥ唴瀛樹腑瀵归綈
 
};
	*/
	struct ifreq *irp; 
	/*
	struct ifreq//用来保存某个接口的信息
{
 
  char ifr_name[IFNAMSIZ];  
 union {
  struct sockaddr ifr_addr;
  struct sockaddr ifr_dstaddr;
  struct sockaddr ifr_broadaddr;
  struct sockaddr ifr_netmask;
  struct  sockaddr ifr_hwaddr;
  short ifr_flags;
  int ifr_ivalue;
  int ifr_mtu;
  struct  ifmap ifr_map;
  char ifr_slave[IFNAMSIZ]; 
  char ifr_newname[IFNAMSIZ];
  void __user * ifr_data;
  struct if_settings ifr_settings;
 } ifr_ifr;
};
struct sockaddr {
unsigned short sa_family; //address family, AF_xxx 
char sa_data[14]; // 14 bytes of protocol address
};
	*/
	struct ifconf ic;  //ifconf保存所有接口信息，ioctl获得本地ip地址时要用到两个结构体ifconf和ifreq
	/*
	struct ifconf 
{
    int    ifc_len;            //size of buffer
    union 
    {
        char *ifc_buf;                        //input from user->kernel
        struct ifreq *ifc_req;        //return from kernel->user
    } ifc_ifcu;

};
	*/
	char buf[512],*iptemp,ethstr[4]={'\0'};
	ic.ifc_len = 512;
	ic.ifc_buf = buf;
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);  //SOCK_DGRAM无连接，不可靠套接字
	ioctl(sockfd, SIOCGIFCONF, &ic); //int ioctl(int handle, int cmd,[int *argdx, int argcx]);获取所有接口清单
	irp = (struct ifreq*)buf;
	for(i=(ic.ifc_len/sizeof(struct ifreq)); i>0; i--){//i为接口总数
		sip=(struct sockaddr_in*)&(irp->ifr_addr);  //每个接口地址
		iptemp=inet_ntoa(sip->sin_addr); //将接口地址转换为点分十进制
		tlen=1+strlen(iptemp); //字符串总长度
		memcpy(ethstr,irp->ifr_name,3); //memcpy(void *dest, const void *src, size_t n);irp->ifr_name赋值3字节长度ethstr(接口名)
		if(0==strcmp(ETH,ethstr)){  //若为Linux网络接口，则iptemp为网络接口ip
			if(len >= tlen)strncpy(ip,iptemp,tlen);
			else r=1;
			break;
		}
		irp++;  //若不是网络接口则继续寻找
	}
	close(sockfd);
	return r;  //0表示成功找到网络接口IP
}
cnode* getSpecNode(const cfg *pc,int num){  //根据序号返回特定节点0返回数据节点，1返回校验节点
	int i,n;
	cnode *pn;  //当前节点
	char ip[GLEN]={'\0'};  //定义空字符串ip
	if(!pc)return NULL;
	n=pc->k+pc->r+pc->dk;//总的节点数   
	if(0==num){ //
		getHostIPAddr(ip,GLEN);  //返回本地ip
		for(i=0;i<n;i++){
			pn=pc->p+i;
			if(0==strcmp(ip,pn->ip))return pn; //返回与本地ip相同的节点
		}
		return NULL;
	}else return pc->p+num-1;
}

int connectTry(char *ip,ushort port){
	int nsec,sockfd;
//	int nsbufsz=SOCKBUFSZ,nrbufsz=SOCKBUFSZ;
	struct linger lg;
	/*
	struct linger
    {
   int l_onoff;//
   int l_linger; //鑻�=0锛屽垯寮哄埗閫�鍑猴紝鍦ㄨ皟鐢╟losesocket鐨勬椂鍊欏悓鏍蜂細绔嬪埢杩斿洖锛屼絾涓嶄細鍙戦�佹湭鍙戦�佸畬鎴愮殑鏁版嵁
    };
	*/
	lg.l_onoff=1;
	lg.l_linger=5;//调用closesock函数时不会立刻关闭socket，而是等待5，若在此期间套接字数据传输完成则优雅退出，否则强制退出
	struct sockaddr_in snode;
	snode.sin_family = AF_INET;  //ipv4协议簇
	snode.sin_port = htons(port);//将主机字节序转化为网络字节序
	snode.sin_addr.s_addr=inet_addr(ip);//将字符串转化为二进制地址表示  	
	sockfd=socket(AF_INET,SOCK_STREAM,0);//创建套接字
	if((-1 != sockfd)){
		//printf("...\n");
		for(nsec=1;nsec <= MAXSLEEP;nsec <<= 1){
			if(0 == connect(sockfd,(struct sockaddr *)&snode,sizeof(snode))){//sockfd连接服务器地址
				setsockopt(sockfd,SOL_SOCKET,SO_LINGER,(const char*)&lg,sizeof(struct linger));//int PASCAL FAR setsockopt(SOCKET s,int level,int optname,const char FAR *optval,int optlen);s套接字描述字，level选项定义层次，optname设置的选项，optval指向存放选项值的缓冲区，optlen缓冲区长度
// 				setsockopt(sockfd,SOL_SOCKET,SO_SNDBUF,(const char*)&nsbufsz,sizeof(int));
// 				setsockopt(sockfd,SOL_SOCKET,SO_RCVBUF,(const char*)&nrbufsz,sizeof(int));
				return sockfd;  //返回连接套接字
			}
			if(nsec <= MAXSLEEP/2)sleep(nsec);
		}
		close(sockfd);
	}
	return -1;
}
//
void xorParity(char *d1,char * d2,int len){  // 将两数据块d1,d2异或奇偶校验,len数据块长度
	int i,j;
	if(!d1 || !d2)return;
	long *pd1=(long *)d1;  //将字符串数据块d1转化为长整形数据块pd1
	long *pd2=(long *)d2; //将字符串数据块d2转化为长整形数据块pd2
	for(i=0; i < len/sizeof(long);i++) *(pd1+i)^= *(pd2+i); //有多少长整形数据块，并对pd1与pd2进行异或运算
	if(0 != len%sizeof(long)){
		for(j=i*sizeof(long);j<len;j++) *(d1+j)^= *(d2+j);  //对剩余数据块进行异或运算
	}
}
//////////////////////////////////////////////////////////////////////////
//mode:0,read. 1,write. 
size_t diskIO(int fd,off_t offset,size_t nbytes,char *buf,int mode){//磁盘I/O读写，0读文件，1写文件
	ssize_t once,total;
	if(-1!=lseek(fd,offset,SEEK_SET)){  //off_t lseek(int handle, off_t offset, int fromwhere);  //lseek()便是用来控制该文件的读写位置，SEEK_SET文件读写位置指向文件头后增加offset位移量
		for(total=0;total<nbytes;total+=once){
			once = (!mode) ? read(fd,buf,nbytes) : write(fd,buf,nbytes);
			if(once<=0)break;
		}
	}
	return total; //total为读写文件实际字节数 
}
//mode:0,send. 1,recv.
size_t sockIO(int sockfd,char *buf,size_t len,int mode){
	size_t n,total=0;
	while(total<len){
		n = (!mode) ? send(sockfd,buf+total,len-total,0) : recv(sockfd,buf+total,len-total,0);
		if(-1 !=n)total += n;
		else break;
	}
	return total;   //实际socket读写
}

//////////////////////////////////////////////////////////////////////////

