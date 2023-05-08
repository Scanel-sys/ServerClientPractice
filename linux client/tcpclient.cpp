#ifdef _WIN32 
    #define WIN32_LEAN_AND_MEAN 
    #include <windows.h> 
    #include <winsock2.h> 
    #include <ws2tcpip.h> 
    // Директива линковщику: использовать библиотеку сокетов 
    #pragma comment(lib, "ws2_32.lib") 
#else // LINUX 
    #include <sys/types.h> 
    #include <sys/socket.h> 
    #include <limits.h>
    #include <unistd.h>
    #include <netdb.h> 
    #include <errno.h> 
#endif

#include <stdio.h> 
#include <string.h>
#include <iostream>
#include <chrono>
#include <thread>

#define WEBHOST "google.com"
#define MAX_PATH 32768

int init();
void deinit();
void init_addr(struct sockaddr_in &addr, int family, const char *addres, int port);

int sock_err(const char* function, int sock);
void s_close(int sock);

unsigned int get_host_ipn(const char* name);
int send_request(int sock);
int recv_response(int sock, FILE* f);
int parse_err(int ret_code);
int parse_cmd(int argc, char *argv[], char *addres, int &port, char fl_path[256]);
int parse_cmd_to_addr(char *cmd_addr, int &i, char *addres);
int parse_cmd_to_port(char *cmd_addr, int i);
int parse_cmd_to_path(char *cmd_flname, char *fl_path);
int try_to_connect(int sock, sockaddr_in &addr);


int init() 
{ 
#ifdef _WIN32 // Для Windows следует вызвать WSAStartup перед началом использования сокетов 
    WSADATA wsa_data; 
    return (0 == WSAStartup(MAKEWORD(2, 2), &wsa_data)); 
#else 
    return 1; // Для других ОС действий не требуется 
#endif 
}

void deinit() 
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

        sent += res; printf(" %d bytes sent.\n", sent); 
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

int main(int argc, char *argv[]) 
{
    int port;
    char addres[256];
    char fl_path[MAX_PATH];

    parse_cmd(argc, argv, addres, port, fl_path);
    
    FILE *data_file = fopen(fl_path, "r");
    if(data_file == NULL)
    {
        perror("Cant open data file\n");
        fclose(data_file);
        return 0;
    }

    int client_tcp_socket; 
    struct sockaddr_in addr; 
    FILE *f;
    // Инициалиазация сетевой библиотеки 
    init();
    // Создание TCP-сокета 
    client_tcp_socket = socket(AF_INET, SOCK_STREAM, 0); 
    if (client_tcp_socket < 0) 
        return sock_err("socket", client_tcp_socket);

    // Заполнение структуры с адресом удаленного узла 
    init_addr(addr, AF_INET, addres, port);

    // Установка соединения с удаленным хостом
    try_to_connect(client_tcp_socket, addr);

    // Отправка запроса на удаленный сервер 
    send_request(client_tcp_socket);
    // Прием результата 
    f = fopen("page.html", "wb"); 
    recv_response(client_tcp_socket, f); 
    fclose(f);

    fclose(data_file);
    
    // Закрытие соединения 
    s_close(client_tcp_socket);    
    deinit();
    return 0;
}