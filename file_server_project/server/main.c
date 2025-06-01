#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <signal.h>
#include <sys/wait.h>
#include "../common/common.h"
#include "user.c"  


// 错误处理函数
void error(const char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

// 处理客户端请求的函数
void handle_client(int connfd) {


    char buffer[MAXLINE];

    while (1) {
        memset(buffer, 0, sizeof(buffer));
        int n = recv(connfd, buffer, MAXLINE, 0);
        if (n <= 0) break;

        buffer[n] = '\0';
        buffer[strcspn(buffer, "\n")] = 0;  // 清除换行符

        // 注册命令
        if (strncmp(buffer, "register", 8) == 0) {
            send(connfd, "请输入用户名：", strlen("请输入用户名："), 0);
            char username[64], password[64];

            memset(username, 0, sizeof(username));
            recv(connfd, username, sizeof(username), 0);
            username[strcspn(username, "\n")] = 0;

            send(connfd, "请输入密码：", strlen("请输入密码："), 0);
            memset(password, 0, sizeof(password));
            recv(connfd, password, sizeof(password), 0);
            password[strcspn(password, "\n")] = 0;

            int ok = register_user(username, password);
            if (ok)
                send(connfd, "注册成功\n", strlen("注册成功\n"), 0);
            else
                send(connfd, "用户名已存在\n", strlen("用户名已存在\n"), 0);
        }

        //登录命令
        else if (strncmp(buffer, "login", 5) == 0) {
    send(connfd, "请输入用户名：", strlen("请输入用户名："), 0);
    char username[64], password[64], role[16];

    memset(username, 0, sizeof(username));
    recv(connfd, username, sizeof(username), 0);
    username[strcspn(username, "\n")] = 0;

    send(connfd, "请输入密码：", strlen("请输入密码："), 0);
    memset(password, 0, sizeof(password));
    recv(connfd, password, sizeof(password), 0);
    password[strcspn(password, "\n")] = 0;

    if (login_user(username, password, role)) {
        char msg[128];
        snprintf(msg, sizeof(msg), "登录成功，欢迎您 [%s]（%s）\n", username, role);
        send(connfd, msg, strlen(msg), 0);
    } else {
        send(connfd, "登录失败：用户名或密码错误\n", strlen("登录失败：用户名或密码错误\n"), 0);
    }
        }

    //文件上传功能
    else if (strncmp(buffer, "upload", 6) == 0) {
    // 提取上传的文件名
    char filename[64];
    sscanf(buffer + 7, "%s", filename);  // 从 upload filename 中提取 filename

    char filepath[128];
    snprintf(filepath, sizeof(filepath), "data/%s", filename);

    FILE *fp = fopen(filepath, "wb");
    if (!fp) {
        send(connfd, "文件创建成功\n", strlen("文件创建成功\n"), 0);
        return;
    }

    send(connfd, "开始上传\n", strlen("开始上传\n"), 0);

    char filebuf[1024];
    int n;

    while ((n = recv(connfd, filebuf, sizeof(filebuf), 0)) > 0) {
        if (n == 3 && strncmp(filebuf, "EOF", 3) == 0) break;  // 客户端上传完毕的标志
        fwrite(filebuf, 1, n, fp);
    }

    fclose(fp);
    send(connfd, "文件上传成功\n", strlen("文件上传成功\n"), 0);
}

//文件下载功能
else if (strncmp(buffer, "download", 8) == 0) {
    char filename[64];
    sscanf(buffer + 9, "%s", filename);

    char filepath[128];
    snprintf(filepath, sizeof(filepath), "data/%s", filename);

    FILE *fp = fopen(filepath, "rb");
    if (!fp) {
        send(connfd, "文件不存在\n", strlen("文件不存在\n"), 0);
        return;
    }

    send(connfd, "开始下载\n", strlen("开始下载\n"), 0);

    char filebuf[1024];
    int n;
    while ((n = fread(filebuf, 1, sizeof(filebuf), fp)) > 0) {
        send(connfd, filebuf, n, 0);
    }

    fclose(fp);
    send(connfd, "EOF", 3, 0);  // 通知客户端传输结束
}




        // 其他命令统一处理（临时）
        else {
            send(connfd, "未知命令\n", strlen("未知命令\n"), 0);
        }
    }

    close(connfd);
    exit(0);
}



int main() {
    signal(SIGCHLD, SIG_IGN);  // 避免僵尸进程

    int listenfd, connfd;
    struct sockaddr_in serv_addr, cli_addr;
    socklen_t cli_len = sizeof(cli_addr);

    // 创建监听套接字
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd < 0) error("socket error");
    // 允许端口快速复用（解决 bind: Address already in use）
    int opt = 1;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));


    // 配置服务器地址结构
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(PORT);

    // 绑定套接字到地址
    if (bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0)
        error("bind error");

    // 开始监听
    listen(listenfd, 5);
    printf("服务器启动，监听端口 %d...\n", PORT);

    while (1) {
        // 接收客户端连接
        connfd = accept(listenfd, (struct sockaddr*)&cli_addr, &cli_len);
        if (connfd < 0) {
            perror("accept error");
            continue;
        }

        // 创建子进程处理客户端请求
        if (fork() == 0) {
            close(listenfd);  // 子进程不需要监听套接字
            handle_client(connfd);
        } else if (fork() < 0) {
            perror("fork error");
            close(connfd);    // 如果 fork 失败，关闭连接套接字
        } else {
            close(connfd);    // 父进程关闭连接套接字
        }
    }
    return 0;
}
