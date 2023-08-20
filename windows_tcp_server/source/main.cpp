#include "tcpserver.h"


int main(int argc, char *argv[]) 
{
    init_netw_lib();
    ServerData server;
    server.port = get_port(argc, argv);
    server.socket = Socket(AF_INET, SOCK_STREAM, 0);
    server.addrlen = sizeof(server.ip);
    init_sockaddr(server.ip, AF_INET, INADDR_ANY, server.port);

    set_non_block_mode(server.socket);
    
    if(Bind(server.socket, (struct sockaddr*) &server.ip, server.addrlen) == 0 &&
        Listen(server.socket, 1) == 0)
    {
        printf("Listening TCP port : %d\n", server.port);
        std::ofstream clients_data_file;
        clients_data_file.open(get_msg_file_path());
        serveClients(server, clients_data_file);
        clients_data_file.close();
    }
    
    close_socket(server.socket);     
    deinit_netw_lib();
    return 0; 
}
