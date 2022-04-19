//
// Created by Spencer Keith on 4/18/22.
//

#include <iostream>
#include <string>
#include <fstream>
#include <netinet/in.h>
#include <unistd.h>
#include "packet.h"


//*****//*****//*****//*****//*****//*****//*****//*****//*****//*****//
//Fields      //*****//*****//*****//*****//*****//*****//*****//*****//

Packet myPacket;

std::string ipAddress; //IP address of the target server
int portNum; //port number of the target server
int protocolType; //true for GBN, false for SR
int packetSize; //specified size of packets to be sent
int timeoutInterval; //user-specified (0+) or ping calculated (-1)
int slidingWindowSize;  //ex. [1, 2, 3, 4, 5, 6, 7, 8], size = 8
int rangeOfSequenceNumbers; //ex. (sliding window size = 3) [1, 2, 3] -> [2, 3, 4] -> [3, 4, 5], range = 5
std::string filePath;
bool quit; //true for yes, false for no

int sequenceNumber;
int iterator; //iterator for network protocols
bool *receivedPtr; //ptr to bool array; true for packet received, false for packet not (yet) received

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

void executeSAWProtocol(int serverSocket);

//*****//*****//*****//*****//*****//*****//*****//*****//*****//*****//
//Functions (including main)       //*****//*****//*****//*****//*****//

int main() {

    //set up UDP socket
    struct sockaddr_in serverAddress, clientAddress;
    memset(&serverAddress, 0, sizeof(serverAddress));
    memset(&clientAddress, 0, sizeof(clientAddress));
    int serverSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if(serverSocket == -1) {
        perror("Failed to create server socket!");
        exit(EXIT_FAILURE);
    }

    do {

        serverAddress.sin_family = AF_INET;

        //prompt user for each of the following fields
        ipAddress = ipAddressPrompt();
        serverAddress.sin_addr.s_addr = INADDR_ANY; //inet_addr(ipAddress.c_str());

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
            perror("Failed to bind server socket!");
            close(serverSocket);
            exit(EXIT_FAILURE);
        }

        switch (protocolType) {
            case 0:
                std::cout << "Executing Stop & Wait protocol..." << std::endl;
                executeSAWProtocol(serverSocket);
                break;
            case 1:
                std::cout << "Executing Go Back N protocol..." << std::endl;
                //executeGBNProtocol();
                break;
            case 2:
                std::cout << "Executing Selective Repeat protocol..." << std::endl;
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

void sendAck(int serverSocket) {

    write(serverSocket, &myPacket, packetSize);
    std::cout << "Sent Ack #" << iterator << std::endl;

}

//*****//*****//*****//*****//*****//*****//*****//*****//*****//*****//
//Network protocols (algorithms)

void executeSAWProtocol(int serverSocket) {

    iterator = 0;
    for(;;) {

        Packet ack;

        while(true) {

            long ret = read(serverSocket, &ack, sizeof(ack));

            if(ret == 0) {
                break;
            } else if(ret < 0) {
                perror("Error when receiving packet!");
                exit(-1);
            }

            if(ack.sequenceNumber == iterator && ack.valid) {
                std::cout << "Received packet #" << ack.sequenceNumber << " successfully! [ " << ack.contents.data() << " ]" << std::endl;
                myPacket.sequenceNumber = ack.sequenceNumber;
                myPacket.valid = true;

                iterator++;

                sendAck(serverSocket);
                break;
            } else {
                std::cout << "Received packet #" << ack.sequenceNumber << "... valid = " << ack.valid << std::endl;
                sendAck(serverSocket);
            }

        }
    }

}
