#include "udpserver.h"


int main(int argc, char *argv[]) 
{
    init_netw_lib();
    ServerData server = init_server(argc, argv);
    set_non_block_mode_to_server(server);
    bind_ports(server);

    std::ofstream input_msgs_file;
    input_msgs_file.open(get_msg_file_path());

    launch_server_with_epoll(server, input_msgs_file);

    input_msgs_file.close();
    close_server_sockets(server);
    deinit_netw_lib();
    return 0;
}
