/*
 * rpcMessage.cpp
 *
 *  Created on: Jul 24, 2018
 *      Author: hhdang
 */
#include "rpcMessage.h"

namespace app
{

rpcMessage::rpcMessage(rpcMessageType msgType) :
        msgType(msgType),
        msgAddrType(rpcMessageAddr::system_manager_addr_t)
{
}

rpcMessage::~rpcMessage()
{
    // TODO Auto-generated destructor stub
}

bool rpcMessage::send(int fd)
{
    uint16_t buff = this->msgType;

    if(rpcMessage::sendInterruptRetry(fd, &buff, sizeof(buff)) != true) {
        return false;
    }

    this->serialize(fd);
    return true;
}

bool rpcMessage::receive(int fd)
{
    return this->deserialize(fd);
}

} /* namespace app */
