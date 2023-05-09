#ifdef _WIN32 
    #define WIN32_LEAN_AND_MEAN 
    #include <windows.h> 
    #include <winsock2.h> 
    #include <ws2tcpip.h> 
    // Директива линковщику: использовать библиотеку сокетов 
    #pragma comment(lib, "ws2_32.lib") 
#else // LINUX 
    #include <sys/types.h> 
    #include <sys/socket.h> 
    #include <limits.h>
    #include <unistd.h>
    #include <netdb.h> 
    #include <errno.h> 
#endif

#include <stdio.h> 
#include <cstdlib>
#include <string.h>
#include <iostream>
#include <fstream>
#include <chrono>
#include <thread>
#include <sstream>
#include <vector>

#define WEBHOST "google.com"
#define MAX_PATH 32768

int init_netw_lib();
void deinit_netw_lib();
void init_addr(struct sockaddr_in &addr, int family, const char *addres, int port);

int sock_err(const char* function, int sock);
void s_close(int sock);

int try_to_connect(int sock, sockaddr_in &addr);

void get_msg(std::ifstream source, std::string destination);
void handle_msg(int sock, std::string source);

void send_date(int sock, std::string source);
void send_time(int sock, std::string source);

unsigned int get_host_ipn(const char* name);
int send_msg(int sock, const void * buf, int len);
int send_msg_index(int sock, int msg_index);
int send_client_msgs(int sock, std::ifstream &source);
void signal_to_server(int sock);
int send_request(int sock);
int recv_response(int sock, FILE* f);

int parse_err(int ret_code);
int parse_cmd(int argc, char *argv[], char *addres, int &port, char fl_path[256]);
int parse_cmd_to_addr(char *cmd_addr, int &i, char *addres);
int parse_cmd_to_port(char *cmd_addr, int i);
int parse_cmd_to_path(char *cmd_flname, char *fl_path);

void parse_msg(std::string source, std::string &date_1, std::string &date_2, std::string &time, std::string &msg);

unsigned char stouc(std::string source);
unsigned int stouint(std::string source);
unsigned short stous(std::string source);

std::vector<std::string> split_string(std::string str, std::string separator, int max_splits = 0);
