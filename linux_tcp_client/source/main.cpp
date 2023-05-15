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

    std::cout << addres << ' ' << port << '\n';

    int client_tcp_socket; 
    int sent_msgs;
    struct sockaddr_in addr; 
    std::ifstream data_file;

    init_netw_lib();

    client_tcp_socket = socket(AF_INET, SOCK_STREAM, 0); 
    if (client_tcp_socket < 0) 
        return sock_err("socket", client_tcp_socket);

    init_addr(addr, AF_INET, addres, port);

    try_to_connect(client_tcp_socket, addr);
    data_file.open(fl_path);
    sent_msgs = send_client_msgs(client_tcp_socket, data_file);
    
    // send_msg(client_tcp_socket, buff, 4);
    recvn_response_ok(client_tcp_socket, sent_msgs);
    
    data_file.close();    
    shutdown_server(client_tcp_socket);
    s_close(client_tcp_socket);    
    deinit_netw_lib();

    return 0;
}