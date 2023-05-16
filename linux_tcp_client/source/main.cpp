#include "tcpclient.h"


int main(int argc, char *argv[]) 
{
    int port;
    char ip_addr[16];
    char fl_path[MAX_PATH];

    parse_cmd(argc, argv, ip_addr, port, fl_path);
    
    if(!(std::filesystem::exists(fl_path)))
    {
        perror("Data file seems like doesnt exist\n");
        return 0;
    }

    int client_tcp_socket; 
    int sent_msgs;
    struct sockaddr_in ip_addr_internet_style; 
    std::ifstream data_file;

    init_netw_lib();

    client_tcp_socket = Socket(AF_INET, SOCK_STREAM, 0); 
    init_addr(ip_addr_internet_style, AF_INET, ip_addr, port);

    try_to_connect(client_tcp_socket, ip_addr_internet_style);
    init_talk_with_server(client_tcp_socket);
    
    data_file.open(fl_path);
    sent_msgs = send_client_msgs(client_tcp_socket, data_file);
    data_file.close();    
    
    recvn_response_ok(client_tcp_socket, sent_msgs);

    close_sock(client_tcp_socket);    
    deinit_netw_lib();

    return 0;
}