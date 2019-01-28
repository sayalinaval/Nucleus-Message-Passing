// exception.cc
//	Entry point into the Nachos kernel from user programs.
//	There are two kinds of things that can cause control to
//	transfer back to here from user code:
//
//	syscall -- The user code explicitly requests to call a procedure
//	in the Nachos kernel.  Right now, the only function we support is
//	"Halt".
//
//	exceptions -- The user code does something that the CPU can't handle.
//	For instance, accessing memory that doesn't exist, arithmetic errors,
//	etc.
//
//	Interrupts (which can also cause control to transfer from user
//	code into the Nachos kernel) are handled elsewhere.
//
// For now, this only handles the Halt() system call.
// Everything else core dumps.
//
// Copyright (c) 1992-1996 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

#include "../lib/copyright.h"
#include "../threads/main.h"
#include "syscall.h"
#include "ksyscall.h"
#include "../threads/thread.h"
#include "buffer.h"
#include "sstream"
#include <vector>
//----------------------------------------------------------------------
// ExceptionHandler
// 	Entry point into the Nachos kernel.  Called when a user program
//	is executing, and either does a syscall, or generates an addressing
//	or arithmetic exception.
//
// 	For system calls, the following is the calling convention:
//
// 	system call code -- r2
//		arg1 -- r4
//		arg2 -- r5
//		arg3 -- r6
//		arg4 -- r7
//
//	The result of the system call, if any, must be put back into r2.
//
// If you are handling a system call, don't forget to increment the pc
// before returning. (Or else you'll loop making the same system call forever!)
//
//	"which" is the kind of exception.  The list of possible exceptions
//	is in machine.h.
//----------------------------------------------------------------------


string readString(int begLoc)
{
    int val;string value = "";    
    while (true){
        kernel->machine->ReadMem(begLoc, 1, &val);
        if ((char)val == '\0'){
            break;
        }else{
        	value += (char)val;
            begLoc++;
        }
    }
	return value;
}

Buffer *authComm(int bID, Thread *rcv)
{
	Buffer *buf = kernel->bufferPool[bID]->Front();
	if (buf != NULL){
		if (buf->getReceiver() == rcv){
			return buf;
		}
	}
}


void write(string message, int startLoc)
{
	while (true){
		if (message == "\0"){
			break;
		}
		else{
			int msg;
			stringstream(message) >> msg;
			kernel->machine->WriteMem(startLoc, 1, msg);
			startLoc++;	
		}
	}
}

Buffer *takeBuf(int bufferId,string sender){
	std::map<int, Buffer *> messagesQueue = kernel->currentThread->messageQueue;
	if (!messagesQueue.empty()){
		if (messagesQueue.find(bufferId) != messagesQueue.end()){
			return messagesQueue[bufferId];
		}
		else if(sender != ""){
			vector<Buffer *> vec;
			for (map<int, Buffer *>::iterator it = messagesQueue.begin(); it != messagesQueue.end(); ++it){
				vec.push_back(it->second);
			}
			for (vector<Buffer *>::iterator it=vec.begin(); it!=vec.end(); it++){
				if ((*it)->getSender()->getName() == sender){
					return (*it);
				}
			}
		}
	}
	return NULL;
}


void ExceptionHandler(ExceptionType which)
{
	int type = kernel->machine->ReadRegister(2);

	DEBUG(dbgSys, "Received Exception " << which << " type: " << type << "\n");

	switch (which)
	{
	case SyscallException:
		switch (type)
		{
		case SC_Halt:
			DEBUG(dbgSys, "Shutdown, initiated by user program.\n");
			SysHalt();

			ASSERTNOTREACHED();
			break;

		case SC_Add:
			DEBUG(dbgSys, "Add " << kernel->machine->ReadRegister(4) << " + " << kernel->machine->ReadRegister(5) << "\n");

			/* Process SysAdd Systemcall*/
			int result;
			result = SysAdd(/* int op1 */ (int)kernel->machine->ReadRegister(4),
							/* int op2 */ (int)kernel->machine->ReadRegister(5));

			DEBUG(dbgSys, "Add returning with " << result << "\n");
			/* Prepare Result */
			kernel->machine->WriteRegister(2, (int)result);

			/* Modify return point */
			{
				/* set previous programm counter (debugging only)*/
				kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg));

				/* set programm counter to next instruction (all Instructions are 4 byte wide)*/
				kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);

				/* set next programm counter for brach execution */
				kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg) + 4);
			}

			return;

			ASSERTNOTREACHED();

			break;
		case SC_Exit:
      	{
			IntStatus oldLevel = kernel->interrupt->SetLevel(IntOff);
          	kernel->currentThread->Finish();  
          	(void) kernel->interrupt->SetLevel(oldLevel);       
          	return;
		}
          
		case Send_Message:
		{
			IntStatus oldLevel = kernel->interrupt->SetLevel(IntOff);
			cout << "Send Message " << "\n" << "----------------" << endl;
			cout << "limit " << kernel->limit << endl;
			int bufferId = kernel->machine->ReadRegister(6);
			string rec = readString((int)kernel->machine->ReadRegister(4));
			string msg = readString((int)kernel->machine->ReadRegister(5));
			int buffId;
			Thread *t = Thread::getByName(rec);
			if(t->getName() != rec){
				if (bufferId >= 0)
					kernel->clearBuffer(bufferId);
				kernel->currentThread->Finish();
			}else{
				if (kernel->currentThread->getCount() <= kernel->limit){
					if (bufferId == -1){
						Buffer *buff = new Buffer();
						buff->setSender(kernel->currentThread);
						buff->setReceiver(t);
						buffId = buff->getBufferID();
						if (buffId == -1){
							cout << "No buffer free. Please try later." << endl;
							kernel->machine->WriteRegister(2, buffId);
							{
								kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg));
								kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);
								kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg) + 4);
							}
							(void)kernel->interrupt->SetLevel(oldLevel);
							return;
							break;
						}
		            	cout << "BufferId: " <<buffId <<endl;
		            	kernel->currentThread->bufferMap.insert({buffId, rec});
						buff->getMessages()->Append(msg);
						List<Buffer *> *messageList = new List<Buffer *>();
						messageList->Append(buff);
						kernel->bufferPool[buffId] = messageList;
						t->messageQueue[buffId] = buff;
						kernel->machine->WriteRegister(2, buffId);

						cout << kernel->currentThread->getName() << " Sending Message to " << rec << endl;
						cout <<  "Message: " << msg << endl;
					}
					else{
						Buffer *buff2 = authComm(bufferId, t);
						if (buff2 == NULL){
							kernel->machine->WriteRegister(2, -1);
							cout << "Malicious Process " << kernel->currentThread->getName() << " trying to communicate with process " << rec << endl;
						}
						else{
							buff2->getMessages()->Append(msg);
							t->messageQueue[bufferId] = buff2;
							kernel->machine->WriteRegister(2, bufferId);
							cout << kernel->currentThread->getName() << " Sending Message to " << rec << endl;
							cout <<  "Message: " << msg << endl;
						}
					}
				}
				else{
					Buffer *buff3 = authComm(bufferId, t);
					if (buff3 != NULL){
						buff3->getMessages()->Append("This is dummy message");
						t->messageQueue[bufferId] = buff3;
						kernel->machine->WriteRegister(2, bufferId);
					}
					cout<<"Message limit exceeds"<<endl;
				}
			}
			{
				/* set previous programm counter (debugging only)*/
				kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg));

				/* set programm counter to next instruction (all Instructions are 4 byte wide)*/
				kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);

				/* set next programm counter for brach execution */
				kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg) + 4);
			}
			(void)kernel->interrupt->SetLevel(oldLevel);
			return;
			break;
		}

		case Wait_Message:
		{
			DEBUG(dbgSys, "Wait Message " << kernel->machine->ReadRegister(4) <<"\n");
			cout << "Wait Message " << "\n" << "----------------" << endl;
			IntStatus oldLevel = kernel->interrupt->SetLevel(IntOff);
			string sender = readString((int)kernel->machine->ReadRegister(4));
			int bufferID = kernel->machine->ReadRegister(6);
			{
				if(Thread::getByName(sender) == NULL){
					Buffer *buff5 = takeBuf(bufferID, sender);
					if (buff5 != NULL && !buff5->getMessages()->IsEmpty()){
						string message = buff5->getMessages()->RemoveFront();
						kernel->machine->WriteRegister(2, buff5->getBufferID());
						kernel->currentThread->mcount++;
						cout << "BufferId: " << bufferID << endl;
						cout << kernel->currentThread->getName() << " Received Message from " << sender << endl;
						cout << "Message: " << message << endl;
					}
					else{
						if (bufferID >= 0)
							kernel->clearBuffer(bufferID);
						kernel->currentThread->Finish();
						kernel->machine->WriteRegister(2, -1);
						cout << "Sender " << sender << " does not exist"<<endl;
						cout << "Sending dummy message to process " << kernel->currentThread->getName() << endl;
					}
					{
						kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg));
						kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);
						kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg) + 4);
					}
					(void)kernel->interrupt->SetLevel(oldLevel);
					return;
					break;
				}else{
					Thread *s = Thread::getByName(sender);
					std::map<int, string>::iterator it;
					for (it = s->bufferMap.begin(); it != s->bufferMap.end(); ++it){
						if (it->second == kernel->currentThread->getName()){
							bufferID = it->first;
							break;
						}
					}
					Buffer *buff4 = takeBuf(bufferID, sender);
					if (buff4 != NULL && !buff4->getMessages()->IsEmpty()){
						cout << "BufferId: " << bufferID << endl;
						string message = buff4->getMessages()->RemoveFront();
						kernel->machine->WriteRegister(2, buff4->getBufferID());
						kernel->currentThread->mcount++;
						cout << kernel->currentThread->getName() << " Received Message from " << sender << endl;
						cout << "Message: " << message << endl;
						{
							kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg));
							kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);
							kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg) + 4);
						}
						(void)kernel->interrupt->SetLevel(oldLevel);
						return;
						break;
					}
					else{
						kernel->currentThread->Yield();
					}
				}	
			}
			(void)kernel->interrupt->SetLevel(oldLevel);
			return;
			break;
		}
		case Send_Answer:
		{
			DEBUG(dbgSys, "Send Answer " << kernel->machine->ReadRegister(4) << "\n");
			cout << "Send Answer " << "\n" << "----------------" << endl;
			IntStatus oldLevel = kernel->interrupt->SetLevel(IntOff);

			int bufferID = kernel->machine->ReadRegister(6);
			int result = kernel->machine->ReadRegister(4);
			string answer = readString((int)kernel->machine->ReadRegister(5));
			Buffer *buff6 = kernel->currentThread->messageQueue[bufferID];
			if (buff6 != NULL){
				Thread *receiver = buff6->getSender();
				if (receiver != NULL && Thread::IsTAlive(receiver->getThreadId())){
					if (receiver->messageQueue[bufferID] == NULL){
						Buffer *buff = new Buffer(bufferID);
						buff->setSender(kernel->currentThread);
						buff->setReceiver(receiver);
						buff->getMessages()->Append(answer);
						kernel->bufferPool[bufferID]->Append(buff);
						receiver->messageQueue[bufferID] = buff;
					}
					else{
						receiver->messageQueue[bufferID]->getMessages()->Append(answer);
					}
					kernel->machine->WriteRegister(2, kernel->currentThread->getThreadId());
					kernel->currentThread->mcount--;
					cout << "BufferId: " << bufferID << endl;
					cout << kernel->currentThread->getName() << " Sending Answer to " << receiver->getName() << endl;
					cout << "Answer: " << answer << endl;
				}
				else{
					if (bufferID >= 0)
						kernel->clearBuffer(bufferID);
					kernel->currentThread->Finish();

					kernel->machine->WriteRegister(2, -1);
					cout << "Receiver does not exist" << endl;
				}
			}
			kernel->currentThread->Yield();
			{
				kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg));

				kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);

				kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg) + 4);
			}
			(void)kernel->interrupt->SetLevel(oldLevel);
			return;
			break;
		}
		case Wait_Answer:
		{
			DEBUG(dbgSys, "Wait Answer " << kernel->machine->ReadRegister(4)  << "\n");
			cout << "Wait Answer " << "\n" << "----------------" << endl;
			IntStatus oldLevel = kernel->interrupt->SetLevel(IntOff);
			int bufferID = kernel->machine->ReadRegister(6);
			string sender = kernel->currentThread->getName();	
			Buffer *buff7 = takeBuf(bufferID, sender);
			if(buff7 == NULL){
				kernel->currentThread->Yield();
			}
			else if(buff7->getMessages()->IsEmpty()){
				kernel->currentThread->Yield();
			}

			sender = "";
			if (kernel->currentThread->messageQueue[bufferID] != NULL)
				sender = kernel->currentThread->messageQueue[bufferID]->getSender()->getName();
			else{
				sender = kernel->bufferPool[bufferID]->Front()->getReceiver()->getName();
			}
			{
				if(Thread::getByName(sender) == NULL){
					Buffer *buff7 = takeBuf(bufferID, sender);
					if (buff7 != NULL && !buff7->getMessages()->IsEmpty()){
						string message = buff7->getMessages()->RemoveFront();
						kernel->machine->WriteRegister(2, kernel->currentThread->getThreadId());
						cout << "Wait Answer " << "\n" << "----------------" << endl;
						cout << "BufferId: " << bufferID << endl;
						cout << kernel->currentThread->getName() << " Received Answer from " << sender << endl;
						cout << "Answer: " << message << endl;
					}
					else{
						if (bufferID >= 0)
							kernel->clearBuffer(bufferID);
						kernel->currentThread->Finish();
						write("This is dummy message", 0);
						kernel->machine->WriteRegister(2, 0);
						cout << "Dummy Message sent to " << kernel->currentThread->getName() <<" as Receiver " <<sender<<" does not exist" << endl;
					}
					break;
				}else{
					buff7 = takeBuf(bufferID, sender);
					if (buff7 != NULL && !buff7->getMessages()->IsEmpty()){
						string message = buff7->getMessages()->RemoveFront();
						kernel->machine->WriteRegister(2, kernel->currentThread->getThreadId());
						cout << "Wait Answer " << "\n" << "----------------" << endl;
						cout << "BufferId: " << bufferID << endl;
						cout << kernel->currentThread->getName() << " Received Answer from " << sender << endl;
						cout << "Answer: " << message << endl;	
					}
					else{
						kernel->currentThread->Yield();
					}
				}
			}
			{
				kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg));

				kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);

				kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg) + 4);
			}
			(void)kernel->interrupt->SetLevel(oldLevel);
			return;
			break;
		}
		default:
			cerr << "Unexpected system call " << type << "\n";
			break;
		}
		
		break;
	default:
		cerr << "Unexpected user mode exception" << (int)which << "\n";
		
		break;
	}
	ASSERTNOTREACHED();
}
