#ifdef _WIN32 
    #define WIN32_LEAN_AND_MEAN 
    #include <windows.h> 
    #include <winsock2.h> // Директива линковщику: использовать библиотеку сокетов 
    #include <WS2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")

#else // LINUX 
    #include <sys/types.h> 
    #include <sys/socket.h> 
    #include <sys/time.h> 
    #include <sys/select.h> 
    #include <netdb.h> 
    #include <errno.h>
    #include <unistd.h>
    #include <arpa/inet.h>
    #include <netinet/in.h>
#endif

#include <sys/stat.h>
#include <stdio.h> 
#include <cstdlib>
#include <string.h>
#include <iostream>
#include <fstream>
#include <thread>
#include <sstream>
#include <vector>


#define DNS_PORT 53
#define MAX_PATH 32768
#define SERVER_DATAGRAM_SZ 80

union to_htonl{
    unsigned int htonl_index;
    char bytes[4];
};

struct datagram
{
    int msg_index;
    int msg_size;
    char msg[1024];
};

struct parsed_time
{
    char hour;
    char min;
    char sec;
};

struct parsed_date
{
    char day;
    char month;
    uint16_t year;
};

struct parsed_message
{
    struct parsed_date date1;
    struct parsed_date date2;
    struct parsed_time time;
    std::string msg_text;
};


int init_netw_lib();
void deinit_netw_lib();

void close_socket(int sock);
int Socket(int domain, int type, int protocol);
int sock_err(const char* function, int sock);
void init_udp_addr(sockaddr_in &addr, int family, const char *addres, int port);

bool if_file_exists(const std::string &fl_name);
int parse_err(const char* function);
int parse_cmd(int argc, char *argv[], char *addres, int &port, char fl_path[256]);
int parse_cmd_to_addr(char *cmd_addr, int &i, char *addres);
int parse_cmd_to_port(char *cmd_addr, int i);
int parse_cmd_to_path(char *cmd_flname, char *fl_path);

parsed_date parse_date(std::string date);
parsed_time parse_time(std::string time);
parsed_message parse_msg(std::string source);

int assemble_msg(parsed_message &msg_parts, char *result);

std::vector <datagram> 
get_datagrams(char *fl_path);


void send_request(int socket, struct sockaddr_in* addr);
void send_msg(int socket, struct sockaddr_in* addr, const void *msg, int msg_len);
std::vector<datagram> get_missed_msgs(std::vector<datagram> &datagrams, char *response_data_buff, int size);
unsigned int recv_response(int socket, char *datagram);

// Функция извлекает IPv4-адрес из DNS-дейтаграммы. 
unsigned int get_addr_from_dns_datagram(const char* datagram, int size);

std::vector<std::string> 
split_string(std::string str, std::string separator);
void clean_buffer(char *buffer, int size);