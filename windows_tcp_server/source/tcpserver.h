#ifdef _WIN32 
    #define WIN32_LEAN_AND_MEAN
    #include <windows.h> 
    #include <winsock2.h> 
    #include <WS2tcpip.h>
    #include <direct.h>
    #include <stdint.h>
    // Директива линковщику: использовать библиотеку сокетов 
    #pragma comment(lib, "ws2_32.lib") 
#else // LINUX 
    #include <sys/types.h> 
    #include <sys/socket.h> 
    #include <netdb.h> 
    #include <unistd.h>
    #include <errno.h> 
    #include <fcntl.h>      //lib for non blocking behaviour
    #define MAX_PATH              (1024)
#endif

#include <stdexcept>
#include <stdio.h> 
#include <string.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <limits.h>

#define  SERVICE_INFO_SIZE          (15)
#define  MSG_MAX_SIZE             (1024)

#define  FIRST_DAY_SHIFT             (4)
#define  FIRST_MONTH_SHIFT           (5)
#define  FIRST_YEAR_SHIFT            (6)
#define  SECOND_DAY_SHIFT            (8)
#define  SECOND_MONTH_SHIFT          (9)
#define  SECOND_YEAR_SHIFT          (10)
#define  HOURS_SHIFT                (12)
#define  MINUTES_SHIFT              (13)
#define  SECONDS_SHIFT              (14)

//enum MESSAGE_TYPE{ERROR_ENUM = -1, PUT = 0, STOP, MSG};
#define ERROR_ENUM						(-1)
#define	PUT							 (0)
#define STOP						 (1)
#define MSG							 (2)

struct Client{
    int socket;
    sockaddr_in ip;
    bool connected;
};

struct ServerData{
    int socket;
    int port;
    sockaddr_in ip;
    socklen_t addrlen;
    std::vector <struct Client> plugged_socks;
};

struct ParsedTime
{
    char hour;
    char min;
    char sec;
};

struct ParsedDate
{
    char day;
    char month;
    unsigned short year;
};

struct ParsedMessage
{
    struct ParsedDate date1;
    struct ParsedDate date2;
    struct ParsedTime time;
    std::string msg_text;
};

int init_netw_lib();
void deinit_netw_lib();
int set_non_block_mode(int sock);

unsigned int get_client_ip(sockaddr_in sockaddr);
std::string ip_to_str(unsigned int ip);
std::string port_to_str(int port);
std::string generate_msg_metadata(sockaddr_in transport_addres, int port);
std::string get_parsed_datetime(char raw_msg[MSG_MAX_SIZE]);
std::string get_parsed_msg_text(char raw_msg[MSG_MAX_SIZE]);
std::string date_time_to_str(struct ParsedMessage &new_msg);
std::string date_to_str(struct ParsedDate &date);
std::string time_to_str(struct ParsedTime &temp_time);
std::string int_to_str(int number);
int assemble_client_msg(struct ServerData &server, struct Client &temp_client, char msg_to_write[MSG_MAX_SIZE]);

int recv_string(int sock, char *buffer, int size);
int recv_put(int sock, char *buffer);

int sock_err(const char* function, int sock);
int parse_err(const char* function);

int get_port(int argc, char *argv[]);
bool check_if_not_ciphers(char *port);

void close_socket(int sock);
void close_sockets(struct ServerData &server);

int Socket(int domain, int type, int protocol);
int Bind(int sock, const sockaddr *addr, socklen_t addrlen);
int Accept(int sock, sockaddr *addr, socklen_t *addrlen);
int Listen(int sock, int backlog);
int Send(int sock, const char *buf, int len, int flags);
int send_msg(int sock, const void * buf, int len);
int send_ok(int sock);

void serveClients(ServerData &server, std::ofstream &clients_data_file);

void init_sockaddr(sockaddr_in &addr, int family, uint32_t addres, int port);

std::string get_msg_file_path();
