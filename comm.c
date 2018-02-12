#include "comm.h"

//////////////////////////////////////////////////////////////////////////

cfg* readConfig(const char *name){ //¸ù¾İÅäÖÃÎÄ¼ş->ÅäÖÃ¶ÔÏó
	int i,n;//i=64
	FILE *fp;  //ÅäÖÃÎÄ¼şÖ¸Õë
	cnode *pn;  //pnÊÇ¼¯Èº½ÚµãÊı×é
	off_t stb;//stbÊÇ¶ÁÊı¾İ¿éµÄÆğÊ¼Î»ÖÃ
	cfg *pc=NULL;   //ÅäÖÃ¶ÔÏó
	if(NULL == (fp=fopen(name,"r"))){  //fopenÒÔÖ»¶Á·½Ê½´ò¿ªÎÄ¼ş
		printf("Can't open file %s.\n",name);
		return NULL;
	}
	pc=(cfg *)malloc(sizeof(cfg));  //¶¯Ì¬ÉêÇëcfg¶ÔÏó
	if(!pc)return NULL;
	fscanf(fp,"%d %d %d %ld %d %d",&(pc->k),&(pc->r),&(pc->dk),&stb,&i,&(pc->ngroups)); //fscanfÉ¨ÃèÎÄ¼ş¶ÁÈ¡²ÎÊı,ngroups=12
	n=pc->k+pc->r+pc->dk;  //¼¯ÈºÖĞËùÓĞ½Úµã¸öÊı
	pc->stoff=stb*BLKSZ;//·ÅÊı¾İµÄÆğÊ¼µØÖ·
	pc->dbsz=((size_t)i)*MB;//Êı¾İ·Ö¿é´óĞ¡64MB
	pc->p=(cnode *)malloc(n*sizeof(cnode));  //´´½¨¼¯Èº½Úµã
	memset(pc->p,0,n*sizeof(cnode)); //Çå¿Õ¼¯Èº½Úµã
	for(i=0;i<n;i++){
		pn=pc->p+i;
		pn->num=i+1;  //¼¯Èº½ÚµãĞòºÅ
		pn->port=DPORT; //¼¯Èº½Úµã·şÎñÆ÷¶Ë¿ÚºÅ
		fscanf(fp,"%s %s",pn->ip,pn->devname); //±£´æ½ÚµãIPÓëÉè±¸Ãû
	}
	fclose(fp);
	return pc;   //·µ»Ø¼¯ÈºÅäÖÃ¶ÔÏó
}
void freeConfig(cfg *pc){
	if(pc){
		if(pc->p)free(pc->p); //ÊÍ·Å½Úµã¶ÔÏó
		free(pc);
	}
}
//get ip address of ethx network card 
int getHostIPAddr(char *ip,int len){  //·µ»Ø±¾µØIPµØÖ·
	int i=0,sockfd=-1,tlen,r=0;
	if(!ip)return 1;  //Èô´«Èë½ÚµãIPÎªNULL
	struct sockaddr_in *sip;
	/*
	struct sockaddr_in
{
 
short sin_family; //Address familyä¸€èˆ¬æ¥è¯´AF_INETï¼ˆåœ°å€æ—ï¼‰PF_INETï¼ˆåè®®æ—ï¼‰
 
unsigned short sin_port;//Port number(å¿…é¡»è¦é‡‡ç”¨ç½‘ç»œæ•°æ®æ ¼å¼,æ™®é€šæ•°å­—å¯ä»¥ç”¨htons()å‡½æ•°è½¬æ¢æˆç½‘ç»œæ•°æ®æ ¼å¼çš„æ•°å­—)
 
struct in_addr sin_addr;//IP address in network byte orderï¼ˆInternet addressï¼‰
 
unsigned char sin_zero[8];//Same size as struct sockaddræ²¡æœ‰å®é™…æ„ä¹‰,åªæ˜¯ä¸ºäº†ã€€è·ŸSOCKADDRç»“æ„åœ¨å†…å­˜ä¸­å¯¹é½
 
};
	*/
	struct ifreq *irp; 
	/*
	struct ifreq//ÓÃÀ´±£´æÄ³¸ö½Ó¿ÚµÄĞÅÏ¢
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
	struct ifconf ic;  //ifconf±£´æËùÓĞ½Ó¿ÚĞÅÏ¢£¬ioctl»ñµÃ±¾µØipµØÖ·Ê±ÒªÓÃµ½Á½¸ö½á¹¹ÌåifconfºÍifreq
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
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);  //SOCK_DGRAMÎŞÁ¬½Ó£¬²»¿É¿¿Ì×½Ó×Ö
	ioctl(sockfd, SIOCGIFCONF, &ic); //int ioctl(int handle, int cmd,[int *argdx, int argcx]);»ñÈ¡ËùÓĞ½Ó¿ÚÇåµ¥
	irp = (struct ifreq*)buf;
	for(i=(ic.ifc_len/sizeof(struct ifreq)); i>0; i--){//iÎª½Ó¿Ú×ÜÊı
		sip=(struct sockaddr_in*)&(irp->ifr_addr);  //Ã¿¸ö½Ó¿ÚµØÖ·
		iptemp=inet_ntoa(sip->sin_addr); //½«½Ó¿ÚµØÖ·×ª»»Îªµã·ÖÊ®½øÖÆ
		tlen=1+strlen(iptemp); //×Ö·û´®×Ü³¤¶È
		memcpy(ethstr,irp->ifr_name,3); //memcpy(void *dest, const void *src, size_t n);irp->ifr_name¸³Öµ3×Ö½Ú³¤¶Èethstr(½Ó¿ÚÃû)
		if(0==strcmp(ETH,ethstr)){  //ÈôÎªLinuxÍøÂç½Ó¿Ú£¬ÔòiptempÎªÍøÂç½Ó¿Úip
			if(len >= tlen)strncpy(ip,iptemp,tlen);
			else r=1;
			break;
		}
		irp++;  //Èô²»ÊÇÍøÂç½Ó¿ÚÔò¼ÌĞøÑ°ÕÒ
	}
	close(sockfd);
	return r;  //0±íÊ¾³É¹¦ÕÒµ½ÍøÂç½Ó¿ÚIP
}
cnode* getSpecNode(const cfg *pc,int num){  //¸ù¾İĞòºÅ·µ»ØÌØ¶¨½Úµã0·µ»ØÊı¾İ½Úµã£¬1·µ»ØĞ£Ñé½Úµã
	int i,n;
	cnode *pn;  //µ±Ç°½Úµã
	char ip[GLEN]={'\0'};  //¶¨Òå¿Õ×Ö·û´®ip
	if(!pc)return NULL;
	n=pc->k+pc->r+pc->dk;//×ÜµÄ½ÚµãÊı   
	if(0==num){ //
		getHostIPAddr(ip,GLEN);  //·µ»Ø±¾µØip
		for(i=0;i<n;i++){
			pn=pc->p+i;
			if(0==strcmp(ip,pn->ip))return pn; //·µ»ØÓë±¾µØipÏàÍ¬µÄ½Úµã
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
   int l_linger; //è‹¥=0ï¼Œåˆ™å¼ºåˆ¶é€€å‡ºï¼Œåœ¨è°ƒç”¨closesocketçš„æ—¶å€™åŒæ ·ä¼šç«‹åˆ»è¿”å›ï¼Œä½†ä¸ä¼šå‘é€æœªå‘é€å®Œæˆçš„æ•°æ®
    };
	*/
	lg.l_onoff=1;
	lg.l_linger=5;//µ÷ÓÃclosesockº¯ÊıÊ±²»»áÁ¢¿Ì¹Ø±Õsocket£¬¶øÊÇµÈ´ı5£¬ÈôÔÚ´ËÆÚ¼äÌ×½Ó×ÖÊı¾İ´«ÊäÍê³ÉÔòÓÅÑÅÍË³ö£¬·ñÔòÇ¿ÖÆÍË³ö
	struct sockaddr_in snode;
	snode.sin_family = AF_INET;  //ipv4Ğ­Òé´Ø
	snode.sin_port = htons(port);//½«Ö÷»ú×Ö½ÚĞò×ª»¯ÎªÍøÂç×Ö½ÚĞò
	snode.sin_addr.s_addr=inet_addr(ip);//½«×Ö·û´®×ª»¯Îª¶ş½øÖÆµØÖ·±íÊ¾  	
	sockfd=socket(AF_INET,SOCK_STREAM,0);//´´½¨Ì×½Ó×Ö
	if((-1 != sockfd)){
		//printf("...\n");
		for(nsec=1;nsec <= MAXSLEEP;nsec <<= 1){
			if(0 == connect(sockfd,(struct sockaddr *)&snode,sizeof(snode))){//sockfdÁ¬½Ó·şÎñÆ÷µØÖ·
				setsockopt(sockfd,SOL_SOCKET,SO_LINGER,(const char*)&lg,sizeof(struct linger));//int PASCAL FAR setsockopt(SOCKET s,int level,int optname,const char FAR *optval,int optlen);sÌ×½Ó×ÖÃèÊö×Ö£¬levelÑ¡Ïî¶¨Òå²ã´Î£¬optnameÉèÖÃµÄÑ¡Ïî£¬optvalÖ¸Ïò´æ·ÅÑ¡ÏîÖµµÄ»º³åÇø£¬optlen»º³åÇø³¤¶È
// 				setsockopt(sockfd,SOL_SOCKET,SO_SNDBUF,(const char*)&nsbufsz,sizeof(int));
// 				setsockopt(sockfd,SOL_SOCKET,SO_RCVBUF,(const char*)&nrbufsz,sizeof(int));
				return sockfd;  //·µ»ØÁ¬½ÓÌ×½Ó×Ö
			}
			if(nsec <= MAXSLEEP/2)sleep(nsec);
		}
		close(sockfd);
	}
	return -1;
}
//
void xorParity(char *d1,char * d2,int len){  // ½«Á½Êı¾İ¿éd1,d2Òì»òÆæÅ¼Ğ£Ñé,lenÊı¾İ¿é³¤¶È
	int i,j;
	if(!d1 || !d2)return;
	long *pd1=(long *)d1;  //½«×Ö·û´®Êı¾İ¿éd1×ª»¯Îª³¤ÕûĞÎÊı¾İ¿épd1
	long *pd2=(long *)d2; //½«×Ö·û´®Êı¾İ¿éd2×ª»¯Îª³¤ÕûĞÎÊı¾İ¿épd2
	for(i=0; i < len/sizeof(long);i++) *(pd1+i)^= *(pd2+i); //ÓĞ¶àÉÙ³¤ÕûĞÎÊı¾İ¿é£¬²¢¶Ôpd1Óëpd2½øĞĞÒì»òÔËËã
	if(0 != len%sizeof(long)){
		for(j=i*sizeof(long);j<len;j++) *(d1+j)^= *(d2+j);  //¶ÔÊ£ÓàÊı¾İ¿é½øĞĞÒì»òÔËËã
	}
}
//////////////////////////////////////////////////////////////////////////
//mode:0,read. 1,write. 
size_t diskIO(int fd,off_t offset,size_t nbytes,char *buf,int mode){//´ÅÅÌI/O¶ÁĞ´£¬0¶ÁÎÄ¼ş£¬1Ğ´ÎÄ¼ş
	ssize_t once,total;
	if(-1!=lseek(fd,offset,SEEK_SET)){  //off_t lseek(int handle, off_t offset, int fromwhere);  //lseek()±ãÊÇÓÃÀ´¿ØÖÆ¸ÃÎÄ¼şµÄ¶ÁĞ´Î»ÖÃ£¬SEEK_SETÎÄ¼ş¶ÁĞ´Î»ÖÃÖ¸ÏòÎÄ¼şÍ·ºóÔö¼ÓoffsetÎ»ÒÆÁ¿
		for(total=0;total<nbytes;total+=once){
			once = (!mode) ? read(fd,buf,nbytes) : write(fd,buf,nbytes);
			if(once<=0)break;
		}
	}
	return total; //totalÎª¶ÁĞ´ÎÄ¼şÊµ¼Ê×Ö½ÚÊı 
}
//mode:0,send. 1,recv.
size_t sockIO(int sockfd,char *buf,size_t len,int mode){
	size_t n,total=0;
	while(total<len){
		n = (!mode) ? send(sockfd,buf+total,len-total,0) : recv(sockfd,buf+total,len-total,0);
		if(-1 !=n)total += n;
		else break;
	}
	return total;   //Êµ¼Êsocket¶ÁĞ´
}

//////////////////////////////////////////////////////////////////////////

