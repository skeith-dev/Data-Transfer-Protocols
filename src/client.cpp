//
// Created by Spencer Keith on 4/18/22.
//

#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include "packet.h"


//*****//*****//*****//*****//*****//*****//*****//*****//*****//*****//
//Fields      //*****//*****//*****//*****//*****//*****//*****//*****//

Packet myPacket;

std::string ipAddress; //IP address of the target server
int portNum; //port number of the target server
int protocolType; //0 for S&W, 1 for GBN, 2 for SR
int packetSize; //specified size of packets to be sent
int timeoutInterval; //user-specified (0+) or ping calculated (-1)
int slidingWindowSize; //ex. [1, 2, 3, 4, 5, 6, 7, 8], size = 8
int rangeOfSequenceNumbers; //ex. (sliding window size = 3) [1, 2, 3] -> [2, 3, 4] -> [3, 4, 5], range = 5
int situationalErrors; //none (0), randomly generated (1), or user-specified (2)
bool quit; //true for yes, false for no

std::string filePath; //path to file to be sent
int fileSize; //size of file in bytes

//DECLARE UDP SOCKET GLOBALS HERE

bool outstanding; //flag to indicate packets still in transit
int iterator; //iterator for network protocols
int situationalErrorsIterator;
std::vector<int> situationalErrorsIterations; //indices of packets to be dropped (ex. every 5th packet)
bool *acksPtr; //ptr to bool array; true for ack received, false for ack not (yet) received

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

void writeFileToPacket();

void sawSignalHandler(int signal);

void gbnSignalHandler(int signal);

void srSignalHandler(int signal);

void generateRandomSituationalErrors();

void generateUserSituationalErrors();

void executeSAWProtocol();

void executeGBNProtocol();

void executeSRProtocol();

//*****//*****//*****//*****//*****//*****//*****//*****//*****//*****//
//Function implementations (including main)      //*****//*****//*****//

int main() {

    do {

        //prompt user for each of the following fields
        ipAddress = ipAddressPrompt();
        portNum = portNumPrompt();
        protocolType = protocolTypePrompt();
        packetSize = packetSizePrompt();
        timeoutInterval = timeoutIntervalPrompt();

        if(protocolType != 0) {
            slidingWindowSize = slidingWindowSizePrompt();
        }

        rangeOfSequenceNumbers = rangeOfSequenceNumbersPrompt();

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

        bool acks[rangeOfSequenceNumbers];
        acksPtr = acks;

        switch (protocolType) {
            case 0:
                executeSAWProtocol();
                break;
            case 1:
                executeGBNProtocol();
                break;
            case 2:
                executeSRProtocol();
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
    //clear the current contents of the global packet struct char vector
    myPacket.contents.clear();
    //copy the contents of the array to the global packet struct char vector
    std::copy(&contents[0], &contents[packetSize], back_inserter(myPacket.contents));

    fileInputStream.close();

}

//*****//*****//*****//*****//*****//*****//*****//*****//*****//*****//
//Situational errors

void generateRandomSituationalErrors() {

    srand(time(nullptr));

    //generate a random number between 1 and 3
    int numberOfErrors = (rand() % 3) + 1;

    for(int i = 0; i < numberOfErrors; i++) {
        situationalErrorsIterations.insert(situationalErrorsIterations.end(), rand() % 10);
    }

    std::cout << "Every iteration of each of the following packets will be dropped: ";
    for(int situationalErrorIndex : situationalErrorsIterations) {
        std::cout << situationalErrorIndex << " ";
    }
    std::cout << std::endl << std::endl;

}

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

//*****//*****//*****//*****//*****//*****//*****//*****//*****//*****//
//Network protocols (algorithms)

void executeSAWProtocol() {

    signal(SIGALRM, sawSignalHandler);

    auto startTime = std::chrono::system_clock::now();

    bool situationalErrorFlag;
    situationalErrorsIterator = 0;
    iterator = 0;
    while(iterator < rangeOfSequenceNumbers) {

        alarm(timeoutInterval);

        situationalErrorFlag = false;
        for(int situationalErrorIteration : situationalErrorsIterations) {
            if(situationalErrorsIterator % situationalErrorIteration == 0) {
                situationalErrorFlag = true;
            }
        }

        if(!situationalErrorFlag) {
            //READ SECTION OF FILE TO myPacket

            //SEND PACKET TO SERVER
        }

        outstanding = true;

        while(/*READ ACK RESPONSE FROM SERVER*/) {
            if(/*ACK NUM*/ == iterator) {
                outstanding = false;
                iterator++;
                break;
            }
        }

        situationalErrorsIterator++;

    }

    auto endTime = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsedSeconds = endTime - startTime;
    std::cout << std::endl << "Total execution time = " << elapsedSeconds.count() << std::endl;

}

void executeGBNProtocol() {

    signal(SIGALRM, gbnSignalHandler);

    auto startTime = std::chrono::system_clock::now();

    asio::streambuf streamBuffer;
    std::string response;

    int ackNum;
    iterator = 0;
    while(iterator < rangeOfSequenceNumbers) {

        alarm(timeoutInterval);

        //SEND WINDOW OF PACKETS TO SERVER (EXCLUDING PACKETS OF ERROR ITERATION)

        while(/*READ ACK RESPONSE FROM SERVER*/) {
            if(/*ACK NUM*/ >= iterator + slidingWindowSize || /*ACK NUM*/ >= rangeOfSequenceNumbers) {
                outstanding = false;
            }
            if(/*ACK NUM*/ > iterator) {
                iterator = /*ACK NUM*/;
                printWindow();
            }
        }

    }

    auto endTime = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsedSeconds = endTime - startTime;
    std::cout << std::endl << "Total execution time = " << elapsedSeconds.count() << std::endl;

}

void executeSRProtocol() {

    signal(SIGALRM, srSignalHandler);

    auto startTime = std::chrono::system_clock::now();

    asio::streambuf streamBuffer;
    std::string response;

    int ackNum;
    iterator = 0;
    while(iterator < rangeOfSequenceNumbers) {

        alarm(timeoutInterval);

        //SEND WINDOW OF PACKETS TO SERVER (EXCLUDING PACKETS OF ERROR ITERATION)

        while(/*READ ACK RESPONSE FROM SERVER*/) {
            if(/*ACK NUM*/ >= iterator + slidingWindowSize || /*ACK NUM*/ >= rangeOfSequenceNumbers) {
                outstanding = false;
            }
            if(/*ACK NUM*/ >= iterator) {
                acksPtr[/*ACK NUM*/] = true;

                while(acksPtr[iterator]) {
                    iterator++;
                    printWindow();
                }
            }
        }

    }

    auto endTime = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsedSeconds = endTime - startTime;
    std::cout << std::endl << "Total execution time = " << elapsedSeconds.count() << std::endl;

}