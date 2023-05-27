#ifdef _WIN32 
    #define WIN32_LEAN_AND_MEAN 
    #include <windows.h> 
    #include <winsock2.h> 
    #pragma comment(lib, "ws2_32.lib") 
#else // LINUX 
    #include <sys/types.h> 
    #include <sys/socket.h> 
    #include <sys/time.h> 
    #include <sys/select.h> 
    #include <netdb.h> 
    #include <errno.h> 
    #include <fcntl.h>      //lib for non blocking behaviour
#endif

#include <stdio.h> 
#include <string.h>
#include <fstream>
#include <unistd.h>


struct serverData{
    int socket;
    int port;
    sockaddr_in ip;
    socklen_t addrlen;
};

int init_netw_lib();
void deinit_netw_lib();
int set_non_block_mode(int sock);

int sock_err(const char* function, int sock);
void close_socket(int sock);
int Socket(int domain, int type, int protocol);
int Bind(int sock, const sockaddr *addr, socklen_t addrlen);
void init_sockaddr(sockaddr_in &addr, int family, u_int32_t addres, int port);