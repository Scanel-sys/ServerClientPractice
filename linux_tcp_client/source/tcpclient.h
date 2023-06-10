#ifdef _WIN32 
    #define WIN32_LEAN_AND_MEAN 
    #include <windows.h> 
    #include <winsock2.h> 
    #include <ws2tcpip.h> 
    #include <direct.h>
    // Директива линковщику: использовать библиотеку сокетов 
    #pragma comment(lib, "ws2_32.lib") 
#else // LINUX 
    #include <sys/types.h> 
    #include <sys/socket.h> 
    #include <limits.h>
    #include <unistd.h>
    #include <netdb.h> 
    #include <errno.h> 
    #include <unistd.h>
#endif

#include <sys/stat.h>
#include <stdio.h> 
#include <cstdlib>
#include <string.h>
#include <iostream>
#include <fstream>
#include <chrono>
#include <thread>
#include <sstream>
#include <vector>
#include <limits.h>

#define WEBHOST "google.com"

struct parsed_date;
struct parsed_time;
struct parsed_message;

int init_netw_lib();
void deinit_netw_lib();
void init_addr(struct sockaddr_in &addr, int family, const char *addres, int port);
int sock_err(const char* function, int sock);
void close_sock(int sock);

unsigned int get_host_ipn(const char* name);
unsigned int get_client_ip(sockaddr_in sockaddr);
std::string ip_to_str(unsigned int ip);

bool if_file_exists(const std::string &fl_name);
int parse_err(const char* function);
int parse_cmd(int argc, char *argv[], char *addres, int &port, char fl_path[256]);
int parse_cmd_to_addr(char *cmd_addr, int &i, char *addres);
int parse_cmd_to_port(char *cmd_addr, int i);
int parse_cmd_to_path(char *cmd_flname, char *fl_path);

parsed_date parse_date(std::string date);
parsed_time parse_time(std::string time);
parsed_message parse_msg(std::string source);

void handle_msg(int sock, std::string source);

void send_date(int sock, parsed_date &date_to_sent);
void send_time(int sock, parsed_time &time_to_sent);

int send_msg(int sock, const void * buf, int len);
int send_msg_index(int sock, int msg_index);
int send_client_msgs(int sock, std::ifstream &source);

int Socket(int domain, int type, int protocol);
int try_to_connect(int sock, sockaddr_in &addr);

void init_talk_with_server(int sock);
int send_request(int sock);
int recv_response_to_file(int sock, FILE* f);
int recv_response_once(int sock, char *buffer, int len);
bool recv_response_ok(int sock);
int recvn_response_ok(int sock, int msgs_number);

std::vector<std::string> 
split_string(std::string str, std::string separator);
