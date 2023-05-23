#ifdef _WIN32 
    #define WIN32_LEAN_AND_MEAN
    #include <windows.h> 
    #include <winsock2.h> 
    // Директива линковщику: использовать библиотеку сокетов 
    #pragma comment(lib, "ws2_32.lib") 
#else // LINUX 
    #include <sys/types.h> 
    #include <sys/socket.h> 
    #include <netdb.h> 
    #include <unistd.h>
    #include <errno.h> 

    #include <fcntl.h>      //lib for non blocking behaviour
#endif
    
#include <stdexcept>
#include <stdio.h> 
#include <string.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>


enum MESSAGE_TYPE{ERROR = -1, PUT = 0, STOP, MSG};

struct serverData{
    int socket;
    int port;
    sockaddr_in ip;
    socklen_t addrlen;
};

struct client{
    int socket;
    bool connected;
};

int init_netw_lib();
void deinit_netw_lib();
int set_non_block_mode(int sock);

unsigned int get_client_ip(sockaddr_in sockaddr);
std::string format_ip_to_str(unsigned int ip);
std::string format_port_to_str(int port);
std::string generate_msg_metadata(sockaddr_in transport_addres, int port);
std::string get_client_msg(int sock);
int assemble_client_msg(sockaddr_in transport_addres, int port, client &temp_client, char msg_to_write[1024]);

int recv_string(int sock, char *buffer, int size);
int send_notice(int sock, int len);
int recv_put(int sock, char *buffer);

int sock_err(const char* function, int sock);
int parse_err(const char* function);

int get_port(int argc, char *argv[]);
bool check_if_not_ciphers(char *port);

void close_socket(int sock);

int Socket(int domain, int type, int protocol);
int Bind(int sock, const sockaddr *addr, socklen_t addrlen);
int Accept(int sock, sockaddr *addr, socklen_t *addrlen);
int Listen(int sock, int backlog);
int Send(int sock, const char *buf, int len, int flags);
int send_msg(int sock, const void * buf, int len);
int send_ok(int sock);

void serveClients(serverData &server, std::ofstream &clients_data_file);

void init_sockaddr(sockaddr_in &addr, int family, u_int32_t addres, int port);

std::string get_msg_file_path();
