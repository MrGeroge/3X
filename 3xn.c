#include "comm.h"

typedef struct _xparam{ //
	int n,flg,sock;
	pthread_mutex_t mutex;  //������
	char *pbuf;  //������
}xparam;

//global variables
cfg *g_pc;//g_pcȫ�����ö���
cnode *g_pn;//�ȫ�ֱǰ���
pthread_attr_t g_attr;//ȫ���߳�����
int *g_psocks;//ȫ���׽�����������
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
		if(g_pn->num%2)sendNode();  //���ͽڵ�
		else recvNode();  //���ܽڵ�
	}else parityNode();  //У��ڵ�
	xnodeClean();  //�ڵ��ͷ�
	return 0;
}
//////////////////////////////////////////////////////////////////////////
int xnodeInit(void){
	int i;
	setvbuf(stdout,NULL,_IONBF,0);//int setvbuf(FILE *stream, char *buf, int type, unsigned size);type _IONBF(�޻��壩��ֱ�Ӵ����ж������ݻ�ֱ��������д�����ݣ���û�л�����
	g_pc=readConfig(CONFIG); //�õ�ȫ�����ö���
	g_pn=getSpecNode(g_pc,0);//�õ���ǰ���
	if(!g_pn){
		printf("error config.\n");
		return 1;
	}else printf("local ip:%s\n",g_pn->ip);
	pthread_attr_init(&g_attr); //��ʼ����������
	pthread_attr_setdetachstate(&g_attr, PTHREAD_CREATE_DETACHED); //ϵͳ�Զ�����߳�
	for(i=0;i<XDEF;i++)pthread_mutex_init(&g_xparam[i].mutex,NULL); //��ʼ��XDEF��������
	return 0;
}
void xnodeClean(void){
	int i=0;
	pthread_attr_destroy(&g_attr); //����߳�����
	for(i=0;i<XDEF;i++)pthread_mutex_destroy(&g_xparam[i].mutex);//����̻߳�����
	freeConfig(g_pc); //������ö���
}
//////////////////////////////////////////////////////////////////////////

void *dataRead(void *p){  //���豸�ļ��ж�����
	int dev=*(int *)p;
	int cnt=0,i=0,f=0;
	while(1){
		if(0==pthread_mutex_trylock(&g_xparam[i].mutex)){  //�����ݷֿ����
			if(0==g_xparam[i].flg){
				diskIO(dev,g_pc->stoff+cnt*g_pc->dbsz,g_pc->dbsz,g_xparam[i].pbuf,0); //g_xparam[i].pbuf���ݷֿ�ʵ������
				g_xparam[i].flg=1;
				cnt++;
				f=1;
			}
			pthread_mutex_unlock(&g_xparam[i].mutex);   //�����ݷֿ����
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
void *dataSend(void *p){ //�������ݷֿ�
	xparam *xp=(xparam *)p;//��ǰ���ݷֿ�
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
	cnode *pnn=g_pn+1;//�õ����ܽڵ�
	printf("Node %d starts.\n",g_pn->num);
	buf=(char *)malloc(XDEF*g_pc->dbsz);  //���ݻ�����
	dev=open(g_pn->devname,O_RDONLY);  //��ֻ����ʽ�򿪽ڵ��ļ�
	for(i=0;i<XDEF;i++){
		g_xparam[i].flg=0;
		g_xparam[i].pbuf=buf+i*g_pc->dbsz;//��i�����ݷֿ��ַ
		g_xparam[i].sock=connectTry(pnn->ip,pnn->port+(ushort)i);  //�����ܽڵ���Ϊ�����������������׽���
		if(-1==g_xparam[i].sock){
			printf("nodeSend ~ connectTry() error.\n");
			while(i>=0)close(g_xparam[i--].sock);//�ر�֮ǰ���зֿ���׽���
			close(dev);
			free(buf);
			return;
		}
	}
	pthread_create(&tid[3],NULL,dataRead,&dev);//����һ���߳�ִ��dataRead
	for(i=0;i<XDEF;i++)pthread_create(&tid[i],NULL,dataSend,&g_xparam[i]);//���������߳�ִ�����ݷ����������ݿ�
	for(i=0;i<=XDEF;i++)pthread_join(tid[i],NULL);//ÿ���߳�ִ�����
	for(i=0;i<XDEF;i++)close(g_xparam[i].sock);  //�ر�ÿ�����ݷֿ���׽���
	close(dev);
	free(buf);
	printf("Node %d end.\n",g_pn->num);
	return;
}

void recvNode(void){
	int dev,ary[XDEF]={0,1,2},i,j,k;
	pthread_t tid[4],t;
	char *buf;
	cnode *ppn=g_pc->p+g_pc->k;  //ppn��У��ڵ�
	printf("Node %d starts.\n",g_pn->num);//g_pnָ����ܽڵ�
	dev=open(g_pn->devname,O_RDONLY);//��ֻ��ģʽ�򿪽��ܽڵ��豸
	g_psocks=(int *)malloc(g_pc->r*XDEF*sizeof(int));  //У�����ݷֿ��׽���
	for(i=0;i<g_pc->r;i++){
		ppn+=i;  //ppn��ǰУ��ڵ�
		for(j=0;j<XDEF;j++){ //У��ڵ�����ݷֿ�
			*(g_psocks+i*XDEF+j)=connectTry(ppn->ip,ppn->port+j);  //���У�����ݿ��׽���
			if(-1==*(g_psocks+i*XDEF+j)){
				k=i*XDEF+j;  //У�����ݷֿ�ƫ����
				printf("recvNode ~ connectTry() error.\n");
				while(k>=0)close(*(g_psocks+k--));
				close(dev);
				free(g_psocks);
				return;
			}
		}
	}
	buf=(char *)malloc(XDEF*g_pc->dbsz);  //XDEF��У��ֿ���
	for(i=0;i<XDEF;i++){  //���ݷֿ�����ʼ��
		g_xparam[i].n=i;
		g_xparam[i].flg=0;
		g_xparam[i].pbuf=buf+i*g_pc->dbsz;
		g_xparam[i].sock=-1;
	}
	
	pthread_create(&tid[3],NULL,dataRead,&dev);  //����һ���߳�dataRead
	for(i=0;i<XDEF;i++){
		pthread_create(&t,NULL,portListen,&ary[i]);  //����һ���߳�ִ�ж˿ڼ���
		pthread_join(t,NULL);
		pthread_create(&tid[i],NULL,dataRecv,&g_xparam[i]);//����һ���߳�ִ��dataRecv
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
	sa.sin_port=htons(g_pn->port+(ushort)n);//���ܽڵ�˿ڣ���������ַ
	ls=socket(AF_INET,SOCK_STREAM,0);
	setsockopt(ls,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));//SO_REUSEADDR ������ʾ�������ñ��ر��ص�ַ�����ض˿�
	bind(ls,(struct sockaddr *)&sa,sizeof(sa));
	if(listen(ls,10)){
		printf("portListen ~ listen() error.\n");
		return NULL;
	}
	while(1){
		as=accept(ls,(struct sockaddr *)&sa,&slen);  //�����׽���
		if(-1==as)printf("portListen ~ accept() error.\n");
		else{
			printf("%s connect to local port:%hu .\n",inet_ntoa(sa.sin_addr),g_pn->port+(ushort)n);
			if(g_pn->num<=g_pc->k){
				g_xparam[n].sock=as;  //���ݷֿ���׽���
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
	char *buf=(char *)malloc(g_pc->dbsz);  //���ܻ�����
	while(1){
		if(g_pc->dbsz!=sockIO(xp->sock,buf,g_pc->dbsz,1))break; //sock���ն��׽��֣�buf���ܻ����������ܵ����ݿ��С
		cnt++;
		printf("%d recv count:%d\n",xp->sock,cnt);
		for(f=0; ; ){
			if(0==pthread_mutex_lock(&xp->mutex)){  //�����ݷֿ����
				if(1==xp->flg){
					xorParity(buf,xp->pbuf,g_pc->dbsz); //Դ���ݷֿ�����ܵ������ݷֿ�
					xp->flg=0;
					f=1;
				}
				pthread_mutex_unlock(&xp->mutex);   //�����ݷֿ����
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
	printf("Node %d starts.\n",g_pn->num);  //��ǰ�ڵ�ΪУ��ڵ�
	g_psocks=(int *)malloc((g_pc->k/2)*XDEF*sizeof(int));//У�����ݷ����׽���
	buf=(char *)malloc(XDEF*g_pc->dbsz);  //У�����ݷ���
	dev=open(g_pn->devname,O_WRONLY);  //ֻдģʽ��У��ڵ�
	for(i=0;i<XDEF;i++){
		g_xparam[i].flg=0;
		g_xparam[i].pbuf=buf+i*g_pc->dbsz;
	}
	for(i=0;i<XDEF;i++)pthread_create(&tid[i],NULL,portListen,&ary[i]);  //����һ���߳�ִ�ж˿ڼ���
	for(i=0;i<XDEF;i++)pthread_join(tid[i],NULL);
	for(i=0;i<XDEF;i++)pthread_create(&tid[i],NULL,recvParity,&ary[i]);//����һ���߳�ִ�н���У������
	pthread_create(&tid[3],NULL,writeParity,&dev);  //����һ���߳�ִ��дУ������
	for(i=0;i<=XDEF;i++)pthread_join(tid[i],NULL);
	for(i=0;i<(g_pc->k/2)*XDEF;i++)close(*(g_psocks+i));
	free(g_psocks);
	close(dev);
	free(buf);
	printf("Node %d end.\n",g_pn->num);
	return;
}
void *recvParity(void *p){
	int n=*(int *)p;//У��������
	int cnt=0,i,j,r,f;
	char *buf=(char *)malloc(2*g_pc->dbsz);//����������У�����ݹ���
	while(1){
		for(i=0;i<g_pc->k/2;i++){
			r=sockIO(*(g_psocks+n*(g_pc->k/2)+i),(!i)?buf:(buf+g_pc->dbsz),g_pc->dbsz,1);
			if(r!=g_pc->dbsz)goto _LBEND;
			if(i>=1)xorParity(buf,buf+g_pc->dbsz,g_pc->dbsz);
		}
		for(j=0,f=0;;){
			if(0==pthread_mutex_lock(&g_xparam[j].mutex)){  //������������
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
	int dev=*(int *)p;  //У��ڵ�
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