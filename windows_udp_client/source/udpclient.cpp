#include "udpclient.h"


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

void init_udp_addr(sockaddr_in &addr, int family, const char *addres, int port)
{
    memset(&addr, 0, sizeof(addr)); 
    addr.sin_family = family; 
    addr.sin_port = htons(port); 
    addr.sin_addr.s_addr = inet_addr(addres);
}


bool if_file_exists(const std::string &fl_name)
{
    struct stat buffer;
    return (stat (fl_name.c_str(), &buffer) == 0);
}

int parse_err(const char* function) 
{ 
    int err; 
#ifdef _WIN32 
    err = WSAGetLastError(); 
#else
    err = errno; 
#endif
    fprintf(stderr, "%socket: parse error: %d\n", function, err); 
    return -1; 
}

int parse_cmd(int argc, char *argv[], char *addres, int &port, std::string &file_path)
{
    if(argc != 3)
        return parse_err("Wrong number of parameters passed");
 
    int i = 0;

    if(parse_cmd_to_addr(argv[1], i, addres) != 0)
        return parse_err("Bad format of addres (parse_cmd_to_addr)");
    
    port = parse_cmd_to_port(argv[1], i);
    if(port < 0)
        return parse_err("Wrong port is passed");
    
    file_path = parse_cmd_to_path(argv[2]);

    return 0;
}

int parse_cmd_to_addr(char *cmd_addr, int &i, char *addres)
{
    for(; cmd_addr[i] != '\0' && cmd_addr[i] != ':'; i++)
        addres[i] = cmd_addr[i];
    addres[i] = '\0';
    
    if(cmd_addr[i] == ':')
        i++;
    else
        return -1;

    return 0;
}

int parse_cmd_to_port(char *cmd_addr, int i)
{
    int port = 0;
    while(cmd_addr[i] != '\0')
    {
        if(cmd_addr[i] < '0' || cmd_addr[i] > '9')
            return -1;
        
        port *= 10;
        port += cmd_addr[i] - '0';

        i++;
    }
    return port;
}

std::string parse_cmd_to_path(char *cmd_flname)
{
    char path[MAX_PATH];
#ifdef _WIN32 
    if(_getcwd(path, MAX_PATH) == NULL)
    {
        perror("_getcwd error");
        exit(-1);
    }
#else
    if(getcwd(path, MAX_PATH) == NULL)
    {
        perror("getcwd error");
        exit(-1);
    }
#endif

    std::string result;
    result.assign(path);
    result += "/";
    result.assign(cmd_flname);
    return result;
}

parsed_message parse_msg(std::string source)
{
    parsed_message temp_message;
    std::stringstream str_stream(source);
    std::string temp_str;
    
    char dummy;
    str_stream >> std::noskipws;

    str_stream >> temp_str;
    str_stream.clear();
    str_stream >> dummy;
    temp_message.date1 = parse_date(temp_str);

    str_stream >> temp_str;
    str_stream.clear();
    str_stream >> dummy;
    temp_message.date2 = parse_date(temp_str);
    
    str_stream >> temp_str;
    str_stream.clear();
    str_stream >> dummy;
    temp_message.time = parse_time(temp_str);
    
    getline(str_stream, temp_message.msg_text);

    return temp_message;
}

parsed_date parse_date(std::string date)
{
    parsed_date temp_date;
    std::vector <std::string> data = split_string(date, ".");
    
    temp_date.day = (char)std::stoi(data[0]);
    temp_date.month = (char)std::stoi(data[1]);
    temp_date.year = htons((uint16_t)std::stoi(data[2]));

    return temp_date;
}

parsed_time parse_time(std::string time)
{
    parsed_time temp_time;
    std::vector <std::string> data = split_string(time, ":");

    temp_time.hour = (char)std::stoi(data[0]);
    temp_time.min = (char)std::stoi(data[1]);
    temp_time.sec = (char)std::stoi(data[2]);

    return temp_time;
}

int assemble_msg(parsed_message &msg_parts, char *result)
{
    memcpy(result + 4, &msg_parts.date1.day, 1);
    memcpy(result + 5, &msg_parts.date1.month, 1);
    memcpy(result + 6, &msg_parts.date1.year, 2);
    
    memcpy(result + 8, &msg_parts.date2.day, 1);
    memcpy(result + 9, &msg_parts.date2.month, 1);
    memcpy(result + 10, &msg_parts.date2.year, 2);

    memcpy(result + 12, &msg_parts.time.hour, 1);
    memcpy(result + 13, &msg_parts.time.min, 1);
    memcpy(result + 14, &msg_parts.time.sec, 1);

    memcpy(result + 15, msg_parts.msg_text.c_str(), msg_parts.msg_text.size());

    return msg_parts.msg_text.size() + 16;
}

std::vector <datagram> 
get_datagrams(std::string file_path)
{
    std::ifstream data_file;
    
    unsigned int msg_index = 0;
    unsigned net_msg_index;
    std::vector <datagram> datagrams(0);
    datagram temp_datagram;
    parsed_message msg_parts;
    std::string buffer;

    data_file.open(file_path);
    while(std::getline(data_file, buffer))
    {
        if(buffer.length() == 0)
            continue;
             
        clean_buffer(temp_datagram.msg, 1024);
        temp_datagram.msg_index = msg_index;
        net_msg_index = htonl(msg_index++);
        memcpy(temp_datagram.msg, &net_msg_index, 4);

        msg_parts = parse_msg(buffer);
        temp_datagram.msg_size = assemble_msg(msg_parts, temp_datagram.msg);

        datagrams.push_back(temp_datagram);
    }
    data_file.close();

    return datagrams;
}

void send_request(int socket, struct sockaddr_in* addr) 
{ 
    // Данные дейтаграммы DNS-запроса. Детальное изучение для л/р не требуется. 
    char dns_datagram[] = {0x00, 0x00, 0x00, 0x00, 
                            0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
                            3, 'w', 'w','w', 6, 'y','a','n','d','e','x', 2, 'r', 'u', 0, 
                            0x00, 0x01, 0x00, 0x01};

#ifdef _WIN32 
    int flags = 0; 
#else 
    int flags = MSG_NOSIGNAL; 
#endif
    int res = sendto(socket, dns_datagram, sizeof(dns_datagram), flags, (struct sockaddr*) addr, sizeof(struct sockaddr_in)); 
    if (res <= 0) 
        sock_err("sendto", socket); 
}

void send_msg(int socket, struct sockaddr_in* addr, const void *msg, int msg_len)
{ 
#ifdef _WIN32 
    int flags = 0; 
#else 
    int flags = MSG_NOSIGNAL; 
#endif
    if (sendto(socket, (const char *)msg, msg_len, flags, (struct sockaddr*) addr, sizeof(struct sockaddr_in)) <= 0) 
        sock_err("sendto", socket); 
}

std::vector<datagram> get_missed_msgs(std::vector<datagram> &datagrams, char *response_data_buff, int size)
{
    int temp_msg_number;
    std::vector<datagram> result = datagrams;
    for(int i = 0; i < 80; i+=4)
    {
        memcpy(&temp_msg_number, &response_data_buff[i], 4);
        temp_msg_number = ntohl(temp_msg_number);
        if(temp_msg_number >= 0)
        {
            bool found_msg = false;
            for(unsigned int counter = 0; counter < result.size() && !found_msg; counter++)
            {
                if(result[counter].msg_index == temp_msg_number)
                {
                    result.erase(result.begin() + counter);
                    found_msg = true;
                }
            }
        }
    }
    return result;
}

unsigned int recv_response(int socket, char *datagram)
{ 
    clean_buffer(datagram, SERVER_DATAGRAM_SZ);
    struct timeval tv = {0, 100*1000}; // 100 msec
    int res;

    fd_set read_fd; 
    FD_ZERO(&read_fd); 
    FD_SET(socket, &read_fd);
    
    res = select(socket + 1, &read_fd, 0, 0, &tv); 
    if (res > 0) 
    { 
        struct sockaddr_in addr; 
        socklen_t addrlen = sizeof(addr);
        int received = recvfrom(socket, datagram, SERVER_DATAGRAM_SZ,
                                 0, (struct sockaddr*) &addr, &addrlen);
        if (received <= 0) 
            return sock_err("recvfrom", socket);

        return res;
    } 
    else if (res == 0) 
    { 
        //nodata
        return 0; 
    } 
    else 
    { 
        return sock_err("select", socket);
    } 
}

unsigned int get_addr_from_dns_datagram(const char* datagram, int size) 
{ 
    unsigned short req_cnt, ans_cnt, i; 
    const char* ptr;
    req_cnt = ntohs(*(unsigned short*)(datagram + 4)); 
    ans_cnt = ntohs(*(unsigned short*)(datagram + 6));
    ptr = datagram + 12; 
    
    for (i = 0; i < req_cnt; i++) 
    { 
        unsigned char psz; 
        do 
        { 
            psz = *ptr; 
            ptr += psz + 1; 
        } while (psz > 0); 
        ptr += 4; 
    }
    
    for (i = 0; i < ans_cnt; i++) 
    { 
        unsigned char psz; 
        unsigned short asz; 
        do 
        { 
            psz = *ptr; 
            if (psz & 0xC0) 
            { 
                ptr += 2; 
                break; 
            }
            ptr += psz + 1; 
        } while (psz > 0);

        ptr += 8; 
        asz = ntohs(*(unsigned short*)ptr);
        if (asz == 4) 
        { 
            printf(" Found IP: %u.%u.%u.%u\n", (unsigned char)ptr[1], (unsigned char)ptr[2], (unsigned char)ptr[3], (unsigned char)ptr[4]); 
        }
        ptr += 2 + asz;
    }
    return 1; 
}


std::vector<std::string> 
split_string(std::string str, std::string separator)
{
    int max_splits = 0;

    std::vector <std::string> result;
    size_t pos = 0;
    int num_splits = 0;
    while (((max_splits == 0) || (num_splits < max_splits)) && ((pos = str.find(separator)) != std::string::npos))
    {
        result.push_back(str.substr(0, pos));
        str.erase(0, pos + separator.length());
        // Skip over any additional separators at the beginning of the remaining string
        while (str.find(separator) == 0)
        str.erase(0, separator.length());
        num_splits++;
    }
    result.push_back(str);
    return result;
}

void clean_buffer(char *buffer, int size)
{
    for(int i = 0; i < size; i++)
        buffer[i] = '\0';
}
