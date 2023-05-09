#include "tcpclient.h"


int main(int argc, char *argv[]) 
{
    int port;
    char addres[256];
    char fl_path[MAX_PATH];

    parse_cmd(argc, argv, addres, port, fl_path);
    
    if(!(std::filesystem::exists(fl_path)))
    {
        perror("Data file seems like doesnt exist\n");
        return 0;
    }
    std::ifstream data_file;
    data_file.open(fl_path);


    int client_tcp_socket; 
    struct sockaddr_in addr; 
    FILE *f;

    init_netw_lib();
    // Создание TCP-сокета 
    client_tcp_socket = socket(AF_INET, SOCK_STREAM, 0); 
    if (client_tcp_socket < 0) 
        return sock_err("socket", client_tcp_socket);

    init_addr(addr, AF_INET, addres, port);

    try_to_connect(client_tcp_socket, addr);
    send_client_msgs(client_tcp_socket, data_file);

    // Отправка запроса на удаленный сервер 
    send_request(client_tcp_socket);
    // Прием результата 
    f = fopen("page.html", "wb"); 
    recv_response(client_tcp_socket, f); 
    fclose(f);

    data_file.close();    
    // Закрытие соединения 
    s_close(client_tcp_socket);    
    deinit_netw_lib();

    return 0;
}