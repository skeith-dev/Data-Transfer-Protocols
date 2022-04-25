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
#include "packet.h"

#define FINAL_SEQUENCE_NUMBER -1


//*****//*****//*****//*****//*****//*****//*****//*****//*****//*****//
//Fields      //*****//*****//*****//*****//*****//*****//*****//*****//

Packet myPacket;

std::string ipAddress; //IP address of the target server
int portNum; //port number of the target server
int protocolType; //0 for S&W, 1 for GBN, 2 for SR
int packetSize; //specified size of packets to be sent
int timeoutInterval; //user-specified (0+) or default (-1)
int slidingWindowSize; //ex. [1, 2, 3, 4, 5, 6, 7, 8], size = 8
int situationalErrors; //none (0), randomly generated (1), or user-specified (2)
std::string filePath; //path to file to be sent
int rangeOfSequenceNumbers; //ex. (sliding window size = 3) [1, 2, 3] -> [2, 3, 4] -> [3, 4, 5], range = 5
bool quit; //true for yes, false for no

int fileSize; //size of file in bytes
int fileSizeRangeOfSequenceNumbers; //the range of sequence numbers necessary to send the whole file

int iterator; //iterator for network protocols
int situationalErrorsIterator; //iterator used to drop packets of a particular iteration
std::vector<int> situationalErrorsIterations; //indices of packets to be dropped (ex. every 5th packet)

//*****//*****//*****//*****//*****//*****//*****//*****//*****//*****//
//Function declarations            //*****//*****//*****//*****//*****//

std::string ipAddressPrompt();

int portNumPrompt();

int protocolTypePrompt();

int packetSizePrompt();

int timeoutIntervalPrompt();

int slidingWindowSizePrompt();

int rangeOfSequenceNumbersPrompt();

int situationalErrorsPrompt();

std::string filePathPrompt();

bool quitPrompt();

void printWindow();

void openFile();

void writeFileToPacket(int sequenceNumber);

void sendPacket(int clientSocket, sockaddr_in serverAddress, int sequenceNumber);

void generateRandomSituationalErrors();

void generateUserSituationalErrors();

bool checkIfDropPacket();

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

    do {

        serverAddress.sin_family = AF_INET;

        //prompt user for each of the following fields
        ipAddress = ipAddressPrompt();
        serverAddress.sin_addr.s_addr = inet_addr(ipAddress.c_str());

        portNum = portNumPrompt();
        serverAddress.sin_port = htons(portNum);

        protocolType = protocolTypePrompt();
        packetSize = packetSizePrompt();
        timeoutInterval = timeoutIntervalPrompt();

        if(protocolType != 0) {
            slidingWindowSize = slidingWindowSizePrompt();
        }

        situationalErrors = situationalErrorsPrompt();
        switch (situationalErrors) {
            case 1:
                generateRandomSituationalErrors();
                break;
            case 2:
                generateUserSituationalErrors();
                break;
            default:
                break;
        }

        filePath = filePathPrompt();
        openFile();

        rangeOfSequenceNumbers = rangeOfSequenceNumbersPrompt();

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

    std::cout << "Type of protocol, S&W (0), GBN (1), or SR (2):" << std::endl;

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

int timeoutIntervalPrompt() {

    std::cout << "Timeout interval, user-specified or ping calculated (-1):" << std::endl;

    std::string responseString;
    std::getline(std::cin, responseString);

    int responseInt = std::stoi(responseString);
    if(responseInt >= 0) {
        return responseInt;
    } else {
        return 2;
    }

}

int slidingWindowSizePrompt() {

    std::cout << "Size of sliding window:" << std::endl;

    std::string responseString;
    std::getline(std::cin, responseString);

    return std::stoi(responseString);

}

int rangeOfSequenceNumbersPrompt() {

    std::cout << "Range of sequence numbers (" << fileSizeRangeOfSequenceNumbers << " required to send entire file):" << std::endl;

    std::string responseString;
    std::getline(std::cin, responseString);

    return std::stoi(responseString);

}

int situationalErrorsPrompt() {

    std::cout << "Situational errors; none (0), randomly generated (1), or user-specified (2):" << std::endl;

    std::string responseString;
    std::getline(std::cin, responseString);

    return std::stoi(responseString);

}

std::string filePathPrompt() {

    std::cout << "What is the filepath of the file you wish to send:" << std::endl;

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
    for(int i = iterator; i < iterator + slidingWindowSize; i++) {
        if(i < rangeOfSequenceNumbers) {
            std::cout << i << " ";
        }
    }
    std::cout << "]" << std::endl;

}

void openFile() {

    std::ifstream fileInputStream;

    fileInputStream.open(filePath, std::ios_base::in | std::ios_base::binary);
    if (fileInputStream.fail()) {
        throw std::fstream::failure("Failed while opening file " + filePath);
    }

    fileInputStream.seekg(0, fileInputStream.end);
    fileSize = (int) fileInputStream.tellg();
    fileSizeRangeOfSequenceNumbers = fileSize / packetSize + fileSize % packetSize;

    std::cout << std::endl << "File to deliver: " << filePath << std::endl << "File size: " << fileSize << " bytes" << std::endl << std::endl;

    fileInputStream.close();

}

void writeFileToPacket(int sequenceNumber) {

    //create ifstream object
    std::ifstream fileInputStream;
    //open file at filepath in read and binary modes
    fileInputStream.open(filePath, std::ios_base::in | std::ios_base::binary);
    //navigate to section of file beginning at (sequenceNumber * packetSize) offset from beginning
    fileInputStream.seekg(sequenceNumber * packetSize, std::ios_base::beg);

    //create char array for file contents
    char contents[packetSize];
    //read file contents into array of amount packetSize
    fileInputStream.read(contents, packetSize);

    //set global packet struct sequence number
    myPacket.sequenceNumber = sequenceNumber;
    if(myPacket.sequenceNumber != FINAL_SEQUENCE_NUMBER) {
        //copy the contents of the array to the global packet struct char vector
        for (int i = 0; i < packetSize; i++) {
            myPacket.contents[i] = contents[i];
        }
    } else {
        for (int i = 0; i < packetSize; i++) {
            myPacket.contents[i] = '\0';
        }
    }

    fileInputStream.close();

}

void sendPacket(int clientSocket, sockaddr_in serverAddress, int sequenceNumber) {

    writeFileToPacket(sequenceNumber);

    myPacket.valid = true;

    if(!checkIfDropPacket()) {
        sendto(clientSocket, &myPacket, sizeof(myPacket), 0, (const struct sockaddr *) &serverAddress,
               sizeof(serverAddress));

        std::cout << "Sent Packet #" << myPacket.sequenceNumber << ": [ ";
        for(int i = 0; i < packetSize; i++) {
            std::cout << myPacket.contents[i];
        }
        std::cout << " ]" << std::endl;
    } else{
        std::cout << "Dropped Packet #" << myPacket.sequenceNumber << std::endl;
    }

}

//*****//*****//*****//*****//*****//*****//*****//*****//*****//*****//
//Situational errors

#pragma clang diagnostic push
#pragma ide diagnostic ignored "cert-msc50-cpp"
#pragma ide diagnostic ignored "cert-msc51-cpp"
void generateRandomSituationalErrors() {

    srand(time(nullptr));

    //generate a random number between 1 and 3
    int numberOfErrors = (rand() % 3) + 1;

    //insert a random number between 2 and 10 into the vector
    for(int i = 0; i < numberOfErrors; i++) {
        situationalErrorsIterations.insert(situationalErrorsIterations.end(), (rand() % 10) + 2);
    }

    //remove duplicate elements from the vector
    sort(situationalErrorsIterations.begin(), situationalErrorsIterations.end());
    situationalErrorsIterations.erase( unique(situationalErrorsIterations.begin(), situationalErrorsIterations.end() ), situationalErrorsIterations.end() );

    std::cout << "Every iteration of each of the following packets will be dropped: ";
    for(int situationalErrorIndex : situationalErrorsIterations) {
        std::cout << situationalErrorIndex << " ";
    }
    std::cout << std::endl << std::endl;

}
#pragma clang diagnostic pop

void generateUserSituationalErrors() {

    std::string droppedPacketString;
    int droppedPacketCount;

    std::cout << "How many packet iterations do you want to drop:" << std::endl;
    std::getline(std::cin, droppedPacketString);

    droppedPacketCount = std::stoi(droppedPacketString);

    for(int i = 0; i < droppedPacketCount; i++) {
        std::string packetIterationString;
        std::cout << "#" << i + 1 << " packet iteration to be dropped:" << std::endl;
        std::getline(std::cin, packetIterationString);

        situationalErrorsIterations.insert(situationalErrorsIterations.end(), std::stoi(packetIterationString));
    }

    std::cout << "Every iteration of each of the following packets will be dropped: ";
    for(int situationalErrorIndex : situationalErrorsIterations) {
        std::cout << situationalErrorIndex << " ";
    }
    std::cout << std::endl << std::endl;

}

bool checkIfDropPacket() {

    for(int situationalErrorsIteration : situationalErrorsIterations) {
        if(situationalErrorsIterator > 1 && situationalErrorsIterator % situationalErrorsIteration == 0) {
            std::cout << situationalErrorsIterator << " % " << situationalErrorsIteration << " = " << 0 << std::endl;
            return true;
        }
    }
    return false;

}

//*****//*****//*****//*****//*****//*****//*****//*****//*****//*****//
//Network protocols (algorithms)

void executeSAWProtocol(int clientSocket, sockaddr_in serverAddress) {

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
            sendPacket(clientSocket, serverAddress, FINAL_SEQUENCE_NUMBER);
            situationalErrorsIterator++;
            break;
        }

        Packet myAck{};

        if(!outstanding) {
            sendPacket(clientSocket, serverAddress, iterator);
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
            sendPacket(clientSocket, serverAddress, iterator);
            situationalErrorsIterator++;
        }

    }

    endTime = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsedSeconds = endTime - startTime;
    std::cout << std::endl << "Total execution time = " << elapsedSeconds.count() << std::endl;

}

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