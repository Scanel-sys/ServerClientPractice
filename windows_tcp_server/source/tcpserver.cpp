#include "tcpserver.h"


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

int parse_err(const char* function) 
{ 
    int err; 
#ifdef _WIN32 
    err = WSAGetLastError(); 
#else
    err = errno; 
#endif
    fprintf(stderr, "%s: parse error: %d\n", function, err); 
    return -1; 
}

int get_port(int argc, char *argv[])
{
    if(argc != 2)
        return parse_err("Wrong number of args");
 
    if(strlen(argv[1]) > 5 || check_if_not_ciphers(argv[1]))
        return parse_err("Wrong port is passed");

    int port = std::stoi(argv[1]);

    if(port < 0 || port > 65535)
        return parse_err("Wrong port is passed");
    
    return port;
}

bool check_if_not_ciphers(char *port)
{
    int i = 0;
    while(port[i] != '\0')
    {
        if(port[i] > '9' || port[i] < '0')
            return true;
        i++;
    }
    return false;
}

void close_socket(int sock) 
{ 
#ifdef _WIN32 
    closesocket(sock); 
#else 
    close(sock); 
#endif 
}


unsigned int get_client_ip(sockaddr_in sockaddr)
{
    return ntohl(sockaddr.sin_addr.s_addr);
}

std::string format_ip_to_str(unsigned int ip)
{
    return std::to_string((ip >> 24) & 0xFF) + "." + std::to_string((ip >> 16) & 0xFF) + "." +\
            std::to_string((ip >> 8) & 0xFF) + "." + std::to_string((ip) & 0xFF);
}

std::string format_port_to_str(int port)
{
    return std::to_string(port);
}

std::string generate_msg_metadata(sockaddr_in transport_addres, int port)
{
    return format_ip_to_str(get_client_ip(transport_addres)) + ":" + format_port_to_str(port) + " ";
}

int assemble_client_msg(sockaddr_in transport_addres, int port, client &temp_client, char msg_to_write[1024])
{
    enum MESSAGE_TYPE result = MSG;
    char client_msg[512];
    char date_and_time[15];
    ssize_t msg_len;

    for(int i = 0; i < 1024; i++)
    {
        if(i < 512)
            client_msg[i] = '\0';
        msg_to_write[i] = '\0';
    }

    if(temp_client.connected)
    {
        msg_len = recv(temp_client.socket, date_and_time, sizeof(date_and_time), 0);
        msg_len = recv(temp_client.socket, client_msg, sizeof(client_msg), 0);
        if(strcmp(client_msg, "stop") == 0)
            result = STOP;

        strcat(msg_to_write, generate_msg_metadata(transport_addres, port).c_str());
        strcat(msg_to_write, client_msg);
        printf("%s\n", msg_to_write);
    }
    else
    {
        msg_len = recv(temp_client.socket, client_msg, 3, 0);
        if(strcmp(client_msg, "put") == 0)
            result = PUT;
        else
        {
            result = ERROR;
            close_socket(temp_client.socket);
        }
    }
    return result;
}

int recv_string(int sock, char *buffer, int size) 
{ 
    int curlen = 0; 
    int rcv;

    do 
    { 
        int i; 
        rcv = recv(sock, buffer, sizeof(buffer), 0); 
        
        for (i = 0; i < rcv && i < size; i++) 
        { 
            if (buffer[i] == '\n') 
                return curlen; 
            curlen++; 
        }

        if (curlen > 5000) 
        { 
            printf("input string too large\n"); 
            return 5000; 
        } 

    } while (rcv > 0);

    return curlen; 
}

int recv_put(int sock, char *buffer)
{
    int res = recv(sock, buffer, 3, 0);
    if(res < 0)
    {
        sock_err("recv put", sock);
        return ERROR;
    }
    
    if(buffer[0] == 'p' &&
        buffer[1] == 'u' &&
        buffer[2] == 't')
        return PUT;
    
    sock_err("recv put", sock);
    return ERROR;
}

int send_notice(int sock, int len) 
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
        ret = send(sock, buffer + sent, strlen(buffer) - sent, flags); 
        if (ret <= 0) 
            return sock_err("send", sock); 
        sent += ret; 
    }
    return 0; 
}


void serveClients(serverData &server, std::ofstream &clients_data_file)
{
    std::vector <client> plugged_socks(0);
    int max_sock_value = server.socket; 
    client temp_client;
    timeval client_latency = { 1, 0 };
    char msg_to_write[1024];
    char buffer[512];
    int res;
    fd_set read_fd, write_fd; 
    bool server_can_work = true;

    while (server_can_work) 
    { 
        FD_ZERO(&read_fd); 
        FD_ZERO(&write_fd);
        FD_SET(server.socket, &read_fd);
        
        for (int i = 0; i < plugged_socks.size(); i++) 
        { 
            FD_SET(plugged_socks[i].socket, &read_fd); 
            FD_SET(plugged_socks[i].socket, &write_fd); 
            
            if (max_sock_value < plugged_socks[i].socket) 
                max_sock_value = plugged_socks[i].socket; 
        } 
        
        if (select(max_sock_value + 1, &read_fd, &write_fd, 0, &client_latency) > 0) 
        { 
            if (FD_ISSET(server.socket, &read_fd)) 
            { 
                temp_client.socket = Accept(server.socket, (struct sockaddr*) &server.ip, &server.addrlen);
                printf("Client connected : %d\n", temp_client.socket);
                set_non_block_mode(temp_client.socket);
                temp_client.connected = false;
                plugged_socks.push_back(temp_client);
            }
            for (int i = 0; i < plugged_socks.size(); i++) 
            { 
                if (FD_ISSET(plugged_socks[i].socket, &read_fd)) 
                {   
                    res = assemble_client_msg(server.ip, server.port, plugged_socks[i], msg_to_write);
                    
                    if(res == ERROR)
                    {
                        sock_err("assemble client msg", plugged_socks[i].socket);
                    }
                    else if(res != PUT)
                    {
                        send_ok(plugged_socks[i].socket);
                        clients_data_file << msg_to_write << '\n';
                        if(res == STOP)
                        {
                            server_can_work = false;
                            for(int i = 0; i < plugged_socks.size(); i++)
                                close_socket(plugged_socks[i].socket);
                        }
                    }
                    else
                    {
                        plugged_socks[i].connected = true;
                        printf("Put received from : %d\n", plugged_socks[i].socket);
                    }
                }
                /*
                if (FD_ISSET(plugged_socks[i].socket, &write_fd)) 
                { 

                    // Сокет plugged_socks[i] доступен для записи. Функция send и sendto будет успешно завершена 
                } 
                */
            } 
        }
        else 
        { 
            // Произошел таймаут или ошибка
        }
    }
}

int Socket(int domain, int type, int protocol)
{
	int sock = socket(domain, type, protocol);
	// af inet - ipv4
	// tcp - sock_stream
	// udp - sock_dgram
	// 0 - prtocol of lower lvl protocol
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

int Accept(int sock, sockaddr *addr, socklen_t *addrlen)
{
	int res = accept(sock, addr, addrlen);
	if (res == -1)
		return sock_err("accept", sock);

	return res;
}

int Listen(int sock, int backlog)
{
	int res = listen(sock, backlog);
	if (res == -1)
        return sock_err("listen", sock);

    return res;
}

int Send(int sock, const char * buf, int len, int flags)
{
	int res = send(sock, buf, len, flags);
	if(res == -1)
		return sock_err("send", sock); 

	return res;
}

int send_msg(int sock, const void * buf, int len) 
{ 
#ifdef _WIN32 
    int flags = 0; 
#else 
    int flags = MSG_NOSIGNAL; 
#endif

    // Отправка блока данных 
    int res = send(sock, buf, len, flags); 
    // if (res < 0) 
        // return sock_err("send", sock);

    // printf(" %d bytes sent.\n", len); 
    return 0; 
}

int send_ok(int sock)
{
    return send_msg(sock, "ok", 2);
}

void init_sockaddr(sockaddr_in &addr, int family, u_int32_t addres, int port)
{
    memset(&addr, 0, sizeof(addr)); 
    addr.sin_family = AF_INET; 
    addr.sin_port = htons(port); 
    addr.sin_addr.s_addr = htonl(addres);    // all adresses
}

std::string get_msg_file_path()
{
    return std::string(std::filesystem::current_path()) + "/msg.txt";
}