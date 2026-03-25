#include "Client.h"


namespace seneca {

 
    UDPClient::UDPClient(){ }
    UDPClient::~UDPClient(){
        close_socket(sockfd);
    }

    void UDPClient::start(const std::string& server_ip){
        sockfd = socket(AF_INET, SOCK_DGRAM,0);
#ifdef _WIN32
        if(sockfd == INVALID_SOCKET)
#else
        if(sockfd < 0)
#endif 
            throw std::runtime_error("Socket creation failed");

        // configure server address
        memset(&server_addr, 0, sizeof(server_addr));
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(PORT);

#ifdef _WIN32
        InetPton(AF_INET, server_ip.c_str(), &server_addr.sin_addr);
#else 
        inet_pton(AF_INET, server_ip.c_str(), &server_addr.sin_addr);
#endif 
        addr_len = sizeof(from_addr);

        // send handshake
        const char* msg = "start";

        sendto(sockfd, 
            msg,
            strlen(msg),
            0,
            (sockaddr*)&server_addr, 
            sizeof(server_addr)
        );

        std::cout << "Handshake sent to server\n";

    }
    void UDPClient::receive_image(const std::string& filename){

        int client_id;
        recvfrom(sockfd, (char*)&client_id, sizeof(client_id), 0, nullptr, nullptr);

        std::cout << "Received client ID: " << client_id << std::endl;

        //std::string filename = "client_file_" + std::to_string(client_id) + ".jpg";
        std::ofstream file(filenameWithId(filename,client_id), std::ios::binary);
        if(!file)
            throw std::runtime_error("Cannot crate output file");


        Packet pkt;
        int expected_seq = 0;
        int chunk_count = 0;
        int reg_packets = 0;
        int lost_packets = 0;
        int out_of_order_packets = 0;
        while(true){
            int bytes = recvfrom(sockfd, 
                (char*)&pkt, 
                sizeof(pkt),
                0,
                (sockaddr*)&from_addr, 
                &addr_len
            );
            if(bytes <= 0)
                continue;

            if(pkt.size == 4 && strncmp(pkt.data,"DONE",4) == 0){
                std::cout << "[Client] DONE signal received. Closing file and exiting loop.\n";
                break;
            }

            if(pkt.seq < expected_seq)
            {
                if(pkt.seq % 216 == 0){
                    std::cout << "Out-of-order packet: expected "
                        << expected_seq 
                        << " received "
                        << pkt.seq << std::endl;
                }
                out_of_order_packets++;

            }
            else if(pkt.seq > expected_seq){
                if(pkt.seq % 216 == 0){
                    std::cout << "Packet loss detected. Expected "
                        << expected_seq
                        << " but received "
                        << pkt.seq << std::endl;
                    std::cout << "Lost packets: "
                        << " to "
                        << pkt.seq - 1
                        << std::endl;
                }
                lost_packets++;
                expected_seq = pkt.seq + 1;
            } else {

                file.write(pkt.data, pkt.size);
                if(!file){
                    std::cerr << "Error writing to file\n";
                    break;
                }
                reg_packets++;
            }

            expected_seq++;
            chunk_count++;

            if(pkt.size < CHUNK_SIZE)
            {
                std::cout << "Last packet received\n";
                break;
            }


        }
        file.close();
        std::cout << "Image received successfully ("
            << chunk_count 
            << " packets)\n";
        if(reg_packets>0)
            std::cout << "Regular pakets received: " << reg_packets << std::endl;
        if(lost_packets>0)
            std::cout << "Lost packets: " << lost_packets << std::endl;
        if(out_of_order_packets>0)
            std::cout << "Out-of-order packets: " << out_of_order_packets << std::endl;


    }


    std::string UDPClient::filenameWithId(const std::string& filename, int id){
        size_t dot_pos = filename.find_last_of('.');
        std::string filename_with_id; 
        if(dot_pos != std::string::npos){
            filename_with_id = filename.substr(0,dot_pos)
                + "_" + std::to_string(id)
                + filename.substr(dot_pos);
        } else {
            filename_with_id = filename + "_" + std::to_string(id);
        }
        return filename_with_id;
    }




}