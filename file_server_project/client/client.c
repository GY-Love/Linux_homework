#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "../common/common.h"

int main() {
    int sockfd;
    struct sockaddr_in serv_addr;
    char buffer[MAXLINE];

    // 创建 socket
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket error");
        exit(EXIT_FAILURE);
    }

    // 设置服务器地址
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    // 连接服务器
    if (connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("connect error");
        exit(EXIT_FAILURE);
    }

    printf("已连接服务器：127.0.0.1:%d\n", PORT);

  while (1) {
    printf("请输入命令（register/login/upload/download/list/exit）：");
    fgets(buffer, sizeof(buffer), stdin);

    // 判断是否退出
    if (strncmp(buffer, "exit", 4) == 0) break;

    // 判断是否是 register 命令（用户输入）
    if (strncmp(buffer, "register", 8) == 0) {
        send(sockfd, buffer, strlen(buffer), 0);  // 发送 register 命令

        recv(sockfd, buffer, sizeof(buffer), 0); buffer[strcspn(buffer, "\n")] = 0; // 服务器要用户名提示
        printf("%s", buffer);
        fgets(buffer, sizeof(buffer), stdin);
        send(sockfd, buffer, strlen(buffer), 0);

        recv(sockfd, buffer, sizeof(buffer), 0); buffer[strcspn(buffer, "\n")] = 0; // 服务器要密码提示
        printf("%s", buffer);
        fgets(buffer, sizeof(buffer), stdin);
        send(sockfd, buffer, strlen(buffer), 0);

        recv(sockfd, buffer, sizeof(buffer), 0); buffer[strcspn(buffer, "\n")] = 0; // 注册成功 or 失败
        printf("服务器回复：%s\n", buffer);
        continue;  // 回到输入命令
    }

    //判断是否是login命令
    if (strncmp(buffer, "login", 5) == 0) {
    send(sockfd, buffer, strlen(buffer), 0);

    recv(sockfd, buffer, sizeof(buffer), 0);
    buffer[strcspn(buffer, "\n")] = 0;
    printf("%s", buffer);
    fgets(buffer, sizeof(buffer), stdin);
    buffer[strcspn(buffer, "\n")] = 0;
    send(sockfd, buffer, strlen(buffer), 0);

    recv(sockfd, buffer, sizeof(buffer), 0);
    buffer[strcspn(buffer, "\n")] = 0;
    printf("%s", buffer);
    fgets(buffer, sizeof(buffer), stdin);
    buffer[strcspn(buffer, "\n")] = 0;
    send(sockfd, buffer, strlen(buffer), 0);

    recv(sockfd, buffer, sizeof(buffer), 0);
    buffer[strcspn(buffer, "\n")] = 0;
    printf("服务器回复：%s\n", buffer);
    continue;
}

// 文件上传功能
if (strncmp(buffer, "upload", 6) == 0) {
    char filename[64];
    sscanf(buffer + 7, "%s", filename);  // 提取文件名

    FILE *fp = fopen(filename, "rb");
    if (!fp) {
        printf("找不到文件：%s\n", filename);
        continue;
    }

    send(sockfd, buffer, strlen(buffer), 0);  // 发送命令

    recv(sockfd, buffer, sizeof(buffer), 0);  // 服务器回复“开始上传”
    buffer[strcspn(buffer, "\n")] = 0;
    printf("服务器回复：%s\n", buffer);

    char filebuf[1024];
    int n;
    while ((n = fread(filebuf, 1, sizeof(filebuf), fp)) > 0) {
        send(sockfd, filebuf, n, 0);
    }

    fclose(fp);
    // 通知服务器上传完毕
    send(sockfd, "EOF", 3, 0);

    recv(sockfd, buffer, sizeof(buffer), 0);  // 最后确认
    buffer[strcspn(buffer, "\n")] = 0;
    printf("服务器确认：%s\n", buffer);
    continue;
}

//文件下载功能
if (strncmp(buffer, "download", 8) == 0) {
    char filename[64];
    sscanf(buffer + 9, "%s", filename);

    send(sockfd, buffer, strlen(buffer), 0);  // 发送 download 命令

    recv(sockfd, buffer, sizeof(buffer), 0);
    buffer[strcspn(buffer, "\n")] = 0;
    if (strcmp(buffer, "开始下载") != 0) {
        printf("服务器回复：%s\n", buffer);
        continue;
    }

    printf("服务器回复：%s，准备接收文件...\n", buffer);

    FILE *fp = fopen(filename, "wb");
    if (!fp) {
        printf("本地文件创建成功：%s\n", filename);
        continue;
    }

    char filebuf[1024];
    int n;
    while ((n = recv(sockfd, filebuf, sizeof(filebuf), 0)) > 0) {
        if (n == 3 && strncmp(filebuf, "EOF", 3) == 0) break;
        fwrite(filebuf, 1, n, fp);
    }

    fclose(fp);
    printf("文件下载完成，保存为：%s\n", filename);
    continue;
}




    // 默认处理其他命令（还没实现 login/download 等时）
    send(sockfd, buffer, strlen(buffer), 0);
    memset(buffer, 0, sizeof(buffer));
    recv(sockfd, buffer, sizeof(buffer), 0); buffer[strcspn(buffer, "\n")] = 0;
    printf("来自服务器的响应：%s\n", buffer);
}


    close(sockfd);
    return 0;
}
