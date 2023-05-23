#include "tcpserver.h"


int main(int argc, char *argv[]) 
{ 
    serverData server;
    server.port = get_port(argc, argv);
    init_netw_lib();

    server.socket = Socket(AF_INET, SOCK_STREAM, 0);
    set_non_block_mode(server.socket);
    init_sockaddr(server.ip, AF_INET, INADDR_ANY, server.port);
    if(Bind(server.socket, (struct sockaddr*) &server.ip, sizeof(server.ip)) >= 0)
    {
        Listen(server.socket, 1); 

        std::ofstream clients_data_file;
        clients_data_file.open(get_msg_file_path());

        int temp_client;
        bool letwork = true;
        std::string msg_to_write;
        int res;

        server.addrlen = sizeof(server.ip);
        serveClients(server, clients_data_file);
    
        clients_data_file.close();
    }
    
    close_socket(server.socket);     
    deinit_netw_lib();
    return 0; 
}