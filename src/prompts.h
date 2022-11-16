//
// Created by Spencer Keith on 11/15/22.
//

#ifndef DATA_TRANSFER_PROTOCOLS_PROMPTS_H
#define DATA_TRANSFER_PROTOCOLS_PROMPTS_H


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

int rangeOfSequenceNumbersPrompt(int fileSizeRangeOfSequenceNumbers) {

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

std::string inputFilePathPrompt() {

    std::cout << "What is the filepath of the file you wish to send:" << std::endl;

    std::string responseString;
    std::getline(std::cin, responseString);

    return responseString;

}

std::string outputFilePathPrompt() {

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

#endif //DATA_TRANSFER_PROTOCOLS_PROMPTS_H
