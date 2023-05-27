#include "udpserver.h"


int init_netw_lib() 
{ 
#ifdef _WIN32
    WSADATA wsa_data; 
    return (0 == WSAStartup(MAKEWORD(2, 2), &wsa_data)); 
#else 
    return 1;
#endif 
}

void deinit_netw_lib() 
{ 
#ifdef _WIN32
    WSACleanup(); 
#else
#endif 
}

int set_non_block_mode(int sock) 
{ 
#ifdef _WIN32 
    unsigned long mode = 1; 
    return ioctlsocket(sock, FIONBIO, &mode);
#else 
    int fl = fcntl(sock, F_GETFL, 0); 
    return fcntl(sock, F_SETFL, fl | O_NONBLOCK);
#endif 
}


int sock_err(const char* function, int sock) 
{ 
    int err; 
#ifdef _WIN32 
    err = WSAGetLastError(); 
#else
    err = errno; 
#endif
    fprintf(stderr, "%s : socket error: %d\n", function, err); 
    return -1; 
}

void close_socket(int sock) 
{ 
#ifdef _WIN32 
    closesocket(sock); 
#else 
    close(sock); 
#endif 
}

int Socket(int domain, int type, int protocol)
{
	int sock = socket(domain, type, protocol);
	if (sock == -1)
		return sock_err("socket", sock);

	return sock;
} 

int Bind(int sock, const sockaddr *addr, socklen_t addrlen)
{
	int res = bind(sock, addr, addrlen);
	if (res == -1)
		return sock_err("bind", sock);
	
    return res;
}

void init_sockaddr(sockaddr_in &addr, int family, u_int32_t addres, int port)
{
    memset(&addr, 0, sizeof(addr)); 
    addr.sin_family = AF_INET; 
    addr.sin_port = htons(port); 
    addr.sin_addr.s_addr = htonl(addres);
}
