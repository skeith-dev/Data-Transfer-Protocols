//
// Created by Spencer Keith on 4/18/22.
//

#include <iostream>
#include <vector>
#include <string>
#include <iterator>
#include <fstream>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "prompts.h"
#include "packet_file_io.h"

#define FINAL_SEQUENCE_NUMBER -1


//*****//*****//*****//*****//*****//*****//*****//*****//*****//*****//
//Fields      //*****//*****//*****//*****//*****//*****//*****//*****//

//Packet myPacket;

int iterator; //iterator for network protocols
int situationalErrorsIterator; //iterator used to drop packets of a particular iteration
std::vector<int> situationalErrorsIterations; //indices of packets to be dropped (ex. every 5th packet)


void executeSAWProtocol(int clientSocket, sockaddr_in serverAddress);

void executeGBNProtocol(int clientSocket, sockaddr_in serverAddress);

//*****//*****//*****//*****//*****//*****//*****//*****//*****//*****//
//Function implementations (including main)      //*****//*****//*****//

int main() {

    //set up UDP socket
    struct sockaddr_in serverAddress = {0};
    int clientSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if(clientSocket == -1) {
        perror("Failed to create client socket");
        exit(EXIT_FAILURE);
    }

    bool quit; //true for yes, false for no
    do {

        serverAddress.sin_family = AF_INET;

        //prompt user for each of the following fields
        //IP address of the target server
        std::string ipAddress = ipAddressPrompt();
        serverAddress.sin_addr.s_addr = inet_addr(ipAddress.c_str());
        //port number of the target server
        int portNum = portNumPrompt();
        serverAddress.sin_port = htons(portNum);
        //0 for S&W, 1 for GBN, 2 for SR
        int protocolType = protocolTypePrompt();
        //specified size of packets to be sent
        int packetSize = packetSizePrompt();
        //user-specified (0+) or default (-1)
        int timeoutInterval = timeoutIntervalPrompt();
        //ex. [1, 2, 3, 4, 5, 6, 7, 8], size = 8
        int slidingWindowSize;
        if(protocolType != 0) {
            slidingWindowSize = slidingWindowSizePrompt();
        }
        //none (0), randomly generated (1), or user-specified (2)
        int situationalErrors = situationalErrorsPrompt();
        switch (situationalErrors) {
            case 1:
                //generateRandomSituationalErrors();
                break;
            case 2:
                //generateUserSituationalErrors();
                break;
            default:
                break;
        }
        //path to file to be sent
        std::string filePath = inputFilePathPrompt();
        //size of file in bytes
        int fileSize = openFile(filePath);
        //the range of sequence numbers necessary to send the whole file
        int fileSizeRangeOfSequenceNumbers = fileSize / packetSize + fileSize % packetSize;
        //ex. (sliding window size = 3) [1, 2, 3] -> [2, 3, 4] -> [3, 4, 5], range = 5
        int rangeOfSequenceNumbers = rangeOfSequenceNumbersPrompt(fileSizeRangeOfSequenceNumbers);

        switch (protocolType) {
            case 0:
                std::cout << std::endl << "Executing Stop & Wait protocol..." << std::endl << std::endl;
                executeSAWProtocol(clientSocket, serverAddress);
                break;
            case 1:
                std::cout << std::endl << "Executing Go Back N protocol..." << std::endl << std::endl;
                executeGBNProtocol(clientSocket, serverAddress);
                break;
            case 2:
                std::cout << std::endl << "Executing Selective Repeat protocol..." << std::endl << std::endl;
                //executeSRProtocol();
                break;
            default:
                break;
        }

        quit = quitPrompt();

    } while(!quit);

    return 0;

}

//*****//*****//*****//*****//*****//*****//*****//*****//*****//*****//
//Printing functions

void printWindow(int iterator, int slidingWindowSize, int rangeOfSequenceNumbers) {

    std::cout << "Current window = [ ";
    for(int i = iterator; i < iterator + slidingWindowSize; i++) {
        if(i < rangeOfSequenceNumbers) {
            std::cout << i << " ";
        }
    }
    std::cout << "]" << std::endl;

}

//*****//*****//*****//*****//*****//*****//*****//*****//*****//*****//
//Network protocols (algorithms)

void executeSAWProtocol(int clientSocket, sockaddr_in serverAddress, std::string filePath, int fileSize, int packetSize, int rangeOfSequenceNumbers, int fileSizeRangeOfSequenceNumbers, int timeoutInterval) {

    std::chrono::system_clock::time_point startTime;
    std::chrono::system_clock::time_point endTime;

    std::chrono::system_clock::time_point timerStart;

    int serverSize = sizeof(serverAddress);

    startTime = std::chrono::system_clock::now();

    bool outstanding = false;
    iterator = 0;
    situationalErrorsIterator = 0;
    while(true) {

        if(iterator >= rangeOfSequenceNumbers) {
            sendPacket(clientSocket, serverAddress, filePath, iterator, packetSize, fileSize, fileSizeRangeOfSequenceNumbers);
            situationalErrorsIterator++;
            break;
        }

        //Packet myAck{};

        if(!outstanding) {
            sendPacket(clientSocket, serverAddress, filePath, iterator, packetSize, fileSize, fileSizeRangeOfSequenceNumbers);
            situationalErrorsIterator++;
            timerStart = std::chrono::system_clock::now();
            outstanding = true;
        }

        if(recvfrom(clientSocket, &myAck, sizeof(myAck), MSG_DONTWAIT, (struct sockaddr*)&serverAddress, reinterpret_cast<socklen_t *>(&serverSize)) != -1) {
            std::cout << "Received ack #" << myAck.sequenceNumber << std::endl;
            iterator = myAck.sequenceNumber + 1;
            timerStart = std::chrono::system_clock::now();
            outstanding = false;
        }

        std::chrono::duration<double> timer = std::chrono::system_clock::now() - timerStart;
        if(timer.count() >= timeoutInterval) {
            std::cout << "Timed out! " << timer.count() << " > " << timeoutInterval << " (timeout interval)" << std::endl;
            timerStart = std::chrono::system_clock::now();
            sendPacket(clientSocket, serverAddress, filePath, iterator, packetSize, fileSize, fileSizeRangeOfSequenceNumbers);
            situationalErrorsIterator++;
        }

    }

    endTime = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsedSeconds = endTime - startTime;
    std::cout << std::endl << "Total execution time = " << elapsedSeconds.count() << std::endl;

}

/*
void executeGBNProtocol(int clientSocket, sockaddr_in serverAddress) {

    std::chrono::system_clock::time_point startTime;
    std::chrono::system_clock::time_point endTime;

    std::chrono::system_clock::time_point timerStart;

    int serverSize = sizeof(serverAddress);

    startTime = std::chrono::system_clock::now();

    iterator = 0;
    situationalErrorsIterator = 0;
    int next = 0;
    while(true) {

        if(iterator >= rangeOfSequenceNumbers) {
            sendPacket(clientSocket, serverAddress, FINAL_SEQUENCE_NUMBER);
            situationalErrorsIterator++;
            break;
        }

        Packet myAck{};

        if(next < iterator + slidingWindowSize && next < rangeOfSequenceNumbers) {
            sendPacket(clientSocket, serverAddress, next);
            situationalErrorsIterator++;
            timerStart = std::chrono::system_clock::now();
            next++;
        }

        if(recvfrom(clientSocket, &myAck, sizeof(myAck), MSG_DONTWAIT, (struct sockaddr*)&serverAddress, reinterpret_cast<socklen_t *>(&serverSize)) != -1) {

            std::cout << "Received ack #" << myAck.sequenceNumber << std::endl;

            iterator = myAck.sequenceNumber + 1;
            printWindow();

        }

        std::chrono::duration<double> timer = std::chrono::system_clock::now() - timerStart;
        if(std::abs(timer.count()) >= timeoutInterval) {

            std::cout << "Timed out! " << timer.count() << " >= " << timeoutInterval << " (timeout interval)" << std::endl;

            for(int i = iterator; i < next; i++) {
                sendPacket(clientSocket, serverAddress, i);
                situationalErrorsIterator++;
            }

            timerStart = std::chrono::system_clock::now();

        }

    }

    endTime = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsedSeconds = endTime - startTime;
    std::cout << std::endl << "Total execution time = " << elapsedSeconds.count() << std::endl;

}
*/