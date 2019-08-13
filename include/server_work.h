#ifndef __SERVER_WORK_H__
#define __SERVER_WORK_H__
#include "common.h"


/** 广播服务器IP和端口号，每隔3秒广播一次 **/
void *broad();


/** 接收客户端命令参数 **/
void recv_cmd(struct command *cmd);


/** 接收文件信息，建立并保存在本地,并把断点信息回传给客户端 ***/
void recv_fileinfo(struct command *cmd);


/** 接收文件，校验成功放入下载队列，等待客户端下载 **/
void recv_block(struct command *cmd);


/** 客户端下载请求，发送文件 **/
void send_file(struct command *cmd);

#endif

