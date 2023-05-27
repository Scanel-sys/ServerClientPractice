#include "udpserver.h"


int main() 
{ 
    serverData server;
    server.addrlen = sizeof(server.ip);
    int i;

#ifdef _WIN32 
    int flags = 0; 
#else 
    int flags = MSG_NOSIGNAL; 
#endif

    init_netw_lib();

    server.socket = Socket(AF_INET, SOCK_DGRAM, 0); 
    if (server.socket < 0) 
        return -1;
    
    init_sockaddr(server.ip, AF_INET, INADDR_ANY, 8000);

    if (Bind(server.socket, (struct sockaddr*) &server.ip, sizeof(server.ip)) < 0) 
        return -1;

    do { 
        char buffer[1024] = { 0 }; 
        int len = 0; 
        int rcv = recvfrom(server.socket, buffer, sizeof(buffer), 0, 
                            (struct sockaddr*) &server.ip, &server.addrlen); 
        
        if (rcv > 0) 
        { 
            unsigned int ip = ntohl(server.ip.sin_addr.s_addr);
            printf("Datagram received from address: %u.%u.%u.%u ", (ip >> 24) & 0xFF, (ip >> 16) & 0xFF, (ip >> 8) & 0xFF, (ip)& 0xFF);
        
            for (i = 0; i < rcv; i++) 
            { 
                if (buffer[i] == '\n') 
                    break; 
                len++; 
            }
            printf(" string len is: %d\n", len); 
        }
        
        sprintf(buffer, "Length of your string: %d chars.", len);

        sendto(server.socket, buffer, strlen(buffer), flags, (struct sockaddr*) &server.ip, server.addrlen);
    } while (1);

    close_socket(server.socket);
    deinit_netw_lib();
    return 0;
}