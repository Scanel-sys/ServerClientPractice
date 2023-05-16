#include "tcpclient.h"


struct parsed_time
{
    char hour;
    char min;
    char sec;
};

struct parsed_date
{
    char day;
    char month;
    unsigned short year;
};

struct parsed_message
{
    struct parsed_date date1;
    struct parsed_date date2;
    struct parsed_time time;
    std::string msg_text;
};


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

void init_addr(sockaddr_in &addr, int family, const char *addres, int port)
{
    memset(&addr, 0, sizeof(addr)); 
    addr.sin_family = AF_INET; 
    addr.sin_port = htons(port); 
    addr.sin_addr.s_addr = get_host_ipn(addres);
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

void close_sock(int sock) 
{ 
#ifdef _WIN32 
    closesocket(sock); 
#else 
    close(sock); 
#endif 
}

int shutdown_server(int sock)
{
    return send_msg(sock, "stop", 4);
}

// Функция определяет IP-адрес узла по его имени. 
// Адрес возвращается в сетевом порядке байтов. 
unsigned int get_host_ipn(const char* name) 
{ 
    struct addrinfo* addr = 0; 
    unsigned int ip4addr = 0;
    // Функция возвращает все адреса указанного хоста 
    // в виде динамического однонаправленного списка 
    if (0 == getaddrinfo(name, 0, 0, &addr)) 
    { 
        struct addrinfo* cur = addr; 
        while (cur) 
        { 
            // Интересует только IPv4 адрес, если их несколько - то первый 
            if (cur->ai_family == AF_INET) 
            { 
                ip4addr = ((struct sockaddr_in*) cur->ai_addr)->sin_addr.s_addr; 
                break; 
            } 
            cur = cur->ai_next; 
        } 
        freeaddrinfo(addr); 
    }
    return ip4addr; 
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

int parse_cmd(int argc, char *argv[], char *addres, int &port, char fl_path[MAX_PATH])
{
    if(argc != 3)
        return parse_err("Wrong number of parameters passed");
 
    int i = 0;

    if(parse_cmd_to_addr(argv[1], i, addres) != 0)
        return parse_err("Bad format of addres (parse_cmd_to_addr)");
    
    port = parse_cmd_to_port(argv[1], i);
    if(port < 0)
        return parse_err("Wrong port is passed");
    
    parse_cmd_to_path(argv[2], fl_path);

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

int parse_cmd_to_path(char *cmd_flname, char fl_path[MAX_PATH])
{
    if (getcwd(fl_path, MAX_PATH) == NULL)
    {
        perror("getcwd() error");
        return 1;
    }
    int i = strlen(fl_path);
    fl_path[i++] = '/';
    strncpy(fl_path + i, cmd_flname, strlen(cmd_flname) + 1);

    return 0;
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


void handle_msg(int sock, std::string source)
{
    parsed_message msg_to_sent = parse_msg(source);

    send_date(sock, msg_to_sent.date1);
    send_date(sock, msg_to_sent.date2);
    send_time(sock, msg_to_sent.time);
    send_msg(sock, msg_to_sent.msg_text.c_str(), msg_to_sent.msg_text.length() + 1);
}
 
void send_date(int sock, parsed_date &date_to_sent)
{
    send_msg(sock, &date_to_sent.day, 1);
    send_msg(sock, &date_to_sent.month, 1);
    send_msg(sock, &date_to_sent.year, 2);
}

void send_time(int sock, parsed_time &time_to_sent)
{
    send_msg(sock, &time_to_sent.hour, 1);
    send_msg(sock, &time_to_sent.min, 1);
    send_msg(sock, &time_to_sent.sec, 1);
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
    if (res < 0) 
        return sock_err("send", sock);

    printf(" %d bytes sent.\n", len); 
    return 0; 
}


int send_msg_index(int sock, int msg_index)
{
    int to_send = htonl(msg_index);
    return send_msg(sock, &to_send, 4);
}

int send_client_msgs(int sock, std::ifstream &source)
{
    init_talk_with_server(sock);

    int msg_index = 0;
    std::string buffer;

    while(std::getline(source, buffer))
    {
        if(buffer.length() == 0)
            continue;
        
        send_msg_index(sock, msg_index++);
        handle_msg(sock, buffer);
    }
    return msg_index;
}


int Socket(int domain, int type, int protocol)
{
	int sock = socket(domain, type, protocol);
	if (sock == -1)
		return sock_err("socket", sock);

	return sock;
} 

int try_to_connect(int sock, sockaddr_in &addr)
{
    using namespace std::this_thread;
    using namespace std::chrono;

    for(int i = 0; i < 10; i++)
    {
        if (connect(sock, (struct sockaddr*) &addr, sizeof(addr)) == 0)
            break;
        
        else if (i == 9) 
        { 
            close_sock(sock); 
            return sock_err("connect", sock); 
        }
        sleep_for(milliseconds(100));
    }
    return 0;
}

void init_talk_with_server(int sock)
{
    send_msg(sock, "put", 3);
}

int send_request(int sock) 
{ 
    const char* request = "GET / HTTP/1.0\r\nServer: " WEBHOST "\r\n\r\n"; 
    int size = strlen(request); 
    int sent = 0;

#ifdef _WIN32 
    int flags = 0; 
#else 
    int flags = MSG_NOSIGNAL; 
#endif

    while (sent < size) 
    { 
        // Отправка очередного блока данных 
        int res = send(sock, request + sent, size - sent, flags); 
        if (res < 0) 
            return sock_err("send", sock);

        sent += res; 
        printf(" %d bytes sent.\n", sent); 
    }
    return 0; 
}

int recv_response_to_file(int sock, FILE* f) 
{   
    char buffer[256]; int res;
    // Принятие очередного блока данных. 
    // Если соединение будет разорвано удаленным узлом recv вернет 0 
    while ((res = recv(sock, buffer, sizeof(buffer), 0)) > 0) 
    { 
        fwrite(buffer, 1, res, f); 
        printf(" %d bytes received\n", res); 
    }

    if (res < 0) 
        return sock_err("recv", sock);
    return 0; 
}

int recv_response_once(int sock, char *buffer, int len) 
{   
    int res;
    if((res = recv(sock, buffer, len, 0)) > 0)  
        printf(" %d bytes received\n", res); 
    
    if (res < 0) 
        return sock_err("recv_resp", sock);
    return 0; 
}

bool recv_response_ok(int sock) 
{   
    char buffer[2];
    recv_response_once(sock, buffer, 2);
    if(buffer[0] == 'o' &&
       buffer[1] == 'k')
       return true;
    return false;
}

int recvn_response_ok(int sock, int msgs_number)
{
    int i = 0;
    while(i < msgs_number && recv_response_ok(sock))
        i++;
    
    if(i != msgs_number)
        return sock_err("ok recvier", sock);
    return 0;
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
