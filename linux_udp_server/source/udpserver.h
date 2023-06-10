
#include <sys/poll.h>
#include <sys/types.h> 
#include <sys/socket.h> 
#include <sys/time.h> 
#include <sys/epoll.h> 
#include <netdb.h> 
#include <errno.h> 
#include <fcntl.h>      //lib for non blocking behaviour

#include <unistd.h>
#include <stdio.h> 
#include <string.h>
#include <fstream>
#include <unistd.h>
#include <vector>
#include <iostream>
#include <limits.h>


#define  MAXEVENTS_PER_CALL         (20)
#define  MAX_CLIENTS               (256)
#define  NEW_CLIENT                 (-1)
#define  SERVICE_INFO_SIZE          (15)
#define  RAW_MSG_SIZE             (1024)

#define  FIRST_DAY_SHIFT             (4)
#define  FIRST_MONTH_SHIFT           (5)
#define  FIRST_YEAR_SHIFT            (6)
#define  SECOND_DAY_SHIFT            (8)
#define  SECOND_MONTH_SHIFT          (9)
#define  SECOND_YEAR_SHIFT          (10)
#define  HOURS_SHIFT                (12)
#define  MINUTES_SHIFT              (13)
#define  SECONDS_SHIFT              (14)

enum MESSAGE_TYPE{ERROR = -1, STOP, MSG};


struct PortData
{
    int socket;
    int port;
    sockaddr_in ip;
    struct epoll_event event;
};

struct ClientData{
    uint32_t ip;
    int port;
    std::vector <uint32_t> recv_msg_numb;
};

struct ServerData{
    std::vector <PortData> ports;
    std::vector <ClientData> client_database;
    socklen_t addrlen;
};

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

int init_netw_lib();
void deinit_netw_lib();
struct ServerData init_server(int argc, char *argv[]);
int set_non_block_mode(int sock);
std::vector <PortData> get_and_init_ports(int argc, char *argv[]);
void set_non_block_mode_to_server(struct ServerData &server);
void close_server_sockets(struct ServerData server);

int sock_err(const char* function, int sock);
void close_socket(int sock);
int Socket(int domain, int type, int protocol);
int Bind(int sock, const sockaddr *addr, socklen_t addrlen);
void bind_ports(struct ServerData &server);
void init_sockaddr(sockaddr_in &addr, int family, u_int32_t addres, int port);

void add_ports_to_epoll_queue(struct ServerData &server, int epoll_queue);
int create_epoll_queue();
void launch_server_with_epoll(struct ServerData &server, std::ofstream &input_msgs_file);

int get_client_index(struct ServerData &server, struct ClientData &client);
struct ClientData init_temp_client(struct PortData &portData);

uint32_t get_msg_index(char *buffer);
bool if_old_msg(struct ClientData &client, uint32_t msg_numb);

uint32_t ip_to_str(sockaddr_in sockaddr);
std::string ip_to_str(uint32_t ip);
std::string port_to_str(int port);
int char_to_int(char *buffer);

std::string gen_msg_metadata(struct PortData &msg_dealer);
std::string get_parsed_datetime(char raw_msg[RAW_MSG_SIZE]);
std::string get_parsed_msg_text(char raw_msg[RAW_MSG_SIZE]);
std::string date_time_to_str(struct parsed_message &new_msg);
int assemble_client_msg(struct PortData &msg_dealer, char raw_msg[RAW_MSG_SIZE], char msg_buffer[RAW_MSG_SIZE]);
void write_msg_to_file(std::ofstream &file, char source[RAW_MSG_SIZE]);
uint32_t get_msg_number_netformat(char *raw_msg);
void recv_msg(struct ServerData &server, int port_index, char *place_for_msg, size_t buff_len);
int send_msg_number(PortData &port_data, const void *number);
int send_msg(PortData &port_data, const void *buffer, size_t buff_len);

std::string get_msg_file_path();
