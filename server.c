#include "server.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <dirent.h>
#include <sys/stat.h>

static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
char* target=NULL; // dir/file needed to find
DIR *d;
struct dirent* dir;
int find_dir_con=0;
int find_con=0;
int i; //for loop
int thread_cnt;

struct node {
    char data[1025];
    struct node* next;
    char target[1025];
    int forClientSockfd;
};

struct thread_member {
    pthread_t thread;
    int alive;
};

struct thread_member* thread_pool;
struct node* Q_HEAD = NULL;
struct node* Q_TAIL = NULL;
int Q_NUM = 0;
char item[50]; // ./testdir/exsmple.html
char sub_item[50]; // testdir/example.html
char* Pop_s=NULL;
int sup_type=1;
char Pop_ss[50];

struct stat sb; //use for identifying file or dir

char file_content[10000];
char ch;
FILE *file;

void str_clear(char a[],int len)
{
    int i;
    for(i=0; i<len; i++) {
        a[i]='\0';
    }
}


void Push(char d[],int d_len,char t[],int t_len,int forClientSockfd)
{
    if(Q_HEAD == NULL) {
        Q_HEAD = (struct node*)malloc(sizeof(struct node));
        for(i=0; i<d_len; i++) {
            Q_HEAD->data[i]=d[i];
        }
        for(i=0; i<t_len; i++) {
            Q_HEAD->target[i]=t[i];
        }
        Q_HEAD->data[d_len]='\0';
        Q_HEAD->target[t_len]='\0';
        Q_HEAD->forClientSockfd=forClientSockfd;
        Q_HEAD->next = NULL;
        Q_TAIL = Q_HEAD;
    } else {
        struct node* ptr = (struct node*)malloc(sizeof(struct node));
        for(i=0; i<d_len; i++) {
            ptr->data[i]=d[i];
        }
        for(i=0; i<t_len; i++) {
            ptr->target[i]=t[i];
        }
        ptr->data[d_len]='\0';
        ptr->target[t_len]='\0';
        ptr->forClientSockfd=forClientSockfd;
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
//	item[i]=ptr->data[i];
    // }
    Q_HEAD = ptr->next;
    //free(ptr);
    Q_NUM--;
    return ptr;
}

int isEmpty(void)
{
    if(Q_NUM == 0) return 1;
    else return 0;
}

void find(DIR *d,char* message,struct node* ptr)
{
    //printf("ptr->data=%s\n",ptr->data);
    //printf("ptr->target=%s\n",ptr->target);
    if(d) {
        //printf("Into if statement of find function\n");
        while((dir = readdir(d))!=NULL) {
            //printf("Into while statement of find function\n");
            if(strcmp(dir->d_name,".") && strcmp(dir->d_name,"..")) {
                //printf("%s\n", dir->d_name);
                if(find_dir_con==1) { //print ls to message
                    strcat(message,"HTTP/1.x 200 OK\r\n");
                    strcat(message,"Content-type: directory\r\n");
                    strcat(message,"Server: httpserver/1.x \r\n\r\n");
                    strcat(message,dir->d_name);
                    strcat(message," ");
                    find_dir_con++;
                    find_con++;
                } else if(find_dir_con==2) {
                    strcat(message,dir->d_name);
                    strcat(message," ");
                } else {
                    char pathname[1028]= {'\0'};
                    strcat(pathname,ptr->data);
                    if(strlen(pathname)!=2) {
                        strcat(pathname,"/");
                    }
                    strcat(pathname,dir->d_name);
                    if(stat(pathname,&sb)==0&&S_ISDIR(sb.st_mode)) {
                        //dir
                        str_clear(item,50);
                        for(i=0; i<50; i++) {
                            item[i]=ptr->data[i];
                        }
                        item[strlen(ptr->data)]='\0';
                        if(strlen(item)==2) { //item=./
                            strcat(item,dir->d_name); //item=./testdir
                        } else {
                            strcat(item,"/"); //item=./testdir/
                            strcat(item,dir->d_name); //item=./testdir/secfolder
                        }
                        //printf("item:%s\n",item);
                        if(strcmp(dir->d_name,ptr->target)==0) {
                            Push(item,strlen(item),ptr->target,strlen(ptr->target),ptr->forClientSockfd);
                            find_dir_con++;
                        }
                    }
                    //if(stat(dir->d_name,&sb)==0&&S_ISREG(sb.st_mode))
                    else {
                        //file
                        printf("dir->d_name=%s\n",dir->d_name);
                        printf("ptr->target=%s\n",ptr->target);
                        if(strcmp(dir->d_name,ptr->target)==0) { //It's found out the file
                            find_con++;
                            char *res=NULL;
                            res=strchr(ptr->target,'.');
                            printf("res=%s\n",res);
                            if(strcmp(res,".htm")==0) {
                                strcat(message,"HTTP/1.x 200 OK\r\n");
                                strcat(message,"Content-type: text/html\r\n");
                            } else if(strcmp(res,".html")==0) {
                                //printf("456\n");
                                strcat(message,"HTTP/1.x 200 OK\r\n");
                                strcat(message,"Content-type: text/html\r\n");
                            } else if(strcmp(res,".css")==0) {
                                strcat(message,"HTTP/1.x 200 OK\r\n");
                                strcat(message,"Content-type: text/css\r\n");
                            } else if(strcmp(res,".h")==0) {
                                strcat(message,"HTTP/1.x 200 OK\r\n");
                                strcat(message,"Content-type: text/x-h\r\n");
                            } else if(strcmp(res,".hh")==0) {
                                strcat(message,"HTTP/1.x 200 OK\r\n");
                                strcat(message,"Content-type: text/x-h\r\n");
                            } else if(strcmp(res,".c")==0) {
                                strcat(message,"HTTP/1.x 200 OK\r\n");
                                strcat(message,"Content-type: text/x-c\r\n");
                            } else if(strcmp(res,".cc")==0) {
                                strcat(message,"HTTP/1.x 200 OK\r\n");
                                strcat(message,"Content-type: text/x-c\r\n");
                            } else if(strcmp(res,".json")==0) {
                                strcat(message,"HTTP/1.x 200 OK\r\n");
                                strcat(message,"Content-type: application/json\r\n");
                            } else {
                                strcat(message,"HTTP/1.x 415 UNSUPPROT_MEDIA_TYPE\r\n");
                                strcat(message,"Content-type:\r\n");
                                sup_type=0;
                            }
                            strcat(message,"Server: httpserver/1.x \r\n\r\n");
                            if(sup_type==1) {
                                //print file content
                                for(i=0; i<50; i++) {
                                    item[i]=ptr->data[i];
                                }
                                i=0;
                                if(strlen(item)==2) {
                                    strcat(item,dir->d_name); //item=./testdir
                                } else {
                                    strcat(item,"/"); //item=./testdir/
                                    strcat(item,dir->d_name); //item=./testdir/example.html
                                }
                                sscanf(item,"./%s",sub_item); //sub_item=testdir/example.html
                                file=fopen(sub_item,"r");
                                while((ch=fgetc(file))!=EOF) {
                                    file_content[i]=ch;
                                    i++;
                                }
                                fclose(file);
                                if(file_content) {
                                    strcat(message,file_content);
                                }
                            }
                        }
                    }
                }
            }
        }
        closedir(d);
    }
}

void *runner(void *param)
{
    char message[10000];
    int thread_num=*(int *)param;
    while(1) {
        pthread_mutex_lock(&lock);
        //critical section
        find_dir_con=0;
        if(isEmpty()==1) { //queue is empty
            pthread_mutex_unlock(&lock);
            continue;
        } else {
            for(i=0; i<10000; ++i) {
                message[i]='\0';
            }
            printf("Thread %d\n",thread_num);
            struct node* ptr=Pop();
            Pop_s=ptr->data;
            //printf("Pop_s=%s\n",Pop_s);
            for(i=0; i<50; i++) {
                Pop_ss[i]=Pop_s[i]; // ./testdir/secfolder
            }
            char* r=NULL;
            r=strrchr(Pop_ss,'/'); //r=/secfolder
            sscanf(r,"/%s",r); //r=secfolder
            if(strcmp(r,ptr->target)==0) {
                find_dir_con=1;
            }
            printf("r:%s\n",r);
            printf("ptr->data:%s\n",ptr->data);
            printf("ptr->target:%s\n",ptr->target);
            printf("Q_NUM=%d\n\n",Q_NUM);
            d=opendir(ptr->data); //ptr->data=./testdir
            find(d,message,ptr);
            printf("find_con=%d\n",find_con);
            if(find_con!=0) {
                strcat(message,"\n");
                send(ptr->forClientSockfd,message,sizeof(message),0);
            } else if(find_con==0&&find_dir_con==0) {
                strcat(message,"HTTP/1.x 404 NOT_FOUND\r\nContent-type:\r\nServer: httpserver/1.x\r\n\r\n");
                send(ptr->forClientSockfd,message,sizeof(message),0);
                printf("%s",message);
            }
            find_con=0;
            sup_type=1;
            find_dir_con=0;
        }
        pthread_mutex_unlock(&lock);
    }
}

int main(int argc, char *argv[])

{
    char a[4]= {'\0'};
    char b[1000]= {'\0'};
    char c[50]= {'\0'};
    char d[50]= {'\0'};
    char end[50]= {'\0'};
    char av_2[1025]= {'\0'};
    char *e=NULL;
    char dot[1026]=".";
    char bad_request[200]="HTTP/1.x 400 BAD_REQUEST\r\nContent-type:\r\nServer: httpserver/1.x\r\n\r\n";
    char err_method[200]="HTTP/1.x 405 METHOD_NOT_ALLOWED\r\nContent-type:\r\nServer: httpserver/1.x\r\n\r\n";
    char err_type[200]="HTTP/1.x 415 UNSUPPORT_MEDIA_TYPE\r\nContent-type:\r\nServer: httpserver/1.x\r\n\r\n";

    thread_cnt=atoi(argv[6]);

    //set up socket
    char inputBuffer[256] = {};
    int sockfd = 0,forClientSockfd = 0;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd == -1) {
        printf("Fail to create a socket.");
    }

    //socket connection
    struct sockaddr_in serverInfo,clientInfo;
    int addrlen = sizeof(clientInfo);
    bzero(&serverInfo,sizeof(serverInfo));

    serverInfo.sin_family = AF_INET;
    serverInfo.sin_addr.s_addr = inet_addr("127.0.0.1");
    int port=atoi(argv[4]);
    serverInfo.sin_port = htons(port);
    bind(sockfd,(struct sockaddr *)&serverInfo,sizeof(serverInfo));
    listen(sockfd,5);

    Q_NUM=0;
    Q_HEAD=NULL;
    Q_TAIL=NULL;

    thread_pool=malloc(sizeof(struct thread_member)*thread_cnt);
    for(i=0; i<thread_cnt; i++) {
        thread_pool[i].alive=0;
    }
    for(i=0; i<thread_cnt; i++) {
        pthread_create(&thread_pool[i].thread,NULL,runner,&i); //create the thread
    }

    int b_con=0;
    while(1) {
        str_clear(a,4);
        str_clear(b,1000);
        str_clear(c,50);
        str_clear(d,50);
        str_clear(end,50);
        str_clear(item,50);
        str_clear(sub_item,50);
        str_clear(file_content,10000);
        str_clear(Pop_ss,50);
        str_clear(av_2,1025);
        find_dir_con=0;
        sup_type=1;
        target=NULL;
        Pop_s=NULL;
        dot[0]='.';
        dot[1]='\0';
        e=NULL;
        b_con=0;
        //printf("server prepare to receive\n");
        forClientSockfd = accept(sockfd,(struct sockaddr*) &clientInfo, &addrlen);
        recv(forClientSockfd,inputBuffer,sizeof(inputBuffer),0);
        printf("server has received\n");
        char save_inputBuffer[256]= {'\0'};
        strcat(save_inputBuffer,inputBuffer);

        //Handle inputBuffer string
        sscanf(inputBuffer,"%s %s %s %s %s",a,b,c,d,end);
        //printf("a=%s\n",a);
        if(strcmp(b,"/")==0) {
            printf("inputBuffer:\n%s",inputBuffer);
            strcat(av_2,argv[2]);
            strcat(dot,av_2); //dot=./testdir
            e=strrchr(av_2,'/'); //e=/testdir
            sscanf(e,"/%s",e); //e=testdir
            printf("e=%s\n",e);
            printf("dot=%s\n",dot);
            pthread_mutex_lock(&lock);
            Push(dot,strlen(dot),e,strlen(e),forClientSockfd);
            pthread_mutex_unlock(&lock);
        } else {
            printf("inputBuffer:\n%s",inputBuffer);
            char *substr=NULL;
            substr=strchr(inputBuffer,'/'); //remove Get
            char* delim=" ";
            target=strtok(substr,delim);
            if(inputBuffer[4]!='/') {
                printf("Request does not start with a slash\n");
                send(forClientSockfd,bad_request,sizeof(bad_request),0);
                close(forClientSockfd);
                continue;
            }
            if(strlen(substr)>128) {
                printf("Size of QUERY_FILE_OR_DIR is greater than 128 bytes\n");
                send(forClientSockfd,bad_request,sizeof(bad_request),0);
                close(forClientSockfd);
                continue;
            }
            if(strcmp(a,"GET")!=0) {
                send(forClientSockfd,err_method,sizeof(err_method),0);
                close(forClientSockfd);
                continue;
            }
            for(i=0; i<1000; i++) {
                if(b[i]=='.') {
                    b_con++;
                    break;
                }
            }
            if(b_con!=0) {
                char *sss=strrchr(b,'.');
                if(strcmp(sss,".htm")!=0&&strcmp(sss,".html")!=0&&strcmp(sss,".css")!=0&&strcmp(sss,".h")!=0&&strcmp(sss,".hh")!=0&&strcmp(sss,".c")!=0&&strcmp(sss,".cc")!=0&&strcmp(sss,".json")!=0) {
                    send(forClientSockfd,err_type,sizeof(err_type),0);
                    close(forClientSockfd);
                    continue;
                }
            }

            //sscanf(target,"/%s",target);
            if(target[strlen(target)-1]=='/') {
                target[strlen(target)-1]='\0'; //remove the end slash
            }
            printf("substr=%s\n",substr);
            printf("target=%s\n",target); //target=example.html
            e=strrchr(target,'/');
            sscanf(e,"/%s",e);
            printf("e=%s\n",e);
            strcat(dot,argv[2]); //dot=./testdir

            pthread_mutex_lock(&lock);
            Push(dot,strlen(dot),e,strlen(e),forClientSockfd);
            pthread_mutex_unlock(&lock);
        }

        //printf("%s\n",save_inputBuffer);
    }
    return 0;
}
