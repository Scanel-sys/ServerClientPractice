#include "tcpserver.h"

int init_netw_lib() 
{ 
#ifdef _WIN32 // Для Windows следует вызвать WSAStartup перед началом использования сокетов 
    WSADATA wsa_data; 
    return (0 == WSAStartup(MAKEWORD(2, 2), &wsa_data)); 
#else 
    return 1; // Для других ОС действий не требуется 
#endif 
}

void deinit_netw_lib() 
{ 
#ifdef _WIN32 // Для Windows следует вызвать WSACleanup в конце работы 
    WSACleanup(); 
#else // Для других ОС действий не требуется 
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
    fprintf(stderr, "%s: socket error: %d\n", function, err); 
    return -1; 
}

void s_close(int sock) 
{ 
#ifdef _WIN32 
    closesocket(sock); 
#else 
    close(sock); 
#endif 
}

int recv_string(int cs) 
{ 
    char buffer[512]; 
    int curlen = 0; 
    int rcv;

    do 
    { 
        int i; 
        rcv = recv(cs, buffer, sizeof(buffer), 0); 
        
        for (i = 0; i < rcv; i++) 
        { 
            if (buffer[i] == '\n') 
                return curlen; curlen++; 
        }

        if (curlen > 5000) 
        { 
            printf("input string too large\n"); 
            return 5000; 
        } 

    } while (rcv > 0);

    return curlen; 
}

int send_notice(int cs, int len) 
{ 
    char buffer[1024]; 
    int sent = 0; 
    int ret;

#ifdef _WIN32 
    int flags = 0; 
#else 
    int flags = MSG_NOSIGNAL; 
#endif

    sprintf(buffer, "Length of your string: %d chars.", len);
    
    while (sent < (int) strlen(buffer)) 
    { 
        ret = send(cs, buffer + sent, strlen(buffer) - sent, flags); 
        if (ret <= 0) 
            return sock_err("send", cs); 
        sent += ret; 
    }
    return 0; 
}
