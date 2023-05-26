#include "udpclient.h"


int main(int argc, char *argv[]) 
{ 
    int port;
    char ip_addr[16];
    char fl_path[MAX_PATH];

    init_netw_lib();
    parse_cmd(argc, argv, ip_addr, port, fl_path);

    if(!(std::filesystem::exists(fl_path)))
    {
        perror("Data file seems like doesnt exist\n");
        return 0;
    }

    int client_socket = Socket(AF_INET, SOCK_DGRAM, 0);; 
    std::vector <datagram> datagrams = get_datagrams(fl_path);
    struct sockaddr_in ip_addr_netstyle; 

    init_udp_addr(ip_addr_netstyle, AF_INET, ip_addr, port);

    int msgs_amount = datagrams.size(), sent_msgs = 0;
    char response_data_buff[80];

    while(msgs_amount > 0 && sent_msgs < 20)
    {        
        for(int i = 0; i < msgs_amount; i++)
        {
            send_msg(client_socket, &ip_addr_netstyle, 
                    datagrams[i].msg, datagrams[i].msg_size);
            printf("Sending msg : %d\n", datagrams[i].msg_index);
        }
        
        if(recv_response(client_socket, response_data_buff) > 0)
        {
            printf("Some msgs didnt arrive\nResending...\n");
            
            datagrams = get_missed_msgs(datagrams, response_data_buff, SERVER_DATAGRAM_SZ);

            sent_msgs += msgs_amount - datagrams.size();
            msgs_amount = datagrams.size();
        }
    }
    close_socket(client_socket);
    deinit_netw_lib();
    return 0; 
}
