#include "Client.h"

using namespace seneca;
using namespace std;

int main(){
    SocketSystem ss;

    try {
        UDPClient client;
        std::string server_ip = "127.0.0.1";
        client.start(server_ip);
        std::string filename = "received-network-image.jpg";
        client.receive_image(filename.c_str());
    } catch(std::exception& e){
        std::cerr << "Exception: " << e.what() << std::endl;
    }

    return 0;
}