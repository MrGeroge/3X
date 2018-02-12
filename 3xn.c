#include "comm.h"

typedef struct _xparam{ //
	int n,flg,sock;
	pthread_mutex_t mutex;  //互斥量
	char *pbuf;  //缓冲区
}xparam;

//global variables
cfg *g_pc;//g_pc全局配置对象
cnode *g_pn;//等直前结点
pthread_attr_t g_attr;//全局线程属性
int *g_psocks;//全局套接字描述字组
xparam g_xparam[XDEF];//


//function declaration
int xnodeInit(void);
void xnodeClean(void);

void sendNode(void);
void *dataRead(void *p);
void *dataSend(void *p);

void recvNode(void);
void* portListen(void *p);
void *dataRecv(void *p);

void parityNode(void);
void *recvParity(void *p);
void *writeParity(void *p);
///
int main(int argc, char **argv){
	if(0!=xnodeInit())return 1;
	if(g_pn->num<=g_pc->k){
		if(g_pn->num%2)sendNode();  //发送节点
		else recvNode();  //接受节点
	}else parityNode();  //校验节点
	xnodeClean();  //节点释放
	return 0;
}
//////////////////////////////////////////////////////////////////////////
int xnodeInit(void){
	int i;
	setvbuf(stdout,NULL,_IONBF,0);//int setvbuf(FILE *stream, char *buf, int type, unsigned size);type _IONBF(无缓冲）：直接从流中读入数据或直接向流中写入数据，而没有缓冲区
	g_pc=readConfig(CONFIG); //得到全局配置对象
	g_pn=getSpecNode(g_pc,0);//得到当前结点
	if(!g_pn){
		printf("error config.\n");
		return 1;
	}else printf("local ip:%s\n",g_pn->ip);
	pthread_attr_init(&g_attr); //初始化分离属性
	pthread_attr_setdetachstate(&g_attr, PTHREAD_CREATE_DETACHED); //系统自动清除线程
	for(i=0;i<XDEF;i++)pthread_mutex_init(&g_xparam[i].mutex,NULL); //初始化XDEF个互斥量
	return 0;
}
void xnodeClean(void){
	int i=0;
	pthread_attr_destroy(&g_attr); //清除线程属性
	for(i=0;i<XDEF;i++)pthread_mutex_destroy(&g_xparam[i].mutex);//清除线程互斥量
	freeConfig(g_pc); //清除配置对象
}
//////////////////////////////////////////////////////////////////////////

void *dataRead(void *p){  //从设备文件中读数据
	int dev=*(int *)p;
	int cnt=0,i=0,f=0;
	while(1){
		if(0==pthread_mutex_trylock(&g_xparam[i].mutex)){  //对数据分块枷锁
			if(0==g_xparam[i].flg){
				diskIO(dev,g_pc->stoff+cnt*g_pc->dbsz,g_pc->dbsz,g_xparam[i].pbuf,0); //g_xparam[i].pbuf数据分块实际内容
				g_xparam[i].flg=1;
				cnt++;
				f=1;
			}
			pthread_mutex_unlock(&g_xparam[i].mutex);   //对数据分块解锁
			if(f){
				f=0;
				printf("read count:%d\n",cnt);
				if(cnt>=XDEF*g_pc->ngroups)break;
			}else usleep(1000);
		}
		if(XDEF==i+1)i=0;
		else i++;
	}
	printf("dataRead end.\n");
	return NULL;
}
void *dataSend(void *p){ //发送数据分块
	xparam *xp=(xparam *)p;//当前数据分块
	int cnt=0,f=0;
	while(1){
		if(0==pthread_mutex_lock(&xp->mutex)){
			if(1==xp->flg){
				sockIO(xp->sock,xp->pbuf,g_pc->dbsz,0);
				xp->flg=0;
				cnt++;
				f=1;
				
			}
			pthread_mutex_unlock(&xp->mutex);
			if(f){
				f=0;
				printf("%d send count:%d\n",xp->sock,cnt);
				if(cnt>=g_pc->ngroups)break;
			}else usleep(1000);
		}
	}
	return NULL;
}
//////////////////////////////////////////////////////////////////////////
void sendNode(void){
	int i,dev;
	char *buf;
	pthread_t tid[4];
	cnode *pnn=g_pn+1;//得到接受节点
	printf("Node %d starts.\n",g_pn->num);
	buf=(char *)malloc(XDEF*g_pc->dbsz);  //数据缓冲区
	dev=open(g_pn->devname,O_RDONLY);  //以只读方式打开节点文件
	for(i=0;i<XDEF;i++){
		g_xparam[i].flg=0;
		g_xparam[i].pbuf=buf+i*g_pc->dbsz;//第i块数据分块地址
		g_xparam[i].sock=connectTry(pnn->ip,pnn->port+(ushort)i);  //将接受节点作为服务器，返回连接套接字
		if(-1==g_xparam[i].sock){
			printf("nodeSend ~ connectTry() error.\n");
			while(i>=0)close(g_xparam[i--].sock);//关闭之前所有分块的套接字
			close(dev);
			free(buf);
			return;
		}
	}
	pthread_create(&tid[3],NULL,dataRead,&dev);//创建一个线程执行dataRead
	for(i=0;i<XDEF;i++)pthread_create(&tid[i],NULL,dataSend,&g_xparam[i]);//创建三个线程执行数据发送三个数据块
	for(i=0;i<=XDEF;i++)pthread_join(tid[i],NULL);//每个线程执行完成
	for(i=0;i<XDEF;i++)close(g_xparam[i].sock);  //关闭每个数据分块的套接字
	close(dev);
	free(buf);
	printf("Node %d end.\n",g_pn->num);
	return;
}

void recvNode(void){
	int dev,ary[XDEF]={0,1,2},i,j,k;
	pthread_t tid[4],t;
	char *buf;
	cnode *ppn=g_pc->p+g_pc->k;  //ppn是校验节点
	printf("Node %d starts.\n",g_pn->num);//g_pn指向接受节点
	dev=open(g_pn->devname,O_RDONLY);//以只读模式打开接受节点设备
	g_psocks=(int *)malloc(g_pc->r*XDEF*sizeof(int));  //校验数据分块套接字
	for(i=0;i<g_pc->r;i++){
		ppn+=i;  //ppn当前校验节点
		for(j=0;j<XDEF;j++){ //校验节点的数据分块
			*(g_psocks+i*XDEF+j)=connectTry(ppn->ip,ppn->port+j);  //获得校验数据块套接字
			if(-1==*(g_psocks+i*XDEF+j)){
				k=i*XDEF+j;  //校验数据分块偏移量
				printf("recvNode ~ connectTry() error.\n");
				while(k>=0)close(*(g_psocks+k--));
				close(dev);
				free(g_psocks);
				return;
			}
		}
	}
	buf=(char *)malloc(XDEF*g_pc->dbsz);  //XDEF个校验分块区
	for(i=0;i<XDEF;i++){  //数据分块区初始化
		g_xparam[i].n=i;
		g_xparam[i].flg=0;
		g_xparam[i].pbuf=buf+i*g_pc->dbsz;
		g_xparam[i].sock=-1;
	}
	
	pthread_create(&tid[3],NULL,dataRead,&dev);  //创建一个线程dataRead
	for(i=0;i<XDEF;i++){
		pthread_create(&t,NULL,portListen,&ary[i]);  //创建一个线程执行端口监听
		pthread_join(t,NULL);
		pthread_create(&tid[i],NULL,dataRecv,&g_xparam[i]);//创建一个线程执行dataRecv
	}
	for(i=0;i<4;i++)pthread_join(tid[i],NULL);
	
	close(dev);
	for(i=0;i<XDEF;i++)close(g_xparam[i].sock);
	for(i=0;i<g_pc->r*XDEF;i++)close(*(g_psocks+i));
	free(g_psocks);
	free(buf);
	printf("Node %d end.\n",g_pn->num);
	return;
}
void* portListen(void *p){
	int n=*(int *)p,ls,as,i=0;
	struct sockaddr_in sa;
	socklen_t slen=sizeof(sa);
	int opt=SO_REUSEADDR;
	//printf("portListen %d begin.\n",n);
	sa.sin_family=AF_INET;
	sa.sin_addr.s_addr=htonl(INADDR_ANY);
	sa.sin_port=htons(g_pn->port+(ushort)n);//接受节点端口，服务器地址
	ls=socket(AF_INET,SOCK_STREAM,0);
	setsockopt(ls,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));//SO_REUSEADDR 仅仅表示可以重用本地本地地址、本地端口
	bind(ls,(struct sockaddr *)&sa,sizeof(sa));
	if(listen(ls,10)){
		printf("portListen ~ listen() error.\n");
		return NULL;
	}
	while(1){
		as=accept(ls,(struct sockaddr *)&sa,&slen);  //连接套接字
		if(-1==as)printf("portListen ~ accept() error.\n");
		else{
			printf("%s connect to local port:%hu .\n",inet_ntoa(sa.sin_addr),g_pn->port+(ushort)n);
			if(g_pn->num<=g_pc->k){
				g_xparam[n].sock=as;  //数据分块的套接字
				break;
			}else{
				*(g_psocks+n*(g_pc->k/2)+i++)=as;
				if(i==g_pc->k/2)break;
				else continue;
			}
		}
	}
	return NULL;
}
void* dataRecv(void *p){
	xparam *xp=(xparam *)p;
	int cnt=0,f,i;
	char *buf=(char *)malloc(g_pc->dbsz);  //接受缓冲区
	while(1){
		if(g_pc->dbsz!=sockIO(xp->sock,buf,g_pc->dbsz,1))break; //sock接收端套接字，buf接受缓冲区，接受的数据块大小
		cnt++;
		printf("%d recv count:%d\n",xp->sock,cnt);
		for(f=0; ; ){
			if(0==pthread_mutex_lock(&xp->mutex)){  //对数据分块加锁
				if(1==xp->flg){
					xorParity(buf,xp->pbuf,g_pc->dbsz); //源数据分块与接受到的数据分块
					xp->flg=0;
					f=1;
				}
				pthread_mutex_unlock(&xp->mutex);   //对数据分块解锁
				if(f)break;
				else usleep(1000);
			}
		}
		for(i=0;i<g_pc->r;i++)sockIO(*(g_psocks+i*XDEF+xp->n),buf,g_pc->dbsz,0);	// send xor data to XDEF parity nodes
		if(cnt>=g_pc->ngroups)break;
	}
	free(buf);
	return NULL;
}
//////////////////////////////////////////////////////////////////////////
void parityNode(void){
	char *buf;
	int ary[XDEF]={0,1,2},i,dev;
	pthread_t tid[4];
	printf("Node %d starts.\n",g_pn->num);  //当前节点为校验节点
	g_psocks=(int *)malloc((g_pc->k/2)*XDEF*sizeof(int));//校验数据分区套接字
	buf=(char *)malloc(XDEF*g_pc->dbsz);  //校验数据分区
	dev=open(g_pn->devname,O_WRONLY);  //只写模式打开校验节点
	for(i=0;i<XDEF;i++){
		g_xparam[i].flg=0;
		g_xparam[i].pbuf=buf+i*g_pc->dbsz;
	}
	for(i=0;i<XDEF;i++)pthread_create(&tid[i],NULL,portListen,&ary[i]);  //创建一个线程执行端口监听
	for(i=0;i<XDEF;i++)pthread_join(tid[i],NULL);
	for(i=0;i<XDEF;i++)pthread_create(&tid[i],NULL,recvParity,&ary[i]);//创建一个线程执行接受校验数据
	pthread_create(&tid[3],NULL,writeParity,&dev);  //创建一个线程执行写校验数据
	for(i=0;i<=XDEF;i++)pthread_join(tid[i],NULL);
	for(i=0;i<(g_pc->k/2)*XDEF;i++)close(*(g_psocks+i));
	free(g_psocks);
	close(dev);
	free(buf);
	printf("Node %d end.\n",g_pn->num);
	return;
}
void *recvParity(void *p){
	int n=*(int *)p;//校验数据区
	int cnt=0,i,j,r,f;
	char *buf=(char *)malloc(2*g_pc->dbsz);//由于有两个校验数据过来
	while(1){
		for(i=0;i<g_pc->k/2;i++){
			r=sockIO(*(g_psocks+n*(g_pc->k/2)+i),(!i)?buf:(buf+g_pc->dbsz),g_pc->dbsz,1);
			if(r!=g_pc->dbsz)goto _LBEND;
			if(i>=1)xorParity(buf,buf+g_pc->dbsz,g_pc->dbsz);
		}
		for(j=0,f=0;;){
			if(0==pthread_mutex_lock(&g_xparam[j].mutex)){  //对数据区上锁
				if(0==g_xparam[j].flg){
					memcpy(g_xparam[n].pbuf,buf,g_pc->dbsz);
					g_xparam[j].flg=1;
					f=1;
				}
				pthread_mutex_unlock(&g_xparam[j].mutex);
				if(f)break;
				else usleep(1000);
			}
			if(XDEF==j+1)j=0;
			else j++;
		}
		cnt++;
		printf("%d recv count:%d\n",n,cnt);
		if(cnt>=g_pc->ngroups)break;
	}
_LBEND:
	if(r!=g_pc->dbsz)printf("parityRecv ~ sockIO() error.\n");
	free(buf);
	return NULL;
}
void *writeParity(void *p){
	int dev=*(int *)p;  //校验节点
	int i,cnt=0,f=0;
	for(i=0;;){
		if(0==pthread_mutex_trylock(&g_xparam[i].mutex)){
			if(1==g_xparam[i].flg){
				diskIO(dev,g_pc->stoff+cnt*g_pc->dbsz,g_pc->dbsz,g_xparam[i].pbuf,1);
				g_xparam[i].flg=0;
				cnt++;
				f=1;
			}
			pthread_mutex_unlock(&g_xparam[i].mutex);
			if(f){
				f=0;
				printf("write count:%d\n",cnt);
				if(cnt>=XDEF*g_pc->ngroups)break;
			}else usleep(1000);
		}
		if(XDEF==i+1)i=0;
		else i++;
	}
	return NULL;
}