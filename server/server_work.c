#include "server_work.h"

int                 p_id = 0;           //put 上来的数组p_files存储下标
int                 g_id = 0;           //get 请求时的g_files数组下标

struct fileinfo     p_files[FILE_MAX];  //put上来的文件
struct fileinfo     g_files[FILE_MAX];  //客户端下载文件
struct fileinfo     file;               //本次交互文件信息临时存放
extern int listenfd;

struct sockaddr_in cliaddr;
socklen_t          clilen= sizeof(struct sockaddr_in);

void *broad(){
    struct ip_port ip;
    strcpy(ip.ip, IP);
    ip.port=PORT;

    int ip_len = sizeof(struct ip_port);
    char msg[ip_len+1];
    bzero(msg,ip_len+1);
    memcpy(msg, &ip, ip_len);

    int brdcFd = Socket(PF_INET, SOCK_DGRAM, 0);

    int optval = 1;//这个值一定要设置，否则可能导致sendto()失败
    setsockopt(brdcFd, SOL_SOCKET, SO_BROADCAST | SO_REUSEADDR, &optval, sizeof(int));
    struct sockaddr_in theirAddr;
    memset(&theirAddr, 0, sizeof(struct sockaddr_in));
    theirAddr.sin_family = AF_INET;
    theirAddr.sin_addr.s_addr = inet_addr("255.255.255.255");
    theirAddr.sin_port = htons(4001);
    //int timer = 0;//定时器
    for (;;){
        int sendBytes;
        if((sendBytes = sendto(brdcFd, msg, sizeof(msg), 0, (struct sockaddr *)&theirAddr, sizeof(struct sockaddr))) == -1){
            printf("sendto fail, errno=%d\n", errno);
            break;
        }
        //timer += 3;
        sleep(3);
        //if (timer > 180)//广播三分钟结束
            //break;
    }
    close(brdcFd);
    return (void*)0;
}



void recv_cmd(struct command *cmd){
    int connfd = Accept(listenfd, (struct sockaddr *) &cliaddr, (socklen_t *)&clilen);   
    int cmdlen = sizeof(struct command);
    char buf[cmdlen+1];
    bzero(buf, cmdlen+1);
    Readn(connfd, buf, cmdlen);
    memcpy(cmd, buf, cmdlen);
    close(connfd);
    return;
}


void recv_fileinfo(struct command *cmd){
    printf("############ recv File information ############\n");
    bzero(&file, sizeof(struct fileinfo));    
    int connfd = Accept(listenfd, (struct sockaddr *) &cliaddr, (socklen_t *)&clilen); 

    /** 接收文件信息 **/
    char            fileinfo_buf[256] ={'\0'};
    int             fileinfo_len = sizeof(struct fileinfo);
    Readn(connfd, fileinfo_buf, fileinfo_len);

    memcpy(&file, fileinfo_buf, fileinfo_len);
    printf("file from client:\n");
    printf("\t%s\n\tsize=%dM\n\tmd5=%s\n\tlseek=%d\n", file.filename, file.filesize/M, file.md5, file.pos);
    /** 放进上传数组里面 **/
    if (get_cmd(cmd->cmd)==PUT){
        /** 遍历，找到相等的 **/
        for (int i=0; i<FILE_MAX; i++ ){
            if ( strcmp(file.filename, p_files[i].filename)==0 ){
                if (Open(file.filename)==-1)//上传的文件被外部删除
                    break;
                p_id = i;
                file.pos=p_files[i].pos;
                file.used=1;
                break;
            }        
        }
        /** 没找到找，判断为新上传的文件 **/
        if (file.used==0){
            createfile(file.filename, file.filesize);//创建空洞文件
            file.used=1;
            while (p_files[p_id].used){
                   ++p_id;
                   p_id = p_id % FILE_MAX;
            }
            memcpy(&p_files[p_id], &file, fileinfo_len);
        }
        printf("Saving: p_id:%d %s %d %s lseek=%d\n",p_id, p_files[p_id].filename, p_files[p_id].filesize, p_files[p_id].md5, p_files[p_id].pos);
    }
    else{//下载请求，在下载数组里搜
        for (int i=0; i<FILE_MAX; i++ ){
            if (strcmp(file.filename, g_files[i].filename)==0){
                g_id = i;
                memcpy(&file, &g_files[i], fileinfo_len);
                file.used=1;
                break;
            }
        }
        
        /** 没找到找，文件不存在或不完整，拒绝下载                     **/ 
        if (file.used==0){
            printf("404 Not Found\n");
        }

    }

    char            send_buf[256] ={'\0'};
    memcpy(send_buf , &file, fileinfo_len);
    printf("file to client:\n");
    printf("\t%s\n\tsize=%dM\n\tmd5=%s\n\tlseek=%d\n", file.filename, file.filesize/M, file.md5, file.pos);
    Writen(connfd, send_buf,  fileinfo_len);//把更新后的file信息回传，主要是为了传回偏移位置，实现断点续传
    printf("########### send back File information ###########\n");
    close(connfd);
    return;
}


void recv_block(struct command *cmd){
    printf("\n############ 接收文件 ############\n");
    int b_tcp = get_cmd(cmd->mode)==UDP? UDP:TCP;
    
    int fd = Open(file.filename);
    char *begin=(char *)mmap(NULL, file.filesize, PROT_WRITE|PROT_READ, MAP_SHARED, fd , 0);
    close(fd);
    char *seek = begin + file.pos;


    int n=0;
    int left = file.filesize -  file.pos;
    if (left < 1){
        printf("秒传！\n");
        goto over;
    }
    if (b_tcp==TCP){
        printf("======== TCP ========\n");
        int connfd = Accept(listenfd, (struct sockaddr *) &cliaddr, (socklen_t *)&clilen); 
        while(left>0){
            n=read(connfd, seek, left);
            if (n<0) {
                printf("连接异常！\n");
                break;
            }
            file.pos += n;
            p_files[p_id].pos += n;
            left -= n;
            seek +=n ;
            printf("recv:%d  left:%dM  lseek:%d\n",n, left/M, p_files[p_id].pos);
            progress_bar((100*((file.filesize-left)/M))/(file.filesize/M));
        }
        printf("======= TCP OK =======\n");
        close(connfd);
    }else{
        printf("======== UDP ========\n");
        bzero(&cliaddr, clilen);
        int connfd = Server_init(UDP);
        //sleep(1);
        while(left > 0){             
            n=recv_by_udp(connfd, seek, (struct sockaddr *)&cliaddr);
            if (n==-1)break;
            file.pos += n;
            p_files[p_id].pos += n;
            left -= n;
            seek += n;
            //printf("#3 recv:%d  left:%dM  lseek:%d\n",n, left/M, file.pos);
            progress_bar((100*((file.filesize-left)/M))/(file.filesize/M));
        }
        printf("======= UDP OK =======\n");
        reset_udp_id();
        close(connfd);
    }
over:
    munmap((void *)begin, file.filesize);//在Md5前释放
    
    /** 效验完整性 **/
    printf("传输完毕，正在校验文件完整性 ..\n");
    char    md5[33] = {'\0'};
    Md5(file.filename, md5);

    if (strcmp(file.md5,md5)==0){
        printf("文件完好，校验完毕！\n");
        printf("已完成上传 100%%\n");
        p_files[p_id].intact=1;
        p_files[p_id].pos=file.filesize;

        //防止重复加入下载队列
        int b_already=0;
        for (int i=0; i<FILE_MAX; i++ ){
            if ( strcmp(file.filename, g_files[i].filename)==0 ){
                g_id=i;
                b_already=1;
                break;
            }        
        }
        //防止重复加入下载队列
        if (b_already ==0 ){
            while(g_files[g_id].used==1){
                 ++g_id;
                g_id = g_id % FILE_MAX;
            }
            memcpy(&g_files[g_id], &p_files[p_id], sizeof(struct fileinfo));//把上传结束的文件加入下载队列
            g_files[g_id].pos=0;//初始化为0，第一次下载从偏移0开始
        }

        printf("上传结束,加入下载队列 g_files[%d]\n%s %d MD5:%s lseek=%d\n",g_id, g_files[g_id].filename, g_files[g_id].filesize, g_files[g_id].md5, g_files[g_id].pos);
    }else{
        printf("文件缺损！！上传未完成...\n");
        printf("上传进度 %d%%\n", (int)((100*(p_files[p_id].pos/M))/(p_files[p_id].filesize/M)) );
    }
    
    printf("############ 接收完毕 ############\n");
    return;
}


void send_file(struct command *cmd){
    if (file.used==0){
        printf("can't find %s !\n",file.filename);
        return;//服务器不存在此文件
    }

    int fd = Open(cmd->filename);
    if (fd==-1){
        printf("can't find %s !\n",file.filename);
        return;
    }
    printf("\n############ 发送文件 ############\n");      
    printf("%s size:%d MD5:%s 下载位置:%d\n",file.filename, file.filesize, file.md5, file.pos);
    char *begin=(char *)mmap(NULL, file.filesize, PROT_WRITE|PROT_READ, MAP_SHARED, fd , 0);
    close(fd);
    int n=0;
    int left = file.filesize -  file.pos;
    if (left < 1){
        printf("秒传！\n");
        goto over;
    }
    char *seek = begin + file.pos;
    int b_tcp = get_cmd(cmd->mode)==UDP? UDP:TCP;
    if (b_tcp==TCP){
        printf("======== TCP ========\n");
        int connfd = Accept(listenfd, (struct sockaddr *) &cliaddr, &clilen);
        while(left>0){
            int bytes = MIN(left, M);
            n=write(connfd, seek, bytes);
            left -= n;
            g_files[g_id].pos += n;
            file.pos += n;
            seek += n;
            printf("send:%d  left:%dM  lseek:%d\n",n, left/M, g_files[g_id].pos);
            progress_bar((100*((file.filesize-left)/M))/(file.filesize/M));
        }
        printf("====== TCP 发送结束 ======\n");
        close(connfd);
    }
    else{
        printf("======== UDP ========\n");
        bzero(&cliaddr, clilen);
        reset_udp_id();
        int connfd = Server_init(UDP);
        
        /** 接收客户端发来的download字符，顺便存储客户端地址信息 **/
        char buf[10]={'\0'};
        socklen_t               clilen= sizeof(struct sockaddr_in);
        recvfrom(connfd, buf, 10, 0, (struct sockaddr*)&cliaddr,&clilen);
        printf("%s...\n",buf);
        
        while(left > 0){
            n=send_by_udp(connfd, seek, left, (struct sockaddr*)&cliaddr);
            if (n == -1)break;
            file.pos += n;
            g_files[g_id].pos += n;
            left -= n;
            seek += n;
            //printf("#3 Send:%d  left:%dM  lseek:%d\n",n, left/M, g_files[g_id].pos);
            progress_bar((100*((file.filesize-left)/M))/(file.filesize/M));
        }
        printf("====== UDP 发送结束 ======\n");
        close(connfd);
    }
over:    
    printf("下载进度 %d%%\n", (int)((100*(g_files[g_id].pos/M))/(g_files[g_id].filesize/M)) );
    //if (left <= 0)
        //g_files[g_id].pos=0;
    munmap((void *)begin, file.filesize);
    printf("############ 发送完毕 ############\n");
    
    return;
}


