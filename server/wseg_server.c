#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <sys/resource.h>
#include <pthread.h>

#include "mseg.cpp"
using namespace std;
using namespace mingspy;




#define  OPRINT(para) (para)->output_bi+=sprintf(para->output_buf+para->output_bi  

#define	BUF_SIZE   204800 	
#define	TASK_NUM   1200


const char *http_head_text = "HTTP/1.1 200 OK\r\n\
Server: Apache/1.3.12p (Unix)\r\n\
Connection: close\r\n\
Content-type: text/plain; charset=utf-8\r\n\r\n";


FILE      *fp_log;

char currentdt[60], currentdate[30], currenttime[30];
int year, month, day, hour, minute, second;
void get_current_time()
{
	long tm;
	struct tm *ptm;
	time(&tm);
	ptm=localtime(&tm);
	
	year=ptm->tm_year+1900;
	month=ptm->tm_mon+1;
	day=ptm->tm_mday;
	hour=ptm->tm_hour;
	minute=ptm->tm_min;
	second=ptm->tm_sec;
	
	sprintf(currentdt, "%d-%02d-%02d %02d:%02d:%02d", ptm->tm_year+1900, ptm->tm_mon+1, ptm->tm_mday, ptm->tm_hour, ptm->tm_min, ptm->tm_sec);
	
	sprintf(currentdate, "%d-%02d-%02d", ptm->tm_year+1900, ptm->tm_mon+1, ptm->tm_mday);
	sprintf(currenttime, "%02d:%02d:%02d", ptm->tm_hour, ptm->tm_min, ptm->tm_sec);
}




char *substr(char *str, int len)
{
   int i;

   if( !str || !str[0] ) return str;
   for(i=0; i<strlen(str) && i<len; )
         if( (unsigned int)str[i] <= 127 ) i++; else i+=2;
   str[i]='\0';
   return str;

}



static int myread(int fd, void *buf, int len)
{
        int n;
	int bi;

	fd_set rfds;
	struct timeval tv;
	int retval;


	bi = 0;


    printf("\n\nmyread..............begin\n");

	while(1)
	{

		FD_ZERO(&rfds);
		FD_SET(fd, &rfds);

		tv.tv_sec = bi==0?2:0;
		tv.tv_usec = 2000;
	
		retval = select(fd+1, &rfds, NULL, NULL, &tv);
		if (retval == -1)
		{
			printf("retval == -1\n");
            printf("myread ------------------  end\n");
			return bi;
		}

        if(retval == 0)
		{
			printf("retval == 0\n");
            printf("myread ------------------  end\n");
			return bi;
		}
again:
	
        n = read(fd, buf+bi, len-bi);
        printf("read, n:%d, errno:%d\n", n, errno);

		if(n==0)
		{
			printf("n=0\n");
			fflush(stdout);
			break;
		}
        else if (n == -1)
        {
            switch (errno)
            {
                case EINTR:
                    goto again;
                    break;
                case EAGAIN:
                    printf("e_again_break\n");
                    perror("read");
                    fflush(stdout);
                    break;
                default:
                    printf("errno:%d\n", errno);
                    perror("read");
                    fflush(stdout);
                    break;

            }
        }
		else
		{
			bi += n;
		}
	}

	*((char *)(buf + bi)) = '\0';


    printf("myread ------------------  end\n");

    return bi;
}



static int mywrite(int fd, const void *buf, int len)
{
        int n;

	fd_set wfds;
	struct timeval tv;
	int retval;

	FD_ZERO(&wfds);
	FD_SET(fd, &wfds);
	tv.tv_sec = 1;
	tv.tv_usec = 0;

	retval = select(fd+1, NULL, &wfds, NULL, &tv);
	if (retval == -1)   return -1;
        if(retval == 0) return -1;
again:
        n = write(fd, buf, len);
        if (n == -1)
        {
                switch (errno)
                {
                case EINTR: goto again; break;
                case EAGAIN: n = 0; break;
                case EPIPE: return -2; break;
                }
        }
        return n;
}



char ConvertToHex(char *change)
{

        register char hexdigit;

        if(change[0]>='A')
        hexdigit=(change[0]&0xdf)-'A'+10;
        else  hexdigit=(change[0]&0xdf)-'0';

        hexdigit*=16;

        if(change[1]>='A')
        hexdigit+=change[1]-'A'+10;
        else  hexdigit+=change[1]-'0';
        return(hexdigit);
}

void urldecode(char *data)
{

        int i, j;

        if(!data || !data[0]) return;

        for(i=0; data[i]; i++) if(data[i]=='+') data[i]=' ';

        for (i=0,j=0; data[j]; ++i, ++j)
        {
                if((data[i]=data[j])=='%')
                {

                        data[i]=ConvertToHex(&data[j+1]);
                        j+=2;
                }
        }

        data[i]='\0';
}






//---------------------------------------------------------------------------


typedef struct stru_para
{

	int s;
	unsigned long addr;
	char ip[40];
	char *ip2;

	char *prog;
	char *query_para;
	char *ref_url;
	char *key;
	char *data;
	char null[1];

	int  output_bi;
	int  input_bi;
	int  bi;


	char input_buf[BUF_SIZE];
	char output_buf[BUF_SIZE];
	char buff[2*BUF_SIZE];
} PARA;



typedef struct  task_tag
{
	unsigned long addr;
	int s;
	int flag;
} TASK;


TASK tasks[TASK_NUM];
pthread_mutex_t	task_mutex=PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t log_mutex=PTHREAD_MUTEX_INITIALIZER;

pthread_cond_t  task_cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t	task_cond_mutex=PTHREAD_MUTEX_INITIALIZER;


int lock_mutex(pthread_mutex_t *p_mutex)
{
	int status;
	status = pthread_mutex_lock(p_mutex);
	if(status != 0)
	{
		printf("Error lock mutex.");
		exit(1);
	}
	return  0;
}

int unlock_mutex(pthread_mutex_t *p_mutex)
{
	int status;
	status = pthread_mutex_unlock(p_mutex);
	if(status != 0)
	{
		printf("Error unlock mutex.");
		exit(1);
	}
	return  0;
}


int lock_log()
{
        int status;
        status = pthread_mutex_lock(&log_mutex);
        if(status != 0)
        {
                printf("Error lock log_mutex.");
                exit(1);
        }
        return  0;
}

int unlock_log()
{
        int status;
        status = pthread_mutex_unlock(&log_mutex);
        if(status != 0)
        {
                printf("Error unlock log_mutex.");
                exit(1);
        }
        return  0;
}




int add_task(int s, unsigned long addr)
{
	int i;
	int status;
	pthread_t thread;
	void * req_main(void *);

	while(1)
	{

		for(i=1; i<TASK_NUM; i++)
		{
			if(tasks[i].flag == 0 )
			{
				tasks[i].s = s;
				tasks[i].addr = addr;
				tasks[i].flag = 1;


				printf("add task finish: %d\n", i);
				status = pthread_cond_signal(&task_cond);
				if(status != 0)
				{
					printf("Error wake on task_cond.\n");
					exit(1);
				}

				if( i > (TASK_NUM * 3 /4))
				{
					status = pthread_create(&thread, NULL, req_main, (void *)0);
					if(status != 0)
					{
						printf("Error create thread.\n");
						exit(1);
					}

				}

				return  i;
			}
		}



		printf("TASK_FULL\n");


		status = pthread_create(&thread, NULL, req_main, (void *)0);
		if(status != 0)
		{
			printf("Error create thread.\n");
			exit(1);
		}

		status = pthread_cond_broadcast(&task_cond);
		if(status != 0)
		{
			printf("Error wake on task_cond.\n");
			exit(1);
		}

	}

	return 0;
}


int find_task(PARA *para, int forever)
{
	int i;
	int status;

	while(1)
	{
		for(i=1; i<TASK_NUM; i++)
		{
			if(tasks[i].flag)
			{

				lock_mutex(&task_mutex);

				para->s = tasks[i].s;
				para->addr = tasks[i].addr;
				tasks[i].flag = 0;

				unlock_mutex(&task_mutex);

				printf("find task finish: %d\n", i);
				
				return  i;
			}
		}



		printf("task(%u) idle....\n\n", pthread_self());
		if(!forever) break;

		lock_mutex(&task_cond_mutex);
		status = pthread_cond_wait(&task_cond, &task_cond_mutex);
		if(status != 0)
		{
			printf("Error wait on task_cond.");
			exit(1);
		}
		unlock_mutex(&task_cond_mutex);

	}

	return 0;
}

//-------------------------------------------------------------------------





PARA *new_para()
{

	PARA *para;
	para = (PARA *)malloc(sizeof(PARA));
	memset(para, 0, BUF_SIZE);

	OPRINT(para), "%s", http_head_text);
	return para;
}

int free_para(PARA *para)
{
	free(para);
	return 0;
}



int req_flush(PARA *para)
{



	if(para->output_bi) mywrite(para->s, para->output_buf, para->output_bi);
//	printf("%s\n", para->output_buf);
	para->output_bi = 0;
	return 0;
}


int parse_para(PARA *para)
{
	char *p, *begin, *end;
	char *post_data;

	int get_flag = 1;



	begin =  para->input_buf;
	end = begin + para->input_bi;


	get_flag = 1;

	p = strstr(begin, "GET ");
	if(p != begin )
	{
		p = strstr(begin, "POST ");
		get_flag = 0;
	}
	if(p != begin ) return -1;
	

	p += get_flag? strlen("GET ") : strlen("POST ");
	if(!get_flag)
	{
		post_data = strstr(para->input_buf, "\r\n\r\n");
		if(post_data)
		{
			post_data += 4;
		}
		else
		{
			post_data = strstr(para->input_buf, "\n\n");
			if(post_data)
			{
				post_data += 2;
			}
			else
			{
				return -1;
			}
		}
	}

	while(*p == ' ' && p<end) p++;
	if(p>=end) return -1;
	
	para->prog = p;

	while(*p != '?' && *p != ' ' && *p!= '\r' && *p != '\n' && p<end) p++;
	if(p>=end) 
	{
		return -1;
	}

	if(*p == '?')
	{
		para->query_para = p+1;
		*p = '\0';
	}
	else if (*p == ' ')
	{
		para->query_para = para->null;
		*p = '\0';
	}
	else if (*p == '\r' || *p == '\n' )
	{
		para->query_para = para->null;
		*p = '\0';
	}
	else
	{
		return -1;
	}

	while(*p != ' ' && *p!= '\r' && *p != '\n' && p<end) p++;
	if(p>=end) 
	{
		return -1;
	}

	*p = '\0';

	//printf("para: %s\n", para->query_para);

	p = strstr(p+1, "\nReferer: ");
	if( p == NULL)
	{
		para->ref_url = NULL;
	}
	else
	{
		p += strlen("\nReferer: ");
		para->ref_url = p;
		while( *p!= '\r' && *p != '\n' && p<end) p++;
		*p = '\0';
	}



	para->ip2 = strstr(para->query_para, "ip=");
	para->key = strstr(para->query_para, "key=");

	if(para->ip2)
	{
		 para->ip2 += 3;
		 p = strchr(para->ip2, '&');
		 if(p) *p = '\0';
	}

	if(para->key)
	{
		para->key +=4;
		p = strchr(para->key,'&');
		if(p) *p = '\0';
	}


	urldecode(para->key);

	if(post_data)
	{
		para->data = strstr(post_data, "data=");
		if(para->data)
		{
			para->data += strlen("data=");
			for(p=para->data; *p && *p != '&'; ) p++;
			*p = '\0';
			urldecode(para->data);
		}
		
	}

	return 0;
}


int check_para(PARA *para)
{
	if(!para->data)
	{
		//OPRINT(para), "");
		return -1;
	}
	return 0;
}




int  req_finish(PARA *para)
{
	req_flush(para);
	close(para->s);
	return 0;
}



void *para_malloc(PARA *para, int bz)
{

	if( (para->bi + bz ) > BUF_SIZE )
	{
		 printf("para outof memory,  bi:%d, bz:%d!\n", para->bi,  bz);
		 fflush(stdout);
		 return (void *)0;
	}

	para->bi += bz;

//	printf("memory,  bi:%d, bz:%d!\n", para->bi,  bz);
	return (void *)(para->buff + para->bi);
}



char **split(PARA *para, char *mark, char *str, int max, int *count)
{
	
	char **array;
	char *p1, *p2;
	int num, i, j;
	char *buff;
	
	for(i=0, p1=p2=str; p2; i++)
	{
		p2=strstr(p1, mark);
		if(!p2) break;
		p1=p2+strlen(mark);
	}
	
	array= (char **)para_malloc(para, (i+2)*sizeof(char **));

	
	for(j=0; j<=i+1; j++) array[j]=0;
	
	
	buff=(char *)para_malloc(para, strlen(str)+1);

	strcpy(buff, str);
	
	for(i=0, p1=p2=buff; p2; i++)
	{
		array[i]=p1;
		if(max && i==max) break;
		
		p2=strstr(p1, mark);
		
		if(!p2) break;
		*p2='\0';
		p1=p2+strlen(mark);
		
		
	}
	
	if(count) *count=i+1;
	return array;
}



//==========================================================================


void *req_main(void *arg)
{

	struct timeval tpstart,tpend;
	float timeuse;

	struct in_addr tmp_addr;
	long forever;
	PARA *para;


	int bi, len;
	char result[256];

    char *begin, *end;
    char theword[256];
	int flag;

    Token tokens[1000];

	pthread_detach(pthread_self());

	forever = (long)arg;
	para = new_para();

	while(1)
	{

		memset(para, 0, 1024);
		OPRINT(para), "%s", http_head_text);
		if( find_task(para, forever) == 0)
		{
			printf("find nothing.\n");
			break;
		}
	
		get_current_time();

		gettimeofday(&tpstart,NULL);
	
		tmp_addr.s_addr = para->addr;
		strcpy(para->ip, inet_ntoa(tmp_addr));


		memset(para->input_buf, '\0', BUF_SIZE);

		para->input_bi = myread(para->s, para->input_buf, BUF_SIZE);

		printf("para->input_bi: %d\n", para->input_bi);
		printf("input_buf: %s\n", para->input_buf);
		
		if(para->input_bi < 0)
		{
			perror("read");
			fprintf(stderr, "%s[%s](%d)\n", para->ip?para->ip:"-", strerror(errno), errno);
			req_finish(para);
			continue;
		}


		parse_para(para);
	
		if(check_para(para) < 0)
		{
			req_finish(para);
			continue;
		}


        int len = mseg_backward_split(para->data, tokens, 1000);
        print(para->data, tokens,len);
        for(int ii=0; ii<len; ii++)
        {
            char tbuf[1024];
            int start = tokens[ii].start;
            int end = tokens[ii].end;
            int tlen = end - start;

            memcpy(tbuf, para->data + start, tlen);
            tbuf[tlen] = '\0';
            OPRINT(para), "%s\n", tbuf);
        }



		req_flush(para);
	
		gettimeofday(&tpend,NULL);
	        timeuse = 1000000*(tpend.tv_sec-tpstart.tv_sec);
	        timeuse = timeuse+tpend.tv_usec-tpstart.tv_usec;
	        timeuse=timeuse/1000000;
	
		
		lock_log();

		if(fp_log)
		{
			fprintf(fp_log, "%s\t%f\t%s\t%s\n",
			currentdt, timeuse, para->ip, "-");
		}

		unlock_log();

		req_finish(para);

	}

	free(para);
	return (void *)0;
}






int main(int argc, char **argv)
{

	int status;
	pthread_t   thread;

    socklen_t cli_size;

	int i;

    struct sockaddr_in addr;
    int listen_fd = -1;
    int on = 1;

	char *ip;
	int port;

	struct rlimit rl;
	
    if(argc<4)
    {
        printf("%s <ip> <port> <access log>\n", argv[0]);
        return 0;
    }

	ip = argv[1];
	port = atoi(argv[2]);

	fp_log = fopen(argv[3], "a");
	if(!fp_log) fp_log = fopen(argv[3], "w");

	signal(SIGPIPE, SIG_IGN);

	rl.rlim_cur = 20480;
	rl.rlim_max = 20480;

	if( setrlimit(RLIMIT_NOFILE, &rl) < 0)
	{
		perror("setrlimit");
		return 0;
	}


    mseg_init();



	for(i=0; i<TASK_NUM; i++)  memset(&tasks[i], 0, sizeof(TASK));

	for(i=0; i<20; i++)
	{

		status = pthread_create(&thread, NULL, req_main, (void *)1);

		if(status != 0)
		{
			printf("Error create thread.\n");
			return 0;
		}
	}


        memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        inet_pton(AF_INET, ip, &addr.sin_addr);
        addr.sin_port = htons(port);

        listen_fd = socket(PF_INET, SOCK_STREAM, 0);
        if (listen_fd == -1)
	{
		perror("socket");
		return 0;
	}

        setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

    if (bind(listen_fd, (struct sockaddr *)&addr, sizeof(addr)) == -1)
	{
		perror("bind");
		return 0;
	}
    if (listen(listen_fd, 16) == -1)
	{
		perror("listen");
		return 0;
	}


    listen(listen_fd, 0);


    printf("listen at %s:%d\n\n", ip, port);
    cli_size = sizeof(struct sockaddr_in);

    while(1)
    {
        int  t;
		struct sockaddr_in  cli_addr;

        t=accept(listen_fd, (struct sockaddr *)&cli_addr, &cli_size);
        if(t<0)
        {
			perror("accept");
            fprintf(stderr, "\n");
			return 0;
        }

		add_task(t, cli_addr.sin_addr.s_addr);
    }
	return 0;
}


