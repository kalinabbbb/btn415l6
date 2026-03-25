#include "../utilities/SocketSystem.h"
#include "Server.h"

using namespace seneca;
using namespace std;

int main()
{
    SocketSystem ss;

    try{
        UDPServer server; 
        server.start(8081);
    } catch (std::exception& e){
        std::cerr << "Exception: " << e.what() << std::endl;
    }

    return 0;
}