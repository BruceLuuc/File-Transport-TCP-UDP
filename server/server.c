#include "server_work.h"

int listenfd;

int main(){
    /** 创建广播线程 **/
    pthread_t broadthrd;
    int err=-1;
    if (( (err = pthread_create(&broadthrd,NULL,broad,NULL))) !=0 ){
        perror("can't create broad thread");
        exit(-1);
    }

    listenfd = Server_init(TCP);
    
    /** 接收客户端命令行参数 **/
    struct command cmd;
    while(1){
        printf("\nWait for task...\n");
        bzero(&cmd,sizeof(struct command));
        recv_cmd(&cmd);
        printf("cmd:%s -%s -%s\n",cmd.filename,cmd.cmd,cmd.mode); 
        recv_fileinfo(&cmd); 

        switch (get_cmd(cmd.cmd)){

            case GET:
                /** 客户端下载请求 **/
                send_file(&cmd);
                break;
            case PUT:
                /** 客户端上传请求 **/
                recv_block(&cmd);
                break;
            case UNKNOWN:
            default:
                printf("cmd error\n");
                break;
        }
    }

    return 0;
}


