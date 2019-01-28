#pragma once
#include "thread.h"
#include "kernel.h"
#include "scheduler.h"
class Kernel;
class Thread;
class Scheduler;
class Buffer
{
  public:
    Buffer();

    Buffer(int bufferId);
    List<string>* getMessages();
    bool isActive();
    void setAliveFlag(bool f);
    Thread* getSender();
    Thread* getReceiver();
    int getBufferID();
    void setSender(Thread *s);
    void setReceiver(Thread *r);

  private:
    int bufferId;
    bool isAlive;
    Thread *sender;
    Thread *receiver;
    List<string> *messages;

};

