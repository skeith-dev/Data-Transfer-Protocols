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


//*****//*****//*****//*****//*****//*****//*****//*****//*****//*****//
//Fields      //*****//*****//*****//*****//*****//*****//*****//*****//

std::string ipAddress; //IP address of the target server
int portNum; //port number of the target server
int protocolType; //true for GBN, false for SR
int packetSize; //specified size of packets to be sent
int timeoutInterval; //user-specified (0+) or ping calculated (-1)
int slidingWindowSize;  //ex. [1, 2, 3, 4, 5, 6, 7, 8], size = 8
int rangeOfSequenceNumbers; //ex. (sliding window size = 3) [1, 2, 3] -> [2, 3, 4] -> [3, 4, 5], range = 5
std::string filePath;
bool quit; //true for yes, false for no

int iterator; //iterator for network protocols

//*****//*****//*****//*****//*****//*****//*****//*****//*****//*****//
//Function declarations            //*****//*****//*****//*****//*****//

std::string ipAddressPrompt();

int portNumPrompt();

int protocolTypePrompt();

int packetSizePrompt();

int rangeOfSequenceNumbersPrompt();

int slidingWindowSizePrompt();

std::string filePathPrompt();

bool quitPrompt();

void executeSAWProtocol(int serverSocket, sockaddr_in clientAddress);

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

        if(protocolType != 0) {
            slidingWindowSize = slidingWindowSizePrompt();
        }

        rangeOfSequenceNumbers = rangeOfSequenceNumbersPrompt();
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
                executeSAWProtocol(serverSocket, clientAddress);
                break;
            case 1:
                std::cout << std::endl << "Executing Go Back N protocol..." << std::endl << std::endl;
                //executeGBNProtocol();
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

void sendAck(int serverSocket, sockaddr_in clientAddress) {

    Packet myAck{};
    myAck.sequenceNumber = iterator;
    myAck.valid = true;

    sendto(serverSocket, &myAck, sizeof(myAck), 0, (const struct sockaddr *) &clientAddress, sizeof(clientAddress));
    std::cout << "Sent Ack #" << myAck.sequenceNumber << std::endl;

}

//*****//*****//*****//*****//*****//*****//*****//*****//*****//*****//
//Network protocols (algorithms)

void executeSAWProtocol(int serverSocket, sockaddr_in clientAddress) {

    int clientSize = sizeof(clientAddress);

    iterator = 0;
    while(iterator < rangeOfSequenceNumbers) {

        Packet myPacket{};

        while(true) {

            long ret = recvfrom(serverSocket, &myPacket, sizeof(myPacket), 0, (struct sockaddr*)&clientAddress, reinterpret_cast<socklen_t *>(&clientSize));

            if(ret == 0) {
                break;
            } else if(ret < 0) {
                perror("Error when receiving packet");
                exit(-1);
            }

            if(myPacket.sequenceNumber == iterator && myPacket.valid) {
                std::cout << "Received packet #" << myPacket.sequenceNumber << " successfully! [ ";
                for(int i = 0; i < packetSize; i++) {
                    std::cout << myPacket.contents[i];
                }
                std::cout << " ]" << std::endl;

                iterator++;
                sendAck(serverSocket, clientAddress);
                break;
            } else {
                std::cout << "Received packet #" << myPacket.sequenceNumber << "... valid = " << myPacket.valid << std::endl;
                sendAck(serverSocket, clientAddress);
            }

        }
    }

}
