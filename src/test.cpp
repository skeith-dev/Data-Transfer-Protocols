//
// Created by Spencer Keith on 11/15/22.
//

#include <string>
#include <netinet/in.h>
#include <cstdio>
#include <cstdlib>
#include <arpa/inet.h>
#include "packet_file_io.h"


int main() {

    //set up UDP socket
    struct sockaddr_in serverAddress = {0};
    int clientSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if(clientSocket == -1) {
        perror("Failed to create client socket");
        exit(EXIT_FAILURE);
    }
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = inet_addr(std::string("127.0.0.1").c_str());
    serverAddress.sin_port = htons(5000);

    sendPacket(clientSocket, serverAddress, std::string("/Users/spencerkeith/Desktop/School/Spring 2022/CS 462/Data-Transfer-Protocols/Read-Write/Crash Team Racing: Nitro Fueled Dev Time Rankings.txt"), 0, 10, 1511, 152);
    sendPacket(clientSocket, serverAddress, std::string("/Users/spencerkeith/Desktop/School/Spring 2022/CS 462/Data-Transfer-Protocols/Read-Write/Crash Team Racing: Nitro Fueled Dev Time Rankings.txt"), 1, 10, 1511, 152);
    sendPacket(clientSocket, serverAddress, std::string("/Users/spencerkeith/Desktop/School/Spring 2022/CS 462/Data-Transfer-Protocols/Read-Write/Crash Team Racing: Nitro Fueled Dev Time Rankings.txt"), 2, 10, 1511, 152);

    return 0;

}