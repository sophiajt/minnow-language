#ifndef AQUARIUM_HPP
#define AQUARIUM_HPP

#include <ctime>
#include <deque>
#include <ext/hash_map>
#include <iostream>
#include <map>
#include <vector>

#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/thread.hpp>

#include <time.h>

#include "simplearray.h"

//FIXME: I'm using the union in the code with UInt32, but in 64-bit machines that probably isn't best
#define actorId_t uint32_t
#define threadId_t uint32_t

const int RECEIVER_CACHE_SIZE=3;
const unsigned int TIMESLICE_QUANTUM=2000;

class ThreadType { public: enum Type { THREAD, MAILMAN, KERNEL}; };
class ActorState { public: enum State { ACTIVE, WAITING_FOR_ACTION, WAITING_FOR_DATA, DELETED, MOVED }; };
class MessageType { public: enum Type { ACTION_MESSAGE, DATA_MESSAGE, ADD_ACTOR_ID, MOVE_ACTOR_ID, DELETE_ACTOR_ID, DELETE_ACTOR_IDS, CREATE_ISOLATED_ACTOR, LOAD_STATUS, PLEASE_MOVE_ACTOR, PLEASE_RECV_ACTOR, PLEASE_RECV_ACTORS };  };

typedef struct Thread;
typedef struct Actor;

/**
   Hold a meta container for all basic types.
   As a side note: All functions are re-entrant, using the actor->heapStack to store state, so they only need a pointer to that actor for all their activities.
*/
union TypeUnion {
    int8_t Int8;
    uint8_t UInt8;
    int16_t Int16;
    uint16_t UInt16;
    int32_t Int32;
    uint32_t UInt32;
    int64_t Int64;
    uint64_t UInt64;
    float Float;
    double Double;
    bool Bool;
    void(*Function)(Actor *);
    void *VoidPtr;
};

/**
   Message container.  For messages with longer number of arguments, create a vector and use VoidPtr in TypeUnion.
*/
struct Message {
    MessageType::Type messageType;
    actorId_t recipient;
    uint32_t dataTaskTypeId;
    void(*task)(Actor *);
    int numArgs;
    TypeUnion arg[4];
};

/**
   Channel between threads, and between threads and the kernel.
*/
class MailChannel {
    std::vector<Message> *msgChannelIncoming, *msgChannel1, *msgChannel2;
    volatile int currentChannel;
    boost::mutex mailLock;
    volatile bool isEmpty;

    /*
    void DebugMessage(const Message &message) {
        std::cout << "  Message: " << message.messageType << " rec:" << message.recipient << " taskId:";
        std::cout << message.dataTaskTypeId << " nArgs:" << message.numArgs << " arg0:" << message.arg[0].UInt32 << " " << message.arg[1].UInt32 << " " << message.arg[2].UInt32 << std::endl;
    }
    */

  public:
    void sendMessage(const Message &message) {
        boost::mutex::scoped_lock lock(mailLock);
        msgChannelIncoming->push_back(message);
        isEmpty = false;
    }

    std::vector<Message> *recvMessages() {
        boost::mutex::scoped_lock lock(mailLock);

        if (currentChannel == 1) {
            msgChannelIncoming = msgChannel2;
            currentChannel = 2;
            isEmpty = msgChannelIncoming->empty();
            return msgChannel1;
        }
        else {
            msgChannelIncoming = msgChannel1;
            currentChannel = 1;
            isEmpty = msgChannelIncoming->empty();
            return msgChannel2;
        }
    }

    bool empty() {
        //boost::mutex::scoped_lock lock(mailLock);
        return isEmpty;
    }

    MailChannel() {
        msgChannel1 = new std::vector<Message>();
        msgChannel2 = new std::vector<Message>();

        msgChannelIncoming = msgChannel1;
        currentChannel = 1;
        isEmpty = true;
    }
};

struct ThreadPoolThread {
    bool available;
    threadId_t threadId;
    boost::thread *thread;
};

struct ActorWrapper {
    struct Actor* actor;
    actorId_t containedId;
    uint32_t actorPoolRevId;
};

/**
   The microthread task, which we call an actor.
*/
struct Actor {
  public:
    ActorState::State actorState;
    ActorState::State actorStateBeforeMove;
    actorId_t actorId;
    void(*task)(Actor *);
    Thread *parentThread;

    struct ActorWrapper receiverCache[RECEIVER_CACHE_SIZE];
    int nextCacheIndex;

    uint32_t runQueueRevId;

    //incoming DATA_MESSAGE messages have a queue and handlers for each enumerated type.  In the action, a switch statement allows for continuations
    //based on the id of the continuation (basically which slot to jump back into when the function is restarted).
    std::vector<Message> dataMessages;
    __gnu_cxx::hash_map<int, int> dataHandlers;

    //For CPS
    bool isResuming;

    std::vector<Message> actionMessages;
    std::vector<TypeUnion> heapStack;
    //SimpleArray actionMessages;
    //SimpleArray heapStack;

    Actor() : runQueueRevId(0), isResuming(false) {}
};


/**
   The actual OS-level thread.  This will act as a scheduler for actors(microthreads).
*/
class Thread {
  private:
    __gnu_cxx::hash_map<actorId_t, Actor*> actorIds;
    std::vector<Message> localMail;
    bool isRunning;
    actorId_t nextActorId;
    bool sendStatus;

    int32_t numberActiveActors;
    int32_t previousActiveActors;

  public:
    ThreadType::Type threadType;
    std::vector<Actor*> runningActors;
    threadId_t threadId;
    MailChannel *incomingChannel;
    MailChannel *outgoingChannel;

    //For mailmen
    std::vector<MailChannel*> *mailListIncoming;
    __gnu_cxx::hash_map<actorId_t, threadId_t> *mailAddresses;
    __gnu_cxx::hash_map<threadId_t, MailChannel*> *mailChannelOutgoing;

    //For the kernel
    __gnu_cxx::hash_map<threadId_t, int> *scheduleWeights;  //I'm assuming int is big enough
    std::vector<ThreadPoolThread> *threadPoolThreads;

    //For local receiver caching, whenever our actor pool members change, we rev this so that local receiver caches can be recreated
    uint32_t actorPoolRevId;

    //For more efficient run queue handling, this prevents actors from rescheduling themselves in loop scenerios
    uint32_t runQueueRevId;

    //For scheduling tasks, we need to know how much time is remaining
    int timeSliceEndTime;
    Actor *hotActor;

    Thread(threadId_t id, actorId_t nextActor) {
        threadType = ThreadType::THREAD;
        isRunning = true;
        sendStatus = true;
        nextActorId = nextActor;

        threadId = id;

        actorPoolRevId = 0;
        runQueueRevId = 1; //0 denotes "never scheduled", and is the reset value

        incomingChannel = new MailChannel();
        outgoingChannel = new MailChannel();

        Actor *maintenance = new Actor();
        maintenance->actorId = 0xFFFFFFFF;
        maintenance->actorState = ActorState::ACTIVE;
        runningActors.push_back(maintenance);
    }

    void SendMessage(const Message &message);
    //JDT REFACTOR
    void ReceiveMessages();
    void ScheduleExistingActor(Actor *actor);
    void ScheduleNewActor(Actor *actor);
    void ScheduleNewIsolatedActor(Actor *actor);
    void RemoveRunningActor(Actor *actor);
    void RemoveActor(Actor *actor);
    void RemoveActors(std::vector<Actor*> *actors);
    void RemoveActor(Actor *actor, bool sendDeleteMsg);

    void DeleteActor(Actor *actor);
    void SendStatus();
    void MoveHeaviestActor(threadId_t threadId, uint32_t amount);
    Actor* ActorIfLocal(actorId_t actorId, Actor *local);
    void TaskRebalance();
    void MailCheck();
    void SchedulerLoop();
};

//Main startup procedure
int VM_Main(int argc, char *argv[], void(*task)(Actor *), bool passCmdLine);

#endif //AQUARIUM_HPP
