#include "tcpclient.h"


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

void get_msg(std::ifstream source, std::string destination)
{
    std::getline(source, destination);
}

void parse_msg(std::string source, std::string &date_1, std::string &date_2, std::string &time, std::string &msg)
{
    std::stringstream str_stream(source);
    char dummy;
    str_stream >> std::noskipws;

    str_stream >> date_1;
    str_stream.clear();
    str_stream >> dummy;

    str_stream >> date_2;
    str_stream.clear();
    str_stream >> dummy;
    
    str_stream >> time;
    str_stream.clear();
    str_stream >> dummy;
    
    getline(str_stream, msg);
}

void handle_msg(int sock, std::string source)
{
    std::string date_1, date_2, time, msg_text;
    parse_msg(source, date_1, date_2, time, msg_text);

    send_date(sock, date_1);
    send_date(sock, date_2);
    send_time(sock, time);
    send_msg(sock, msg_text.c_str(), msg_text.length() + 1);
}
 
void send_date(int sock, std::string source)
{
    std::vector <std::string> data = split_string(source, ".");
    char temp_byte;
    unsigned short temp_ushort;

    temp_byte = (char)std::stoi(data[0]);    
    send_msg(sock, &temp_byte, 1);
    
    temp_byte = (char)std::stoi(data[1]);    
    send_msg(sock, &temp_byte, 1);

    temp_ushort = htons((uint16_t)std::stoi(data[2]));
    send_msg(sock, &temp_ushort, 2);
}

void send_time(int sock, std::string source)
{
    std::vector <std::string> data = split_string(source, ".");
    char temp_byte;

    temp_byte = (char)std::stoi(data[0]);    
    send_msg(sock, &temp_byte, 1);

    temp_byte = (char)std::stoi(data[1]);    
    send_msg(sock, &temp_byte, 1);

    temp_byte = (char)std::stoi(data[2]);    
    send_msg(sock, &temp_byte, 1);
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

    // printf(" %d bytes sent.\n", len); 
    return 0; 
}

int send_msg_index(int sock, int msg_index)
{
    std::string buf = std::to_string(htonl(msg_index));
    send_msg(sock, buf.c_str(), 4);
}

int send_client_msgs(int sock, std::ifstream &source)
{
    signal_to_server(sock);

    int msg_index = 0;
    std::string buffer;

    while(std::getline(source, buffer))
    {
        if(buffer.length() == 0)
            continue;
        
        send_msg_index(sock, msg_index++);
        handle_msg(sock, buffer);
    }
}

void signal_to_server(int sock)
{
    send_msg(sock, "put", 3);
}

// Отправляет http-запрос на удаленный сервер 
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

int recv_response(int sock, FILE* f) 
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

int parse_err(int ret_code)
{
    return -1;
}

int parse_cmd(int argc, char *argv[], char *addres, int &port, char fl_path[MAX_PATH])
{
    if(argc != 3)
        return parse_err(-1);
 
    int i = 0;

    if(parse_cmd_to_addr(argv[1], i, addres) != 0)
        return parse_err(-1);
    
    port = parse_cmd_to_port(argv[1], i);
    if(port < 0)
        return parse_err(-1);
    
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

void init_addr(sockaddr_in &addr, int family, const char *addres, int port)
{
    memset(&addr, 0, sizeof(addr)); 
    addr.sin_family = AF_INET; 
    addr.sin_port = htons(port); 
    addr.sin_addr.s_addr = get_host_ipn(addres);
}

int try_to_connect(int sock, sockaddr_in &addr)
{
    using namespace std::this_thread;
    using namespace std::chrono;

    for(int i; i < 10; i++)
    {
        if (connect(sock, (struct sockaddr*) &addr, sizeof(addr)) == 0)
            break;
        
        else if (i == 9) 
        { 
            s_close(sock); 
            return sock_err("connect", sock); 
        }
        sleep_for(milliseconds(100));
    }
    return 0;
}

unsigned char 
stouc(std::string source)
{
    unsigned char result = 0;
    for(int i = 0; i < source.length(); i++)
    {
        result *= 10;
        result += source[i] - '0';
    }
    return result;
}

unsigned int
stouint(std::string source)
{
    unsigned int result = 0;
    for(int i = 0; i < source.length(); i++)
    {
        result *= 10;
        result += source[i] - '0';
    }
    return result;
}

unsigned short 
stous(std::string source)
{
    unsigned short result = 0;
    for(int i = 0; i < source.length(); i++)
    {
        result *= 10;
        result += source[i] - '0';
    }
    return result;
}

std::vector<std::string> 
split_string(std::string str, std::string separator, int max_splits = 0)
{
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
