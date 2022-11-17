//
// Created by Spencer Keith on 11/17/22.
//

#ifndef DATA_TRANSFER_PROTOCOLS_SOCKET_IO_H
#define DATA_TRANSFER_PROTOCOLS_SOCKET_IO_H

#include <iostream>
#include <netinet/in.h>
#include <arpa/inet.h>


/**
 * Prints the contents of a char array packet
 * @param packet The packets whose contents are being printed
 * @param packetSize The size of the packets contents (not including seqNum and valid bytes)
 */
void printPacket(char packet[], int packetSize) {

    int length = (int) (sizeof(int) + sizeof(bool) + packetSize);
    std::cout << "[ ";
    for(int i = 0; i < length; i++) {
        std::cout << packet[i];
    }
    std::cout << " ]" << std::endl;

}

/**
 * Sends char array packet to destination server
 * @param clientSocket The client socket, which sends the packet to the server
 * @param serverAddress The address of the server, which receives the packet from the client socket
 * @param packet The char array being sent from client to server; contains seqNum, valid, and contents
 * @param seqNum The sequence number of the packet being sent to the server
 * @param packetSize The size of the packet CONTENTS; does NOT include seqNum and valid bytes
 */
void sendPacket(int clientSocket, sockaddr_in serverAddress, char packet[], int seqNum, int packetSize) {

    int length = (int) (sizeof(int) + sizeof(bool) + packetSize);
    std::cout << length << std::endl;
    sendto(clientSocket, packet, length, 0, (const struct sockaddr *) &serverAddress, sizeof(serverAddress));
    std::cout << errno;

    std::cout << "Sent Packet #" << seqNum << ": ";
    printPacket(packet, packetSize);

}

#endif //DATA_TRANSFER_PROTOCOLS_SOCKET_IO_H
