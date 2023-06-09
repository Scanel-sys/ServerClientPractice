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

struct ServerData init_server(int argc, char *argv[])
{
    struct ServerData server;
    server.ports = get_and_init_ports(argc, argv);
    server.addrlen = sizeof(server.ports[0].ip);
    return server;
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

std::vector <PortData> get_and_init_ports(int argc, char *argv[])
{
    int number_of_ports = 1;
    int first_port = 0, second_port = 0;
    if(argc < 2)
    {
        sock_err("get_port", -1);
        exit(-1);
    }
    
    first_port = char_to_int(argv[1]);
    if(first_port < 1)
    {
        sock_err("wrong port passed", -1);
        exit(-1);
    }

    if(argc > 2)
    {
        second_port = char_to_int(argv[2]);
        number_of_ports = second_port - first_port + 1;
        if(number_of_ports < 1)
        {
            sock_err("wrong ports diapason", -1);
            exit(-1);
        }
    }

    std::vector <PortData> new_ports(0);
    struct PortData temp_portData;
    for(int i = 0; i < number_of_ports; i++)
    {
        temp_portData.socket = Socket(AF_INET, SOCK_DGRAM, 0);
        if (temp_portData.socket < 0) 
            exit(-1);
        
        temp_portData.port = first_port + i;
        init_sockaddr(temp_portData.ip, AF_INET, INADDR_ANY, temp_portData.port);
        
        new_ports.push_back(temp_portData);
    }
    return new_ports;
}

void set_non_block_mode_to_server(struct ServerData &server)
{
    for(int i = 0; i < server.ports.size(); i++)
        set_non_block_mode(server.ports[i].socket);
}

void close_server_sockets(struct ServerData server)
{
    for(int i = 0; i < server.ports.size(); i++)
        close_socket(server.ports[i].socket);
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

void bind_ports(struct ServerData &server)
{
    for(int i = 0; i < server.ports.size(); i++)
    {
        if(Bind(server.ports[i].socket, (struct sockaddr*) &server.ports[i].ip, server.addrlen) < 0)
            exit(-1);
        printf("Port %d binded\n", server.ports[i].port);
    }
}

void init_sockaddr(sockaddr_in &addr, int family, u_int32_t addres, int port)
{
    memset(&addr, 0, sizeof(addr)); 
    addr.sin_family = AF_INET; 
    addr.sin_port = htons(port); 
    addr.sin_addr.s_addr = htonl(addres);
}


void add_ports_to_epoll_queue(struct ServerData &server, int epoll_queue)
{
    for(int i = 0; i < server.ports.size(); i++)
    {
        server.ports[i].event.events = EPOLLIN;
        server.ports[i].event.data.fd = i;

        if(epoll_ctl(epoll_queue, EPOLL_CTL_ADD, server.ports[i].socket, &server.ports[i].event) != 0)
        {
            printf("epoll_ctl(s) error: %d\n", errno);
            exit(-1);
        }
    }
}

int create_epoll_queue()
{
    int epfd = epoll_create(1);
    if(epfd <= 0)
    {
        printf("epoll queue error\n");
        exit(-1);
    }
    return epfd;
}

void launch_server_with_epoll(struct ServerData &server, std::ofstream &input_msgs_file)
{
    struct epoll_event events[MAXEVENTS_PER_CALL];
    int epoll_queue = create_epoll_queue();
    
    add_ports_to_epoll_queue(server, epoll_queue);
    printf("Epoll queue inited\n");
    
    bool stop_received = false;
    int i, events_cnt, client_index, msg_type_recv, idx;
    uint32_t recv_msg_numb;
    char raw_msg[1024] = {0};
    char new_msg[1024] = {0};
    struct ClientData temp_client;

    do { 
        events_cnt = epoll_wait(epoll_queue, events, MAXEVENTS_PER_CALL, 1000);
        if (events_cnt == 0) 
            {continue;} 
        
        for(i = 0; i < events_cnt; i++) 
        { 
            struct epoll_event* temp_event = &events[i]; 
            if (temp_event->data.fd >= 0) 
            { 
                idx = temp_event->data.fd; 
                if (temp_event->events & EPOLLIN) 
                { 
                    recv_msg(server, idx, raw_msg, RAW_MSG_SIZE);
                    temp_client = init_temp_client(server.ports[idx]);
                    client_index = get_client_index(server, temp_client);
                    recv_msg_numb = get_msg_number_netformat(raw_msg);
                    
                    if(client_index == NEW_CLIENT)
                    {
                        temp_client.recv_msg_numb.clear();
                        server.client_database.push_back(temp_client);
                        client_index = server.client_database.size() - 1;
                    }
                    else if(if_old_msg(server.client_database[client_index], recv_msg_numb))
                    {
                        send_msg_number(server.ports[idx], &recv_msg_numb);
                        continue;
                    }

                    send_msg_number(server.ports[idx], &recv_msg_numb);
                    msg_type_recv = assemble_client_msg(server.ports[idx], raw_msg, new_msg);
                    if(msg_type_recv == STOP)
                    {
                        stop_received = true;
                        printf(" STOP received\n");
                    }
                    write_msg_to_file(input_msgs_file, new_msg);
                }
                // else if (temp_event->events & EPOLLOUT) 
                // { 
                //     // ready for sending
                // } 
            }
        }
    } while (!stop_received);
}


int get_client_index(struct ServerData &server, struct ClientData &client)
{
    int i = 0;
    while(i < server.client_database.size())
    {
        if(client.ip == server.client_database[i].ip &&
            client.port == server.client_database[i].port)
            break;
        i++;
    }

    if(server.client_database.size() == i)
        return -1;
    
    return i;
}

struct ClientData init_temp_client(struct PortData &portData)
{
    struct ClientData result;
    result.port = portData.port;
    result.ip = portData.ip.sin_addr.s_addr;
    result.recv_msg_numb.clear();
    return result;
}


uint32_t get_msg_index(char *raw_msg)
{
    uint32_t msg_index;
    memcpy(&msg_index, raw_msg, 4);
    msg_index = ntohl(msg_index);
    return msg_index;
}

bool if_old_msg(struct ClientData &client, uint32_t msg_numb)
{
    bool if_found = false;
    for(int i = 0; i < client.recv_msg_numb.size() && !if_found; i++)
        if(client.recv_msg_numb[i] == msg_numb)
            if_found = true;

    return if_found;
}


uint32_t ip_to_normal_format(sockaddr_in sockaddr)
{
    return ntohl(sockaddr.sin_addr.s_addr);
}

std::string ip_to_str(uint32_t ip)
{
    return std::to_string((ip >> 24) & 0xFF) + "." + std::to_string((ip >> 16) & 0xFF) + "." +\
        std::to_string((ip >> 8) & 0xFF) + "." + std::to_string((ip) & 0xFF);
}

std::string port_to_str(int port)
{
    return std::to_string(port);
}

int char_to_int(char *raw_msg)
{
    int result = 0;
    for(int i = 0; raw_msg[i] != '\0'; i++)
    {
        if(i < 5 && '0' <= raw_msg[i] && raw_msg[i] <= '9')
            result = result * 10 + raw_msg[i] - '0';
        else
        {
            sock_err("char_to_int not appropriate number", -1);
            exit(-1);
        }
    }
    return result;
}


std::string gen_msg_metadata(struct PortData &msg_dealer)
{
    return ip_to_str(ip_to_normal_format(msg_dealer.ip)) + ":" + port_to_str(msg_dealer.port) + " ";
}

int assemble_client_msg(struct PortData &msg_dealer, char raw_msg[1024], char msg_raw_msg[1024])
{
    enum MESSAGE_TYPE result = MSG;
    
    for(int i = 0; i < 1024; i++)
        msg_raw_msg[i] = '\0';

    if(strcmp(raw_msg + SERVICE_INFO_SIZE, "stop") == 0)
        result = STOP;

    strcat(msg_raw_msg, gen_msg_metadata(msg_dealer).c_str());
    strcat(msg_raw_msg, raw_msg + SERVICE_INFO_SIZE);

    return result;
}

void write_msg_to_file(std::ofstream &file, char source[1024])
{
    file << source << '\n';
}

uint32_t get_msg_number_netformat(char *raw_msg)
{
    uint32_t result;
    memcpy(&result, raw_msg, 4);
    return result;
}

void recv_msg(struct ServerData &server, int port_index, char *place_for_msg, size_t buff_len)
{
    recvfrom(server.ports[port_index].socket, place_for_msg, RAW_MSG_SIZE, 0, 
                (struct sockaddr *)&server.ports[port_index].ip, &server.addrlen); 
}

int send_msg_number(PortData &port_data, const void *number)
{
    return send_msg(port_data, number, 4);
}

int send_msg(PortData &port_data, const void *raw_msg, size_t buff_len)
{
#ifdef _WIN32 
    int flags = 0; 
#else 
    int flags = MSG_NOSIGNAL; 
#endif

    int res = sendto(port_data.socket, raw_msg, buff_len, flags, (struct sockaddr*) &port_data.ip, sizeof(port_data.ip));
    if(res < 0)
        return sock_err("send_msg", port_data.socket);
    return res;
}


std::string get_msg_file_path()
{
    return std::string(std::filesystem::current_path()) + "/msg.txt";
}
