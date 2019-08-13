#include "common.h"

/** UDP包全局变量 **/
int                 send_id = 0;    /** UDP发送id **/
int                 receive_id = 0; /** UDP接收id **/
int                 id = 1;         /** UDP确认 **/

int Socket(int family, int type, int protocol){
    int             n;

    if ((n = socket(family, type, protocol)) < 0){
        perror("socket error");
        exit(0);
    }

    return n;
}


void Bind(int fd, const struct sockaddr * sa, socklen_t salen){
    if (bind(fd, sa, salen) < 0){
        perror("bind error");
        exit(1);
    }
}


void Listen(int fd, int backlog){
    char *          ptr;

    if ((ptr = getenv("LISTENQ")) != NULL)
        backlog = atoi(ptr);

    if (listen(fd, backlog) < 0){
        perror("listen error");
        exit(2);
    }
}


int Accept(int fd, struct sockaddr * sa, socklen_t * salenptr){
    int             n;

    if ((n = accept(fd, sa, salenptr)) < 0){
        perror("Accept Error!");
        exit(3);
    }

    return (n);
}


void Connect(int fd, const struct sockaddr * sa, socklen_t salen){
    if (connect(fd, sa, salen) < 0){
        perror("Connect Error");
        exit(4);
    }
}

int Client_init(struct ip_port *ip_p, int mode){
    int         sockfd = -1;
    if (mode == TCP) {
        char        ip[12] = {'\0'};
        int         port =   ip_p->port;
        strcpy(ip,ip_p->ip);
        sockfd = Socket(AF_INET, SOCK_STREAM, 0);    
        struct sockaddr_in servaddr;
        bzero(&servaddr, sizeof(servaddr));
        servaddr.sin_family = AF_INET;
        servaddr.sin_port   = htons(port);
        inet_pton(AF_INET, ip, &servaddr.sin_addr);
        Connect(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr));
    }
    else if (mode == UDP) 
        sockfd = Socket(AF_INET, SOCK_DGRAM, 0);
    return sockfd;
}


int Server_init(int mode){
    int         listen_fd = -1;
    int         port = PORT;
    
    struct sockaddr_in server_addr;
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    
    if (mode == TCP) {
        listen_fd = Socket(AF_INET, SOCK_STREAM, 0);
    }
    else if (mode == UDP) {
        listen_fd = Socket(AF_INET, SOCK_DGRAM, 0);
        port = UDP_PORT;
    }   
    
    inet_pton(AF_INET, IP, &server_addr.sin_addr);
    server_addr.sin_port = htons(port);
    

    Bind(listen_fd, (struct sockaddr *) &server_addr, sizeof(server_addr));
    if (mode == TCP) 
        Listen(listen_fd, LISTENQ);
    return listen_fd;
}


int Open(char * filename){
    int             fd  = 0;

    if ((fd = open(filename, O_RDWR)) == -1){
        printf("open error\n");
        return -1;
    }

    return fd;
}

int Writen(int fd, const void * vptr, const int n){
    int          nleft;
    int          nwritten;
    char *          ptr;

    ptr                 = (char *)vptr;
    nleft               = n;

    while (nleft > 0){
        if ((nwritten = write(fd, ptr, nleft)) < 0){
            if (nwritten == n)
                return - 1;

            else 
                break;
        }
        else if (nwritten == 0){
            break;
        }

        nleft               -= nwritten;
        ptr                 += nwritten;
    }

    return (n - nleft);
}




int Readn(int fd, const void * vptr, const int n){
    int          nleft;
    int         nread;

    nleft               = n;
    char *          ptr = (char*)vptr;

    while (nleft > 0){
        if ((nread = read(fd, ptr, nleft)) < 0){
            if (nleft == n)
                return (-1);

            else 
                break;
        }
        else if (nread == 0){
            break;
        }

        nleft               -= nread;
        ptr                 += nread;
    }

    return (n - nleft);
}


int createfile(char * filename, int size){
    int             fd  = open(filename, O_RDWR | O_CREAT);

    fchmod(fd, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    lseek(fd, size - 1, SEEK_SET);
    write(fd, "", 1);
    close(fd);
    return 0;
}


size_t get_filesize(char *path){
    int fd = Open(path);
    struct stat st;
    fstat(fd, &st);
    close(fd);
    return st.st_size;
}



char * Md5(char * filename, char *md5){
	int 			fd = Open(filename);

	unsigned char 	*md = (unsigned char *) malloc(16*sizeof(char));
	if (md == NULL || md5==NULL){
        perror("malloc md5");
        exit (-1);
    }

	char 			tmp[3] = {'\0'};

	size_t          file_size = get_filesize(filename);
	unsigned char   *mbegin  = (unsigned char *)mmap(NULL, file_size, PROT_WRITE | PROT_READ, MAP_SHARED, fd, 0);
    
	md = MD5(mbegin, file_size, md);

	for(int i = 0; i < 16;i++){
		sprintf(tmp, "%02X", md[i]);//X 16进制大写
		strcat(md5, tmp);//追加到md5
	}

	munmap(mbegin,file_size);
	free(md);
	md=NULL;
	close(fd);
	return md5;
}


int get_cmd(char *str){
    if (str == NULL || *str == '\0')
        return EMPTY;
    
    if (strcmp(str, "q") == 0 || strcmp(str, "quit") == 0 || strcmp(str, "exit") == 0)
        return EXIT;
        
    if (strcmp(str, "get")  == 0)
        return GET;
        
    if (strcmp(str, "put")  == 0)
        return PUT;
        
    if (strcmp(str, "t") == 0 || strcmp(str, "tcp") == 0)
        return TCP;

    if (strcmp(str, "u") == 0 || strcmp(str, "udp") == 0)
        return UDP;

    return UNKNOWN;

}


int send_by_udp(int fd, char *seek, int left, struct sockaddr *addr){
    struct PackInfo        pack_info;
    struct RecvPack        data;
    bzero(&pack_info, sizeof(pack_info));
    bzero(&data, sizeof(data));
    socklen_t                 serv_len= sizeof(struct sockaddr_in);
    int                     buf_size = MIN(left, BUFFER_SIZE);
    int n=-1;
    int res = 0; //成功发送出去的字节数
    memcpy(data.buf, seek, buf_size);//先拷贝进来

    if (receive_id == send_id) {
        ++send_id;
        data.head.id        = send_id; /* 发送id放进包头,用于标记顺序 */
        data.head.buf_size  = buf_size; /* 记录数据长度 */
        //printf("#1 Pack_id: %d\n",send_id);
        n=sendto(fd, (char *) &data, sizeof(data), 0, (struct sockaddr *) addr, serv_len);
        if (n > 0){
            /* 接收确认消息 */
            //printf("#2 Recv Ack..\n");
            if (Readable_timeo(fd, 10)==0){
                printf("!! Timeout !!\n");
                return -1;
            }  
            recvfrom(fd, (char *) &pack_info, sizeof(pack_info), 0, (struct sockaddr *) addr, &serv_len);
            receive_id          = pack_info.id;
            res = buf_size;
        }
        else 
            return 0;
    }
    else {
        /* 如果接收的id和发送的id不相同,重新发送 */
        printf("!!!!#1 Pack_id: %d\n",send_id);
        if (sendto(fd, (char *) &data, sizeof(data), 0, (struct sockaddr *) addr, serv_len) < 0)
            perror("Send File Failed:");
    
        /* 接收确认消息 */
        printf("!!!!#2 Recv Ack..\n");
        if (Readable_timeo(fd, 10)==0){
            printf("!! Timeout !!\n");
            return -1;
        } 
        recvfrom(fd, (char *) &pack_info, sizeof(pack_info), 0, (struct sockaddr *) addr, &serv_len);
        receive_id          = pack_info.id;
    }

    return res;

}


int recv_by_udp(int fd, char *seek, struct sockaddr *addr){
    struct PackInfo        pack_info;
    struct RecvPack        data;
    bzero(&pack_info, sizeof(pack_info));
    bzero(&data, sizeof(data));
    socklen_t                 clilen= sizeof(struct sockaddr_in);
    int res=0;//本次成功写入的字节数
    //printf("#1 Pack_id: %d\n", id);
    if (Readable_timeo(fd, 10)==0){
        printf("!! Timeout !!\n");
        return -1;
    } 
    int n = recvfrom(fd, (char *) &data, sizeof(data), 0, (struct sockaddr *) addr, &clilen);

    if (n > 0) {
        if (data.head.id == id){
            pack_info.id = data.head.id;
            pack_info.buf_size  = data.head.buf_size;
            ++id;
            /** 发送数据包确认信息 **/
            //printf("#2 Send Ack..\n");
            if (sendto(fd, (char *) &pack_info, sizeof(pack_info), 0, (struct sockaddr *) addr, clilen) < 0)
                printf("Send confirm information failed!");
    
            /* 写入文件 */
            memcpy(seek, data.buf, data.head.buf_size); 
            res = data.head.buf_size;
        }
        else if(data.head.id < id){
            /** 如果是重发的包 **/
            pack_info.id        = data.head.id;
            pack_info.buf_size  = data.head.buf_size;
            /* 重发数据包确认信息 */
            printf("!!!!#3 Resend confirm information..\n");
            if (sendto(fd, (char *) &pack_info, sizeof(pack_info), 0, (struct sockaddr *) addr, clilen) < 0)
                printf("Send confirm information failed!");
        }

    }
    return res;
}


void reset_udp_id(){
    send_id = 0;
    receive_id = 0;
    id = 1;
}

/** > 0 if descriptor is readable **/
int readable_timeo(int fd, int sec){
	fd_set			rset;
	struct timeval	tv;

	FD_ZERO(&rset);
	FD_SET(fd, &rset);

	tv.tv_sec = sec;
	tv.tv_usec = 0;

	return(select(fd+1, &rset, NULL, NULL, &tv));
}

int Readable_timeo(int fd, int sec){
	int		n;

	if ( (n = readable_timeo(fd, sec)) < 0)
		perror("readable_timeo error");
	return(n);
}


void progress_bar(int rate){
    /** 输入有效性校正 **/
    if (rate<0 || rate>100)printf("!! Out of range [0-100] !!\n");
    if (rate < 0)rate=0;
    if (rate > 100)rate=100;

    int old=rate;
    //获取控制台长宽，使进度条长度自适应控制台宽度
    //BUG:最大化会多行#
    struct winsize console;
    int len = 50;
    if (ioctl(STDIN_FILENO, TIOCGWINSZ, (char *)&console)!=-1){
        len = console.ws_col-16;
    }
    rate=len*rate/100;
    char bar[len];
    bzero(bar, len);
    const char *state = "|/-\\";

    printf("[");
    for(int i=0; i<len; i++){
        bar[i]=i<rate? '#':'.';
        printf("%c",bar[i]);
    }
    printf("]");
    printf(" [%d%%] [%c]\r",old, state[old%4]);
   
    fflush(stdout);
    printf("\33[2K\r");
}
