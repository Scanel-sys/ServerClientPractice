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
    #include <errno.h> 
#endif
    
#include <stdio.h> 
#include <string.h>

int init_netw_lib();
void deinit_netw_lib();

int recv_string(int cs);
int send_notice(int cs, int len);

int sock_err(const char* function, int sock);

