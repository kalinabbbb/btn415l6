#ifndef SENECA_SERVER_H
#define SENECA_SERVER_H

#include <vector>
#include <thread>
#include <fstream>
#include <map>
#include <atomic>
#include "../utilities/SocketUtils.h"

#define PORT 8081               // Port to send data
#define CHUNK_SIZE 1024         // Size of each chunk in bytes

namespace seneca {

    struct Packet{
        uint32_t seq;
        uint32_t size;
        char data[CHUNK_SIZE];
    };


    class UDPServer{
        socket_t sockfd;
        sockaddr_in server_addr{};

        std::map<std::string, int> client_ids; // stores IP:port -> client ID
        std::atomic<int> next_client_id{1};
        void send_done(sockaddr_in client_addr, socklen_t client_len);
    public:
        UDPServer();
        ~UDPServer();
        void start(int port);
        void send_image(std::string filename, 
        sockaddr_in client_addr, 
        socklen_t client_len);

        // Helper to generate key for map
        std::string get_client_key(const sockaddr_in& addr){
            return std::string (inet_ntoa(addr.sin_addr)) + ":" + std::to_string(ntohs(addr.sin_port));
        }
    };

}


#endif 