#ifndef SENECA_CLIENT_H
#define SENECA_CLIENT_H

#include <cstdint>
#include <string>
#include <fstream>
#include <iostream>
#include <cstring>

#include "../utilities/SocketSystem.h"
#include "../utilities/SocketUtils.h"

#define PORT 8081
#define CHUNK_SIZE 1024

namespace seneca {

    struct Packet {
        uint32_t seq;
        uint32_t size;
        char data[CHUNK_SIZE];
    };

    class UDPClient {
        socket_t sockfd{};
        sockaddr_in server_addr{};
        sockaddr_in from_addr{};
        socklen_t addr_len{};
    public:
        UDPClient();
        ~UDPClient();
        std::string filenameWithId(const std::string& filename, int id);

        void start(const std::string& server_ip);
        void receive_image(const std::string& filename);
    };

}


#endif 