//
// Created by Spencer Keith on 11/17/22.
//

#ifndef DATA_TRANSFER_PROTOCOLS_SIT_ERRORS_H
#define DATA_TRANSFER_PROTOCOLS_SIT_ERRORS_H


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

#endif //DATA_TRANSFER_PROTOCOLS_SIT_ERRORS_H
