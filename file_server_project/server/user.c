#include <stdio.h>
#include <string.h>
#include "../common/common.h"

// 注册用户函数，返回 1 表示成功，0 表示用户名已存在
int register_user(const char *username, const char *password) {
    FILE *fp = fopen(USER_FILE, "a+");  // 读写追加模式
    if (!fp) {
        perror("open users.txt error");
        return 0;
    }

    char u[64], p[64], r[16];
    while (fscanf(fp, "%s %s %s", u, p, r) != EOF) {
        if (strcmp(u, username) == 0) {
            fclose(fp);
            return 0; // 用户已存在
        }
    }

    // 写入新用户，默认角色为 user
    fprintf(fp, "%s %s user\n", username, password);
    fclose(fp);
    return 1;
}

//用户登录
int login_user(const char *username, const char *password, char *role_out) {
    FILE *fp = fopen(USER_FILE, "r");
    if (!fp) {
        perror("open users.txt error");
        return 0;
    }

    char u[64], p[64], r[16];
    while (fscanf(fp, "%s %s %s", u, p, r) != EOF) {
        if (strcmp(u, username) == 0 && strcmp(p, password) == 0) {
            strcpy(role_out, r);  // 将角色写入输出参数
            fclose(fp);
            return 1;
        }
    }

    fclose(fp);
    return 0;
}

