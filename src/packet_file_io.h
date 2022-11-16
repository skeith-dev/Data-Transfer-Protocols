//
// Created by Spencer Keith on 11/15/22.
//

#ifndef DATA_TRANSFER_PROTOCOLS_PACKET_FILE_IO_H
#define DATA_TRANSFER_PROTOCOLS_PACKET_FILE_IO_H

#include <iostream>
#include <fstream>


/**
 * Opens the file at a specified filePath and returns file size in bytes
 * @param filePath The path of the file being opened
 * @return The size of the file at the specified filePath
 */
int openFile(const std::string& filePath) {

    std::ifstream fileInputStream;

    fileInputStream.open(filePath, std::ios_base::in | std::ios_base::binary);
    if (fileInputStream.fail()) {
        throw std::fstream::failure("Failed while opening file " + filePath);
    }

    fileInputStream.seekg(0, fileInputStream.end);
    int fileSize = (int) fileInputStream.tellg();
    fileInputStream.close();

    std::cout << std::endl << "File to deliver: " << filePath << std::endl << "File size: " << fileSize << " bytes" << std::endl << std::endl;

    return fileSize;

}

/**
 * Reads a file to a "packet" char array and sends to server
 * @param clientSocket The client socket which sends messages to the server
 * @param serverAddress The address of the server, which receives messages from the client socket
 * @param filePath The path of the file being read to a "packet" and sent to the server
 * @param sequenceNumber The protocol iterator
 * @param packetSize Specified size of packets to be sent
 * @param fileSize Size of file in bytes
 * @param fileSizeRangeOfSequenceNumbers The range of sequence numbers necessary to send the whole file
 */
void sendPacket(int clientSocket, sockaddr_in serverAddress, const std::string& filePath, int sequenceNumber, int packetSize, int fileSize, int fileSizeRangeOfSequenceNumbers) {

    //create ifstream object
    std::ifstream fileInputStream;
    //open file at filepath in read and binary modes
    fileInputStream.open(filePath, std::ios_base::in | std::ios_base::binary);
    //navigate to section of file beginning at (sequenceNumber * packetSize) offset from beginning
    fileInputStream.seekg(sequenceNumber * packetSize, std::ios_base::beg);

    //create char array for sequenceNumber int
    char sequenceNumberBytes[sizeof(int)];
    std::copy(static_cast<const char*>(static_cast<const void*>(&sequenceNumber)),
              static_cast<const char*>(static_cast<const void*>(&sequenceNumber)) + sizeof(sequenceNumber),
              sequenceNumberBytes);
    //create char array for valid bool
    char validBytes[sizeof(bool)];
    std::copy(static_cast<const char*>(static_cast<const void*>(&sequenceNumber)),
              static_cast<const char*>(static_cast<const void*>(&sequenceNumber)) + sizeof(sequenceNumber),
              sequenceNumberBytes);
    //create char array for file contents
    char contentsBytes[packetSize];
    if(sequenceNumber + 1 < fileSizeRangeOfSequenceNumbers) {
        fileInputStream.read(contentsBytes, packetSize);
    } else {
        int remainingBytes = fileSize - (sequenceNumber * packetSize);
        std::cout << "REMAINING BYTES: " << remainingBytes << std::endl;
        fileInputStream.read(contentsBytes, remainingBytes);
    }
    //construct char array "packet"
    char packet[sizeof(int) + sizeof(bool) + packetSize];
    for(int i = 0; i < sizeof(int); i++) {
        packet[i] = sequenceNumberBytes[i];
    }
    for(int i = 0; i < sizeof(bool); i++) {
        packet[i + sizeof(int)] = validBytes[i];
    }
    for(int i = 0; i < packetSize; i++) {
        packet[i + sizeof(int) + sizeof(bool)] = contentsBytes[i];
    }

    fileInputStream.close();

    sendto(clientSocket, packet, sizeof(packet), 0, (const struct sockaddr *) &serverAddress, sizeof(serverAddress));

    std::cout << "Sent Packet #" << sequenceNumber << ": [ ";
    for(int i = 0; i < sizeof(int) + sizeof(bool) + packetSize; i++) {
        std::cout << packet[i];
    }
    std::cout << " ]" << std::endl;

}

#pragma clang diagnostic push
#pragma ide diagnostic ignored "cert-msc50-cpp"
#pragma ide diagnostic ignored "cert-msc51-cpp"
/*void generateRandomSituationalErrors() {

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

}*/
#pragma clang diagnostic pop

/*void generateUserSituationalErrors() {

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

}*/

/*bool checkIfDropPacket() {

    for(int situationalErrorsIteration : situationalErrorsIterations) {
        if(situationalErrorsIterator > 1 && situationalErrorsIterator % situationalErrorsIteration == 0) {
            std::cout << situationalErrorsIterator << " % " << situationalErrorsIteration << " = " << 0 << std::endl;
            return true;
        }
    }
    return false;

}*/

#endif //DATA_TRANSFER_PROTOCOLS_PACKET_FILE_IO_H
