#ifndef __COMMON_H__
#define __COMMOM_H__
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <semaphore.h>
#include <time.h>
#include <sys/time.h>
#include <sys/epoll.h>
#include <sys/mman.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "md5.h"

#define M                   (1024*1024)
#define BUFFER_SIZE         1024            //UDP分包大小

#define IP                  "127.0.0.3"
#define PORT                9527            //TCP连接端口
#define UDP_PORT            8000            //UDP端口
#define LISTENQ             666

/** 服务器能保存的文件的最大数量 **/
#define FILE_MAX            64
#define FILENAME_MAXLEN     64

/** 命令行参数类型 **/
#define EMPTY               0
#define EXIT                1
#define LS                  2                   //查询服务器文件功能待加入
#define GET                 3
#define PUT                 4
#define TCP                 5
#define UDP                 6
#define UNKNOWN             7

/** 命令行颜色 **/
#define DEFAULT             "\033[0m"
#define RED                 "\e[0;31m"
#define BLUE                "\e[0;34m"
#define BLACK               "\e[0;30m"

#define MIN(a, b)           (((a) < (b)) ? (a) : (b))  


/** 文件信息 **/
struct fileinfo{
    char    filename[FILENAME_MAXLEN];      //文件名
    int     filesize;                       //文件大小

    char    md5[33];                        //文件md5值，效验文件传输完整性
    int     pos;                            //偏移量，记录断点续传信息
    int     intact;                         //文件完整性标志，1完整
    int     used;                           //使用标记，1代表使用
};

/** 广播服务器IP和端口 **/
struct ip_port{
    char ip[12];
    int  port;
};


/** 客户端命令行参数 **/
struct command{
    char filename[FILENAME_MAXLEN];
    char cmd[4];
    char mode[4];

};

/** UDP包头 **/
struct PackInfo{
    int   id;
    int   buf_size;
};


/** UDP接收包 **/
struct RecvPack{
    struct PackInfo head;
    char            buf[BUFFER_SIZE];
};

/** Socket包裹函数 **/
int Socket(int family,int type,int protocol);
void Bind(int fd, const struct sockaddr *sa, socklen_t salen);
void Listen(int fd, int backlog);
int Accept(int fd, struct sockaddr *sa, socklen_t *salenptr);
void Connect(int fd, const struct sockaddr *sa, socklen_t salen);
int Client_init(struct ip_port *ip_p, int mode);
int Server_init(int mode);


/** 文件操作相关 **/
int Open(char *filename);
int Writen(int fd, const void *vptr, const int n);
int Readn(int fd, const void *vptr, const int n);
int createfile(char *filename, int size);
size_t get_filesize(char *path);

/** 计算文件MD5值 **/
char *Md5(char * filename, char *md5);


/** 返回命令类型:PUT/GET     、TCP/UDP等 **/
int get_cmd(char *str);


/** UDP收发函数，成功返回收发的字节长度，超时返回 -1 **/
int send_by_udp(int fd, char *seek, int left, struct sockaddr *addr);
int recv_by_udp(int fd, char *seek, struct sockaddr *addr);

/** 全局变量复位 **/
void reset_udp_id();

/** 套接字超时返回函数,超过sec秒将返回0 **/
int readable_timeo(int fd, int sec);
int Readable_timeo(int fd, int sec);

/** 显示进度条  0<rate<100 **/
void progress_bar(int rate);

#endif

