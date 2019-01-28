#include "buffer.h"
#include "main.h"
Buffer::Buffer()
{
    isAlive = true;
    this->bufferId = kernel->messageBuffer->FindAndSet();
    this->messages = new List<string>();
}

Buffer::Buffer(int bufferId)
{
    isAlive = true;
    this->bufferId = bufferId;
    this->messages = new List<string>();
}

void Buffer::setSender(Thread *sender){
     this->sender = sender;
}

void Buffer::setReceiver(Thread *receiver){
     this->receiver = receiver;
}

List<string>* Buffer::getMessages()
{
    return messages;
}

bool Buffer::isActive()
{
    return this->isAlive;
}

void Buffer::setAliveFlag(bool f)
{
    this->isAlive = f;
}

Thread* Buffer::getSender()
{
    return this->sender;
}

Thread* Buffer::getReceiver()
{
    return this->receiver;
}

int Buffer::getBufferID()
{
    return this->bufferId;
}

