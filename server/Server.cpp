
#include "Server.h"
#include <cstdlib>
#include <ctime>
#include <chrono>

namespace seneca{

    void UDPServer::send_done(sockaddr_in client_addr, socklen_t client_len) {

        Packet pkt{0,4,"DONE"};

        sendto(sockfd,
            (char*)&pkt,
            sizeof(pkt),
            0,
            (sockaddr*)&client_addr,
            client_len);

        std::cout << "[send_image] Sent DONE signal to client "
                << inet_ntoa(client_addr.sin_addr)
                << ":"
                << ntohs(client_addr.sin_port)
                << " so it can close gracefully"
                << std::endl;
    }

    UDPServer::UDPServer():sockfd{INVALID_SOC}{
        std::srand(static_cast<unsigned int>(std::time(nullptr)));
    }
    UDPServer::~UDPServer(){}

    void UDPServer::start(int port){
        sockfd = socket(AF_INET, SOCK_DGRAM, 0);

        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(port);
        server_addr.sin_addr.s_addr = INADDR_ANY;

        bind(sockfd, (sockaddr*)&server_addr, sizeof(server_addr));

        std::cout << "UDP Server listening on port " << port << std::endl;

        while(true){
            sockaddr_in client_addr{};
            socklen_t client_len = sizeof(client_addr);
            char buffer[32];

            int bytes = recvfrom(sockfd, buffer, sizeof(buffer), 0, 
                (sockaddr*)&client_addr, &client_len);

            if(bytes <= 0) continue;

            std::string key = get_client_key(client_addr);
            int client_id;

            if(client_ids.find(key) == client_ids.end()){
                client_id = next_client_id++;
                client_ids[key] = client_id;
            } else {
                client_id = client_ids[key];
            }

            // TODO-1: Send the client ID back to client [Step 1]
            sendto(sockfd,
                (char*)&client_id,
                sizeof(client_id),
                0,
                (sockaddr*)&client_addr,
                client_len);

            std::cout << "Would call sendto() to send client_id ("
                    << client_id << ") to client "
                    << inet_ntoa(client_addr.sin_addr)
                    << ":" << ntohs(client_addr.sin_port)
                    << std::endl;

            std::cout << "Client request received from "
                    << inet_ntoa(client_addr.sin_addr)
                    << ":" << ntohs(client_addr.sin_port)
                    << std::endl;

            std::cout << "Client key: "
                    << key
                    << " assigned ID: "
                    << client_id
                    << std::endl;

            // TODO-2: Implement thread spawning for the client [Step 2]
            std::cout << "Spawning thread to send image to client..." << std::endl;

            std::thread(&UDPServer::send_image,
                this,
                "../assets/images/network-image.jpg",
                client_addr,
                client_len).detach();

            std::cout << "Thread would execute send_image() with parameters:"
                    << std::endl;

            std::cout << "           filename = ../assets/images/network-image.jpg" << std::endl;

            std::cout << "           client = "
                    << inet_ntoa(client_addr.sin_addr)
                    << ":" << ntohs(client_addr.sin_port)
                    << std::endl;            
        }
    }

    void UDPServer::send_image(std::string filename, 
        sockaddr_in client_addr, 
        socklen_t client_len
    ){
        // TODO-1: read file in chunks and send packets [Step 2]
        std::ifstream file(filename, std::ios::binary);
        if(!file){
            std::cerr << "Cannot open file: " << filename << std::endl;
            return;
        }

        std::cout << "[send_image] Initializing sequence number (seq = 0)" << std::endl;

        int seq = 0;

        std::cout << "[send_image] Entering loop to read file in chunks of "
                << CHUNK_SIZE << " bytes" << std::endl;

        // TODO-2
        while(true){
            Packet pkt{};
            file.read(pkt.data, CHUNK_SIZE);
            pkt.size = static_cast<uint32_t>(file.gcount());

            if(pkt.size == 0)
                break;

            pkt.seq = seq++;

            // TODO-3
            bool drop_packet = (rand() % 10 == 0
                             || rand() % 20 == 0
                             || rand() % 30 == 0
                             || rand() % 40 == 0);

            if(drop_packet){
                if(pkt.seq % 216 == 0)
                    std::cout << "[Server] Simulating packet loss: seq="
                              << pkt.seq << std::endl;
                continue;
            }

            int delay_ms = rand() % 5;
            if(delay_ms > 0){
                std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms));
                if(pkt.seq % 216 == 0)
                    std::cout << "[Server] Simulating delay of "
                              << delay_ms
                              << "ms for packet seq="
                              << pkt.seq << std::endl;
            }

            // TODO-4
            sendto(sockfd,
                (char*)&pkt,
                sizeof(pkt),
                0,
                (sockaddr*)&client_addr,
                client_len);

            if(pkt.size < CHUNK_SIZE)
                break;

            if(pkt.seq % 200 == 0){
                std::cout << "[send_image] Sent chunk seq=" << pkt.seq
                          << ", size=" << pkt.size << std::endl;
            }
        }

        // TODO-5
        std::cout << "[send_image] Finished sending image to "
                << inet_ntoa(client_addr.sin_addr)
                << ":"
                << ntohs(client_addr.sin_port)
                << std::endl;

        send_done(client_addr, client_len);
    }

}
