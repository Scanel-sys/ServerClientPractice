#include "tcpserver.h"


int main() 
{ 
    int s; 
    struct sockaddr_in addr;

    init_netw_lib();

    // Создание TCP-сокета 
    s = socket(AF_INET, SOCK_STREAM, 0); 
    if (s < 0) 
        return sock_err("socket", s);
    
    // Заполнение адреса прослушивания 
    memset(&addr, 0, sizeof(addr)); 
    addr.sin_family = AF_INET; 
    addr.sin_port = htons(9000); 
    // Сервер прослушивает порт 9000 
    addr.sin_addr.s_addr = htonl(INADDR_ANY); 
    // Все адреса
    
    // Связывание сокета и адреса прослушивания 
    if (bind(s, (struct sockaddr*) &addr, sizeof(addr)) < 0) 
        return sock_err("bind", s);

    // Начало прослушивания 
    if (listen(s, 1) < 0) 
        return sock_err("listen", s);

    do { // Принятие очередного подключившегося клиента 
        socklen_t addrlen = sizeof(addr); 
        int cs = accept(s, (struct sockaddr*) &addr, &addrlen); 
        unsigned int ip; 
        int len;
        
        if (cs < 0) 
        { 
            sock_err("accept", s); 
            break; 
        }

        // Вывод адреса клиента 
        ip = ntohl(addr.sin_addr.s_addr); 
        printf(" Client connected: %u.%u.%u.%u ", (ip >> 24) & 0xFF, (ip >> 16) & 0xFF, (ip >> 8) & 0xFF, (ip) & 0xFF);
        
        // Прием от клиента строки 
        len = recv_string(cs);
        // Отправка клиенту сообщения о длине полученной строки 
        send_notice(cs, len);
        printf(" string len is: %d\n", len);
        
        // Отключение клиента 
        s_close(cs);

    } while (1); // Повторение этого алгоритма в беск. цикле s_close(s); deinit();
    
    deinit_netw_lib();
    return 0; 
}