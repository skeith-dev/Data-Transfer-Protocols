//
// Created by Spencer Keith on 4/18/22.
//

#ifndef DATA_TRANSFER_PROTOCOLS_PACKET_H
#define DATA_TRANSFER_PROTOCOLS_PACKET_H

#include <vector>


struct Packet {
    int sequenceNumber;
    char contents[MAX_INPUT];
    bool valid;
};

#endif //DATA_TRANSFER_PROTOCOLS_PACKET_H
