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

void close_sockets(struct ServerData &server)
{
    for(int i = 0; i < server.plugged_socks.size(); i++)
    {
        if(server.plugged_socks[i].connected)
        {
            close_socket(server.plugged_socks[i].socket);
            printf("Peer disconnected : %s\n", ip_to_str(get_client_ip(server.plugged_socks[i].ip)).c_str());
        }
    }
}


unsigned int get_client_ip(sockaddr_in sockaddr)
{
    return ntohl(sockaddr.sin_addr.s_addr);
}

std::string ip_to_str(unsigned int ip)
{
    return std::to_string((ip >> 24) & 0xFF) + "." + std::to_string((ip >> 16) & 0xFF) + "." +\
            std::to_string((ip >> 8) & 0xFF) + "." + std::to_string((ip) & 0xFF);
}

std::string port_to_str(int port)
{
    return std::to_string(port);
}

std::string generate_msg_metadata(sockaddr_in transport_addres, int port)
{
    return ip_to_str(get_client_ip(transport_addres)) + ":" + port_to_str(port) + " ";
}

std::string get_parsed_datetime(char raw_msg[MSG_MAX_SIZE])
{
    struct ParsedMessage new_msg;

    memcpy(&new_msg.date1.day, raw_msg + FIRST_DAY_SHIFT, sizeof(new_msg.date1.day));
    memcpy(&new_msg.date1.month, raw_msg + FIRST_MONTH_SHIFT, sizeof(new_msg.date1.month));
    memcpy(&new_msg.date1.year, raw_msg + FIRST_YEAR_SHIFT, sizeof(new_msg.date1.year));
    new_msg.date1.year = ntohs(new_msg.date1.year);

    memcpy(&new_msg.date2.day, raw_msg + SECOND_DAY_SHIFT, sizeof(new_msg.date1.day));
    memcpy(&new_msg.date2.month, raw_msg + SECOND_MONTH_SHIFT, sizeof(new_msg.date1.month));
    memcpy(&new_msg.date2.year, raw_msg + SECOND_YEAR_SHIFT, sizeof(new_msg.date1.year));
    new_msg.date2.year = ntohs(new_msg.date2.year);

    memcpy(&new_msg.time.hour, raw_msg + HOURS_SHIFT, sizeof(new_msg.time.hour));
    memcpy(&new_msg.time.min, raw_msg + MINUTES_SHIFT, sizeof(new_msg.time.min));
    memcpy(&new_msg.time.sec, raw_msg + SECONDS_SHIFT, sizeof(new_msg.time.sec));

    new_msg.msg_text = date_time_to_str(new_msg);
    return new_msg.msg_text;
}

std::string get_parsed_msg_text(char raw_msg[MSG_MAX_SIZE])
{
    char temp[MSG_MAX_SIZE];
    temp[0] = '\0';
    strcat(temp, raw_msg + SERVICE_INFO_SIZE);
    std::string result(temp);
    return result;
}

std::string date_time_to_str(struct ParsedMessage &new_msg)
{
    std::string result;
    result += std::to_string(new_msg.date1.day) + ".";
    result += std::to_string(new_msg.date1.month) + ".";
    result += std::to_string(new_msg.date1.year) + " ";

    result += std::to_string(new_msg.date2.day) + ".";
    result += std::to_string(new_msg.date2.month) + ".";
    result += std::to_string(new_msg.date2.year) + " ";

    result += std::to_string(new_msg.time.hour) + ":";
    result += std::to_string(new_msg.time.min) + ":";
    result += std::to_string(new_msg.time.sec) + " ";
    return result;
}

int assemble_client_msg(struct ServerData &server, struct Client &temp_client, char msg_to_write[MSG_MAX_SIZE])
{
    enum MESSAGE_TYPE result = MSG;

    for(int i = 0; i < MSG_MAX_SIZE; i++)
        msg_to_write[i] = '\0';
    
    if(temp_client.connected)
    {
        char raw_msg[MSG_MAX_SIZE] = {0};
        recv(temp_client.socket, raw_msg, MSG_MAX_SIZE, 0);
        std::string msg_text = get_parsed_msg_text(raw_msg);
        if(strcmp(msg_text.c_str(), "stop") == 0)
            result = STOP;

        strcat(msg_to_write, generate_msg_metadata(server.ip, server.port).c_str());
        strcat(msg_to_write, get_parsed_datetime(raw_msg).c_str());
        strcat(msg_to_write, msg_text.c_str());
    }
    else
    {
        char buffer[4];
        recv(temp_client.socket, buffer, 3, 0);
        if(strcmp(buffer, "put") == 0)
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


void serveClients(ServerData &server, std::ofstream &clients_data_file)
{
    int max_sock_value = server.socket; 
    Client temp_client;
    timeval client_latency = { 1, 0 };
    char msg_to_write[MSG_MAX_SIZE];
    int res;
    fd_set read_fd, write_fd; 
    bool server_can_work = true;

    while (server_can_work) 
    { 
        FD_ZERO(&read_fd); 
        FD_ZERO(&write_fd);
        FD_SET(server.socket, &read_fd);
        
        for (int i = 0; i < server.plugged_socks.size(); i++) 
        { 
            FD_SET(server.plugged_socks[i].socket, &read_fd); 
            FD_SET(server.plugged_socks[i].socket, &write_fd); 
            
            if (max_sock_value < server.plugged_socks[i].socket) 
                max_sock_value = server.plugged_socks[i].socket; 
        } 
        
        if (select(max_sock_value + 1, &read_fd, &write_fd, 0, &client_latency) > 0) 
        { 
            if (FD_ISSET(server.socket, &read_fd)) 
            { 
                temp_client.socket = Accept(server.socket, (struct sockaddr*) &server.ip, &server.addrlen);
                set_non_block_mode(temp_client.socket);
                temp_client.connected = false;
                temp_client.ip = server.ip;
                server.plugged_socks.push_back(temp_client);
            }
            for (int i = 0; i < server.plugged_socks.size(); i++) 
            { 
                if (FD_ISSET(server.plugged_socks[i].socket, &read_fd)) 
                {   
                    res = assemble_client_msg(server, server.plugged_socks[i], msg_to_write);
                    
                    if(res == ERROR)
                    {
                        sock_err("assemble client msg", server.plugged_socks[i].socket);
                    }
                    else if(res != PUT)
                    {
                        send_ok(server.plugged_socks[i].socket);
                        clients_data_file << msg_to_write << '\n';
                        if(res == STOP)
                        {
                            printf("'stop' received\n");
                            
                            server_can_work = false;
                            for(int i = 0; i < server.plugged_socks.size(); i++)
                                close_socket(server.plugged_socks[i].socket);
                        }
                    }
                    else
                    {
                        server.plugged_socks[i].connected = true;
                        printf("Peer connected : %s\n", ip_to_str(get_client_ip(server.plugged_socks[i].ip)).c_str());
                    }
                }
                /*
                if (FD_ISSET(server.plugged_socks[i].socket, &write_fd)) 
                { 

                    // Сокет server.plugged_socks[i] доступен для записи. Функция send и sendto будет успешно завершена 
                } 
                */
            } 
        }
        else 
        { 
            // Произошел таймаут или ошибка
        }
    }
    close_sockets(server);
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
    char path[PATH_MAX];
#ifdef _WIN32 
    if(_getcwd(path, PATH_MAX) == NULL)
    {
        perror("_getcwd error");
        exit(-1);
    }
#else
    if(getcwd(path, PATH_MAX) == NULL)
    {
        perror("getcwd error");
        exit(-1);
    }
#endif
    std::string result;
    result.assign(path);
    result += "/msg.txt";
    return result;
}