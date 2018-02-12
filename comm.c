#include "comm.h"

//////////////////////////////////////////////////////////////////////////

cfg* readConfig(const char *name){ //���������ļ�->���ö���
	int i,n;//i=64
	FILE *fp;  //�����ļ�ָ��
	cnode *pn;  //pn�Ǽ�Ⱥ�ڵ�����
	off_t stb;//stb�Ƕ����ݿ����ʼλ��
	cfg *pc=NULL;   //���ö���
	if(NULL == (fp=fopen(name,"r"))){  //fopen��ֻ����ʽ���ļ�
		printf("Can't open file %s.\n",name);
		return NULL;
	}
	pc=(cfg *)malloc(sizeof(cfg));  //��̬����cfg����
	if(!pc)return NULL;
	fscanf(fp,"%d %d %d %ld %d %d",&(pc->k),&(pc->r),&(pc->dk),&stb,&i,&(pc->ngroups)); //fscanfɨ���ļ���ȡ����,ngroups=12
	n=pc->k+pc->r+pc->dk;  //��Ⱥ�����нڵ����
	pc->stoff=stb*BLKSZ;//�����ݵ���ʼ��ַ
	pc->dbsz=((size_t)i)*MB;//���ݷֿ��С64MB
	pc->p=(cnode *)malloc(n*sizeof(cnode));  //������Ⱥ�ڵ�
	memset(pc->p,0,n*sizeof(cnode)); //��ռ�Ⱥ�ڵ�
	for(i=0;i<n;i++){
		pn=pc->p+i;
		pn->num=i+1;  //��Ⱥ�ڵ����
		pn->port=DPORT; //��Ⱥ�ڵ�������˿ں�
		fscanf(fp,"%s %s",pn->ip,pn->devname); //����ڵ�IP���豸��
	}
	fclose(fp);
	return pc;   //���ؼ�Ⱥ���ö���
}
void freeConfig(cfg *pc){
	if(pc){
		if(pc->p)free(pc->p); //�ͷŽڵ����
		free(pc);
	}
}
//get ip address of ethx network card 
int getHostIPAddr(char *ip,int len){  //���ر���IP��ַ
	int i=0,sockfd=-1,tlen,r=0;
	if(!ip)return 1;  //������ڵ�IPΪNULL
	struct sockaddr_in *sip;
	/*
	struct sockaddr_in
{
 
short sin_family; //Address family一般来说AF_INET（地址族）PF_INET（协议族）
 
unsigned short sin_port;//Port number(必须要采用网络数据格式,普通数字可以用htons()函数转换成网络数据格式的数字)
 
struct in_addr sin_addr;//IP address in network byte order（Internet address）
 
unsigned char sin_zero[8];//Same size as struct sockaddr没有实际意义,只是为了　跟SOCKADDR结构在内存中对齐
 
};
	*/
	struct ifreq *irp; 
	/*
	struct ifreq//��������ĳ���ӿڵ���Ϣ
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
	struct ifconf ic;  //ifconf�������нӿ���Ϣ��ioctl��ñ���ip��ַʱҪ�õ������ṹ��ifconf��ifreq
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
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);  //SOCK_DGRAM�����ӣ����ɿ��׽���
	ioctl(sockfd, SIOCGIFCONF, &ic); //int ioctl(int handle, int cmd,[int *argdx, int argcx]);��ȡ���нӿ��嵥
	irp = (struct ifreq*)buf;
	for(i=(ic.ifc_len/sizeof(struct ifreq)); i>0; i--){//iΪ�ӿ�����
		sip=(struct sockaddr_in*)&(irp->ifr_addr);  //ÿ���ӿڵ�ַ
		iptemp=inet_ntoa(sip->sin_addr); //���ӿڵ�ַת��Ϊ���ʮ����
		tlen=1+strlen(iptemp); //�ַ����ܳ���
		memcpy(ethstr,irp->ifr_name,3); //memcpy(void *dest, const void *src, size_t n);irp->ifr_name��ֵ3�ֽڳ���ethstr(�ӿ���)
		if(0==strcmp(ETH,ethstr)){  //��ΪLinux����ӿڣ���iptempΪ����ӿ�ip
			if(len >= tlen)strncpy(ip,iptemp,tlen);
			else r=1;
			break;
		}
		irp++;  //����������ӿ������Ѱ��
	}
	close(sockfd);
	return r;  //0��ʾ�ɹ��ҵ�����ӿ�IP
}
cnode* getSpecNode(const cfg *pc,int num){  //������ŷ����ض��ڵ�0�������ݽڵ㣬1����У��ڵ�
	int i,n;
	cnode *pn;  //��ǰ�ڵ�
	char ip[GLEN]={'\0'};  //������ַ���ip
	if(!pc)return NULL;
	n=pc->k+pc->r+pc->dk;//�ܵĽڵ���   
	if(0==num){ //
		getHostIPAddr(ip,GLEN);  //���ر���ip
		for(i=0;i<n;i++){
			pn=pc->p+i;
			if(0==strcmp(ip,pn->ip))return pn; //�����뱾��ip��ͬ�Ľڵ�
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
   int l_linger; //若=0，则强制退出，在调用closesocket的时候同样会立刻返回，但不会发送未发送完成的数据
    };
	*/
	lg.l_onoff=1;
	lg.l_linger=5;//����closesock����ʱ�������̹ر�socket�����ǵȴ�5�����ڴ��ڼ��׽������ݴ�������������˳�������ǿ���˳�
	struct sockaddr_in snode;
	snode.sin_family = AF_INET;  //ipv4Э���
	snode.sin_port = htons(port);//�������ֽ���ת��Ϊ�����ֽ���
	snode.sin_addr.s_addr=inet_addr(ip);//���ַ���ת��Ϊ�����Ƶ�ַ��ʾ  	
	sockfd=socket(AF_INET,SOCK_STREAM,0);//�����׽���
	if((-1 != sockfd)){
		//printf("...\n");
		for(nsec=1;nsec <= MAXSLEEP;nsec <<= 1){
			if(0 == connect(sockfd,(struct sockaddr *)&snode,sizeof(snode))){//sockfd���ӷ�������ַ
				setsockopt(sockfd,SOL_SOCKET,SO_LINGER,(const char*)&lg,sizeof(struct linger));//int PASCAL FAR setsockopt(SOCKET s,int level,int optname,const char FAR *optval,int optlen);s�׽��������֣�levelѡ����Σ�optname���õ�ѡ�optvalָ����ѡ��ֵ�Ļ�������optlen����������
// 				setsockopt(sockfd,SOL_SOCKET,SO_SNDBUF,(const char*)&nsbufsz,sizeof(int));
// 				setsockopt(sockfd,SOL_SOCKET,SO_RCVBUF,(const char*)&nrbufsz,sizeof(int));
				return sockfd;  //���������׽���
			}
			if(nsec <= MAXSLEEP/2)sleep(nsec);
		}
		close(sockfd);
	}
	return -1;
}
//
void xorParity(char *d1,char * d2,int len){  // �������ݿ�d1,d2�����żУ��,len���ݿ鳤��
	int i,j;
	if(!d1 || !d2)return;
	long *pd1=(long *)d1;  //���ַ������ݿ�d1ת��Ϊ���������ݿ�pd1
	long *pd2=(long *)d2; //���ַ������ݿ�d2ת��Ϊ���������ݿ�pd2
	for(i=0; i < len/sizeof(long);i++) *(pd1+i)^= *(pd2+i); //�ж��ٳ��������ݿ飬����pd1��pd2�����������
	if(0 != len%sizeof(long)){
		for(j=i*sizeof(long);j<len;j++) *(d1+j)^= *(d2+j);  //��ʣ�����ݿ�����������
	}
}
//////////////////////////////////////////////////////////////////////////
//mode:0,read. 1,write. 
size_t diskIO(int fd,off_t offset,size_t nbytes,char *buf,int mode){//����I/O��д��0���ļ���1д�ļ�
	ssize_t once,total;
	if(-1!=lseek(fd,offset,SEEK_SET)){  //off_t lseek(int handle, off_t offset, int fromwhere);  //lseek()�����������Ƹ��ļ��Ķ�дλ�ã�SEEK_SET�ļ���дλ��ָ���ļ�ͷ������offsetλ����
		for(total=0;total<nbytes;total+=once){
			once = (!mode) ? read(fd,buf,nbytes) : write(fd,buf,nbytes);
			if(once<=0)break;
		}
	}
	return total; //totalΪ��д�ļ�ʵ���ֽ��� 
}
//mode:0,send. 1,recv.
size_t sockIO(int sockfd,char *buf,size_t len,int mode){
	size_t n,total=0;
	while(total<len){
		n = (!mode) ? send(sockfd,buf+total,len-total,0) : recv(sockfd,buf+total,len-total,0);
		if(-1 !=n)total += n;
		else break;
	}
	return total;   //ʵ��socket��д
}

//////////////////////////////////////////////////////////////////////////

