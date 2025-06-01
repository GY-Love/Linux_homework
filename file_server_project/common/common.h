#ifndef COMMON_H
#define COMMON_H

#define PORT 8888
#define MAXLINE 1024
#define USER_FILE "users.txt"
#define DATA_DIR "data/"

void error(const char *msg);
int register_user(const char *username, const char *password);
int login_user(const char *username, const char *password, char *role_out);


#endif

