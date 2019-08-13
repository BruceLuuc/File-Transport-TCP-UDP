#ifndef __CLIENT_WORK_H__
#define __CLIENT_WORK_H__
#include "common.h"

/*************************************************************************
* @函数名:           menu
* @函数描述:          显示菜单，命令帮助提示
* @输入:            ip_port结构体地址
* @输出:            无
* @返回:            无
 ************************************************************************/
void menu(struct ip_port *ip);


/*************************************************************************
* @函数名:           get_server_list
* @函数描述:          获取局域网可用服务器列表
* @输入:            ip_port结构体地址
* @输出:            打印可用服务器列表，包括服务器序号、IP、端口
* @返回:            无
 ************************************************************************/
void get_server_list(struct ip_port *ip);


/*************************************************************************
* @函数名:           split
* @函数描述:          解析输入的命令行参数，获取文件名、上传/下载、TCP/UDP模式、退出等信息
* @输入:            命令行结构体，终端标准输入的一行
* @输出:            把解析出来的参数存入结构体变量cmd中
* @返回:            成功返回0，失败返回-1
 ************************************************************************/
int split(struct command *cmd, char *line);

/** 获取输入命令，可以通过tab自动补全，上下键历史命令 **/
char *input(char *str);

/*************************************************************************
* @函数名:           get_input
* @函数描述:          从终端获取输入参数，并调用函数split解析
* @输入:            ip_port结构体地址，cmd结构体
* @输出:            无
* @返回:            无
 ************************************************************************/
void get_input(struct command *cmd, struct ip_port *ip);


/*************************************************************************
* @函数名:          exec_cmd
* @函数描述:        根据ip和获取到的命令行执行相关操作
 ************************************************************************/
void exec_cmd(struct command *cmd, struct ip_port *ip);


/** 发送命令给服务器 **/
void send_cmd(struct command *cmd, struct ip_port *ip);


/** 发送文件信息给服务器，返回断点续传信息 **/
void send_fileinfo(struct command *cmd, struct ip_port *ip);


/** 下载文件 **/
void recv_file(struct command *cmd, struct ip_port *ip);


/** 上传文件 **/
void send_block(struct command *cmd, struct ip_port *ip);


/** 初始化UDP服务端 **/
void udp_serv_init(struct sockaddr_in *server_addr, struct ip_port *ip);

#endif

