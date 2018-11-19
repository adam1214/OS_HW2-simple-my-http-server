#include "client.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <sys/stat.h>

char *local_host;
char *port;
char a[129]="/";
int i; //for loop

struct node {
    char req[1025];
    struct node* next;
};

struct node* Q_HEAD = NULL;
struct node* Q_TAIL = NULL;
int Q_NUM = 0;

void Push(char d[],int d_len)
{
    if(Q_HEAD == NULL) {
        Q_HEAD = (struct node*)malloc(sizeof(struct node));
        for(i=0; i<d_len; i++) {
            Q_HEAD->req[i]=d[i];
        }
        Q_HEAD->req[d_len]='\0';
        Q_HEAD->next = NULL;
        Q_TAIL = Q_HEAD;
    } else {
        struct node* ptr = (struct node*)malloc(sizeof(struct node));
        for(i=0; i<d_len; i++) {
            ptr->req[i]=d[i];
        }
        ptr->req[d_len]='\0';
        ptr->next = NULL;
        Q_TAIL->next = ptr;
        Q_TAIL = ptr;
    }
    Q_NUM++;
}

struct node* Pop()
{
    struct node* ptr = Q_HEAD;
    //for(i=0;i<50;i++)
    //{
//      item[i]=ptr->data[i];
    // }
    Q_HEAD = ptr->next;
    //free(ptr);
    Q_NUM--;
    return ptr;
}

char s[50]= {'\0'};
char b[50]= {'\0'};
char c[50]= {'\0'};
char d[50]= {'\0'};
char e[50]= {'\0'};
char f[50]= {'\0'};
char g[50]= {'\0'};
char h[50]= {'\0'};
char aa[50]= {'\0'};
char bb[150]= {'\0'};
char create[1000]="./output";
char path[1000]="output";
int con=0;

void str_clear(char a[],int len)
{
    int i;
    for(i=0; i<len; i++) {
        a[i]='\0';
    }
}


void _mkdir(const char *path)
{
    //printf("%s\n",path);
    char temp[200];
    memset(temp,0,sizeof(temp));
    char* ptr = NULL;

    strncpy(temp, path, sizeof(temp));
    for(ptr = temp+2; *ptr; ptr++) {
        if(*ptr == '/') {
            *ptr = '\0';
            mkdir(temp, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
            *ptr = '/';
        }
    }
    mkdir(temp, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
}

void *runner(void *param)
{
    /*char s[50]={'\0'};
      char b[50]={'\0'};
      char c[50]={'\0'};
      char d[50]={'\0'};
      char e[50]={'\0'};
      char f[50]={'\0'};
      char g[50]={'\0'};
      char h[50]={'\0'};

      char aa[50]={'\0'};
      char bb[150]={'\0'};
      char create[1000]="./output";
      char path[1000]="output";
      int con=0;*/

    //set up socket
    int sockfd=0;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        printf("Fail to create a socket.");
    }

    //socket connect
    struct sockaddr_in info;
    bzero(&info,sizeof(info));
    info.sin_family = AF_INET;

    //localhost test
    info.sin_addr.s_addr = inet_addr(local_host);
    int port_i=atoi(port);
    info.sin_port = htons(port_i);

    int err = connect(sockfd,(struct sockaddr *)&info,sizeof(info));
    if(err==-1) {
        printf("Connection error");
    }

    char receiveMess[10000] = {'\0'};
    char message[5000]= {'\0'};
    struct node* ptr=Pop();
    sprintf(message,"GET %s HTTP/1.x\r\nHOST: %s:%s\r\n\r\n",ptr->req,local_host,port);
    //send(sockfd,message,sizeof(message),0);
    //recv(sockfd,receiveMessage,sizeof(receiveMessage),0);
    //printf("%s",receiveMessage);

    send(sockfd,message,sizeof(message),0);

    str_clear(aa,50);
    str_clear(bb,150);
    str_clear(create,1000);
    strcat(create,"./output");
    str_clear(path,1000);
    strcat(path,"output");
    con=0;
    sscanf(message,"%s %s",aa,bb);
    strcat(path,bb);
    char* q="/";
    char* p=NULL;
    p=strtok(bb,q);
    while(p!=NULL) {
        for(i=0; i<strlen(p); i++) {
            if(p[i]=='.') {
                con=1;
            }
        }
        if(con==0) {
            strcat(create,"/");
            strcat(create,p);
        }
        p=strtok(NULL,q);
    }

    recv(sockfd,receiveMess,sizeof(receiveMess),0);
    printf("%s",receiveMess);
    str_clear(s,50);
    str_clear(b,50);
    str_clear(c,50);
    str_clear(d,50);
    str_clear(e,50);
    str_clear(f,50);
    str_clear(g,50);
    str_clear(h,50);
    sscanf(receiveMess,"%s %s %s\r\n%s %s\r\n%s %s\r\n\r\n%s",s,b,c,d,e,f,g,h);
    if(strcmp(e,"text/html")==0||strcmp(e,"text/css")==0||strcmp(e,"text/x-h")==0||strcmp(e,"text/x-c")==0||strcmp(e,"application/json")==0) {
        char *file_content=NULL;
        file_content=strstr(receiveMess,"\r\n\r\n");
        for(i=4; i<strlen(file_content); i++) {
            file_content[i-4]=file_content[i];
        }
        file_content[strlen(file_content)-4]='\0';
        _mkdir(create);
        FILE *fp=fopen(path,"w+");
        fprintf(fp,"%s",file_content);
        fclose(fp);
    }

    pthread_exit(0);
}

int main(int argc, char *argv[])
{
    /*char s[50]={'\0'};
      char b[50]={'\0'};
      char c[50]={'\0'};
      char d[50]={'\0'};
      char e[50]={'\0'};
      char f[50]={'\0'};
      char g[50]={'\0'};
      char h[50]={'\0'};

      char aa[50]={'\0'};
      char bb[150]={'\0'};
      char create[1000]="./output";
      char path[1000]="output";
      int con=0;*/
    //set up socket
    int sockfd=0;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd == -1) {
        printf("Fail to create a socket.");
    }

    //socket connect
    struct sockaddr_in info;
    bzero(&info,sizeof(info));
    info.sin_family = AF_INET;

    //localhost test
    info.sin_addr.s_addr = inet_addr(argv[4]);
    int port_i=atoi(argv[6]);
    info.sin_port = htons(port_i);

    int err = connect(sockfd,(struct sockaddr *)&info,sizeof(info));
    if(err==-1) {
        printf("Connection error");
    }

    //Send a message to server
    //./client -t QUERY_FILE_OR_DIR -h LOCALHOST -p PORT
    char* file_dir=argv[2];
    local_host=argv[4];
    port=argv[6];
    char message[5000]= {'\0'};
    char receiveMessage[10000] = {'\0'};
    sprintf(message,"GET %s HTTP/1.x\r\nHOST: %s:%s\r\n\r\n",file_dir,local_host,port);
    if(strcmp(file_dir,"/")==0) {
        send(sockfd,message,sizeof(message),0);
        recv(sockfd,receiveMessage,sizeof(receiveMessage),0);
        printf("%s",receiveMessage);

        char *delim=" ";
        char *pch=NULL;
        pch=strtok(receiveMessage,delim);
        pch=strtok(NULL,delim);
        pch=strtok(NULL,delim);
        pch=strtok(NULL,delim);
        pch=strtok(NULL,delim);
        int num=0,j=0;
        while(pch!=NULL) {
            for(j=0; j<50; j++) {
                a[j]='\0';
            }
            a[0]='/';
            pch=strtok(NULL,delim);
            if(num==0) {
                for(j=4; j<strlen(pch); j++) {
                    pch[j-4]=pch[j];
                }
                pch[strlen(pch)-4]='\0';
                num++;
            }
            //printf("pch=%s\n",pch);
            if(pch!=NULL&&pch[0]!='\n') {
                strcat(a,pch); //a=/secfolder
                //printf("a=%s\n",a);
                Push(a,strlen(a));
                //pthread_t tid;
                //pthread_create(&tid,NULL,runner,(void*)&sockfd);
                //pthread_join(tid,NULL);
            }
        }

        while(Q_NUM>0) {
            pthread_t tid;
            pthread_create(&tid,NULL,runner,(void*)&sockfd);
            pthread_join(tid,NULL);
        }
    } else {
        //send(sockfd,message,sizeof(message),0);
        //recv(sockfd,receiveMessage,sizeof(receiveMessage),0);
        //printf("%s",receiveMessage);

        send(sockfd,message,sizeof(message),0);

        str_clear(aa,50);
        str_clear(bb,150);
        str_clear(create,1000);
        strcat(create,"./output");
        str_clear(path,1000);
        strcat(path,"output");
        con=0;
        sscanf(message,"%s %s",aa,bb);
        strcat(path,bb);
        char* q="/";
        char* p=NULL;
        p=strtok(bb,q);
        while(p!=NULL) {
            for(i=0; i<strlen(p); i++) {
                if(p[i]=='.') {
                    con=1;
                }
            }
            if(con==0) {
                strcat(create,"/");
                strcat(create,p);
            }
            p=strtok(NULL,q);
        }

        recv(sockfd,receiveMessage,sizeof(receiveMessage),0);
        printf("%s",receiveMessage);
        str_clear(s,50);
        str_clear(b,50);
        str_clear(c,50);
        str_clear(d,50);
        str_clear(e,50);
        str_clear(f,50);
        str_clear(g,50);
        str_clear(h,50);
        sscanf(receiveMessage,"%s %s %s\r\n%s %s\r\n%s %s\r\n\r\n%s",s,b,c,d,e,f,g,h);
        if(strcmp(e,"text/html")==0||strcmp(e,"text/css")==0||strcmp(e,"text/x-h")==0||strcmp(e,"text/x-c")==0||strcmp(e,"application/json")==0) {
            char *file_content=NULL;
            file_content=strstr(receiveMessage,"\r\n\r\n");
            for(i=4; i<strlen(file_content); i++) {
                file_content[i-4]=file_content[i];
            }
            file_content[strlen(file_content)-4]='\0';
            _mkdir(create);
            FILE *fp=fopen(path,"w+");
            fprintf(fp,"%s",file_content);
            fclose(fp);
        }
        //printf("%s",receiveMessage);

    }
    close(sockfd);
    return 0;
}
