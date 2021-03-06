//
// Created by Spencer Keith on 4/18/22.
//

#include <iostream>
#include <string>
#include <fstream>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "packet.h"

#define FINAL_SEQUENCE_NUMBER -1


//*****//*****//*****//*****//*****//*****//*****//*****//*****//*****//
//Fields      //*****//*****//*****//*****//*****//*****//*****//*****//

std::string ipAddress; //IP address of the target server
int portNum; //port number of the target server
int protocolType; //true for GBN, false for SR
int packetSize; //specified size of packets to be sent
int timeoutInterval; //user-specified (0+) or ping calculated (-1)
int slidingWindowSize;  //ex. [1, 2, 3, 4, 5, 6, 7, 8], size = 8
std::string filePath;
bool quit; //true for yes, false for no

int iterator; //iterator for network protocols

//*****//*****//*****//*****//*****//*****//*****//*****//*****//*****//
//Function declarations            //*****//*****//*****//*****//*****//

std::string ipAddressPrompt();

int portNumPrompt();

int protocolTypePrompt();

int packetSizePrompt();

int slidingWindowSizePrompt();

std::string filePathPrompt();

bool quitPrompt();

void executeSAW_GBNProtocol(int serverSocket, sockaddr_in clientAddress);

//*****//*****//*****//*****//*****//*****//*****//*****//*****//*****//
//Functions (including main)       //*****//*****//*****//*****//*****//

int main() {

    //set up UDP socket
    struct sockaddr_in serverAddress{}, clientAddress{};
    memset(&serverAddress, 0, sizeof(serverAddress));
    memset(&clientAddress, 0, sizeof(clientAddress));
    int serverSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if(serverSocket == -1) {
        perror("Failed to create server socket:");
        exit(EXIT_FAILURE);
    }

    do {

        serverAddress.sin_family = AF_INET;

        //prompt user for each of the following fields
        ipAddress = ipAddressPrompt();
        serverAddress.sin_addr.s_addr = inet_addr(ipAddress.c_str());

        portNum = portNumPrompt();
        serverAddress.sin_port = htons(portNum);

        protocolType = protocolTypePrompt();
        packetSize = packetSizePrompt();

        if(protocolType == 2) {
            slidingWindowSize = slidingWindowSizePrompt();
        }

        filePath = filePathPrompt();

        int socketBinding = bind(serverSocket, (const struct sockaddr *)&serverAddress, sizeof(serverAddress));
        if(socketBinding == -1) {
            perror("Failed to bind server socket");
            close(serverSocket);
            exit(EXIT_FAILURE);
        }

        switch (protocolType) {
            case 0:
                std::cout << std::endl << "Executing Stop & Wait protocol..." << std::endl << std::endl;
                executeSAW_GBNProtocol(serverSocket, clientAddress);
                break;
            case 1:
                std::cout << std::endl << "Executing Go Back N protocol..." << std::endl << std::endl;
                executeSAW_GBNProtocol(serverSocket, clientAddress);
                break;
            case 2:
                std::cout << std::endl << "Executing Selective Repeat protocol..." << std::endl << std::endl;
                //executeSRProtocol();
                break;
            default:
                break;
        }

        close(serverSocket);

        quit = quitPrompt();

    } while(!quit);

    return 0;
}

//*****//*****//*****//*****//*****//*****//*****//*****//*****//*****//
//Prompts

std::string ipAddressPrompt() {

    std::cout << "What is the IP address of the target server:" << std::endl;

    std::string responseString;
    std::getline(std::cin, responseString);

    return responseString;

}

int portNumPrompt() {

    std::cout << "What is the port number of the target server:" << std::endl;

    std::string responseString;
    std::getline(std::cin, responseString);

    return std::stoi(responseString);

}

int protocolTypePrompt() {

    std::cout << "Type of protocol, S&W (0), GBN (1) or SR (2):" << std::endl;

    std::string responseString;
    std::getline(std::cin, responseString);

    return std::stoi(responseString);

}

int packetSizePrompt() {

    std::cout << "Size of packets:" << std::endl;

    std::string responseString;
    std::getline(std::cin, responseString);

    return std::stoi(responseString);

}

int slidingWindowSizePrompt() {

    std::cout << "Size of sliding window:" << std::endl;

    std::string responseString;
    std::getline(std::cin, responseString);

    return std::stoi(responseString);

}

int rangeOfSequenceNumbersPrompt() {

    std::cout << "Range of sequence numbers:" << std::endl;

    std::string responseString;
    std::getline(std::cin, responseString);

    return std::stoi(responseString);

}

std::string filePathPrompt() {

    std::cout << "What is the filepath of the file you wish to write to:" << std::endl;

    std::string responseString;
    std::getline(std::cin, responseString);

    return responseString;

}

bool quitPrompt() {

    std::cout << std::endl << "Would you like to exit (1), or perform another file transfer (0):" << std::endl;

    std::string responseString;
    std::getline(std::cin, responseString);

    return std::stoi(responseString) == 1;

}

//*****//*****//*****//*****//*****//*****//*****//*****//*****//*****//
//UDP NETWORKING FUNCTIONS, FILE READING/WRITING FUNCTIONS

void printWindow() {

    std::cout << "Current window = [ ";
    for (int i = iterator; i < iterator + slidingWindowSize; i++) {
        std::cout << i << " ";
    }
    std::cout << "]" << std::endl;

}

void sendAck(int serverSocket, sockaddr_in clientAddress, int sequenceNumber) {

    Packet myAck{};
    myAck.sequenceNumber = sequenceNumber;
    myAck.valid = true;

    sendto(serverSocket, &myAck, sizeof(myAck), 0, (const struct sockaddr *) &clientAddress, sizeof(clientAddress));
    std::cout << "Sent Ack #" << myAck.sequenceNumber << std::endl;

}

void writePacketToFile(bool append, const std::string& message) {

    std::ofstream fileOutputStream;
    if(append) {
        fileOutputStream.open(filePath, std::ios_base::app);
    } else {
        fileOutputStream.open(filePath);
    }
    fileOutputStream << message;

    fileOutputStream.close();

}

//*****//*****//*****//*****//*****//*****//*****//*****//*****//*****//
//Network protocols (algorithms)

void executeSAW_GBNProtocol(int serverSocket, sockaddr_in clientAddress) {

    int clientSize = sizeof(clientAddress);

    iterator = 0;
    while(true) {

        Packet myPacket{};

        if(recvfrom(serverSocket, &myPacket, sizeof(myPacket), MSG_DONTWAIT, (struct sockaddr*)&clientAddress, reinterpret_cast<socklen_t *>(&clientSize)) != -1) {

            if(myPacket.sequenceNumber == FINAL_SEQUENCE_NUMBER) {
                break;
            }

            std::cout << "Received packet #" << myPacket.sequenceNumber << "! [ ";
            for(int i = 0; i < packetSize; i++) {
                std::cout << myPacket.contents[i];
            }
            std::cout << " ]" << std::endl;

            if(myPacket.valid && myPacket.sequenceNumber == iterator) {
                sendAck(serverSocket, clientAddress, iterator);
                writePacketToFile(true, myPacket.contents);
                iterator++;
            } else {
                std::cout << "Received packet is corrupted!" << std::endl;
            }

        }

    }

}
