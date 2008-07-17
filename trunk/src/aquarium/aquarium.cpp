#include <deque>
#include <ext/hash_map>
#include <iostream>
#include <map>
#include <vector>

#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/thread.hpp>
#include <boost/thread/xtime.hpp>

#include <ctime>

#include "aquarium.hpp"

Actor *Thread::ActorIfLocal(actorId_t actorId, Actor *local) {
    Actor *foundActor = NULL;

    for (int i = 0; i < RECEIVER_CACHE_SIZE; ++i) {
        if (local->receiverCache[i].containedId == actorId) {
            if (local->receiverCache[i].actorPoolRevId == this->actorPoolRevId) {
                return local->receiverCache[i].actor;
            }
        }
    }

    __gnu_cxx::hash_map<actorId_t, Actor*>::iterator finder = actorIds.find(actorId);
    if (finder != actorIds.end()) {
        foundActor = finder->second;
        int index = local->nextCacheIndex + 1;
        if (index >= RECEIVER_CACHE_SIZE) {
            index = 0;
        }
        local->receiverCache[index].actorPoolRevId = this->actorPoolRevId;
        local->receiverCache[index].actor = foundActor;
        local->receiverCache[index].containedId = actorId;
        local->nextCacheIndex = index;
    }
    return foundActor;
}
/**
   Thread scheduler uses this to send messages to itself
*/
void Thread::SendMessage(const Message &message)
{
    __gnu_cxx::hash_map<actorId_t, Actor*>::iterator finder = actorIds.find(message.recipient);
    if (finder != actorIds.end()) {
        Actor *foundActor = finder->second;

        //Receiving actors can be in one of two states to get a solution immediately.  This allows quick turn around time
        //in some cases even before the maintenance task has run.
        if (message.messageType == MessageType::ACTION_MESSAGE) {
            
            if (foundActor->actorState == ActorState::WAITING_FOR_ACTION) {
                foundActor->task = message.task;
                for (int i=0; i < message.numArgs; ++i) {
                    foundActor->heapStack.push_back(message.arg[i]);
                }            

                foundActor->actorState = ActorState::ACTIVE;
                this->hotActor = foundActor;

                if (foundActor->runQueueRevId != this->runQueueRevId) {
                    runningActors.push_back(foundActor);
                    foundActor->runQueueRevId = this->runQueueRevId;
                }
            }
            else {
                foundActor->actionMessages.push_back(message);
            }
        }
        else if ((foundActor->actorState == ActorState::WAITING_FOR_DATA) && (message.messageType == MessageType::DATA_MESSAGE)) {
            __gnu_cxx::hash_map<int, int>::iterator findHandler = foundActor->dataHandlers.find(message.dataTaskTypeId);

            if (findHandler != foundActor->dataHandlers.end()) {
                //Set up our CPS to resume into a data receive
                TypeUnion tu1;
                tu1.UInt32 = findHandler->second;
                for (int i=0; i < message.numArgs; ++i) {
                    foundActor->heapStack.push_back(message.arg[i]);
                }
                foundActor->heapStack.push_back(tu1);
                foundActor->actorState = ActorState::ACTIVE;
                foundActor->isResuming = true;
                
                this->hotActor = foundActor;

                //if (foundActor->runQueueRevId != this->runQueueRevId) {
                    runningActors.push_back(foundActor);
                    //foundActor->runQueueRevId = this->runQueueRevId;
                    //}
            }
            else {
                localMail.push_back(message);
            }
                
        }

        else {
            localMail.push_back(message);
            //foundActor->actionMessages.push_back(message);
        }
    }
    else {
        //we don't have this person, so send the message out to a mailman
        //printf("<%i", message.recipient ); fflush(stdout);
        outgoingChannel->sendMessage(message);
    }
    //printf("[%i]", message.recipient ); fflush(stdout);
}

/**
   Thread schedule uses this to check queued messages to see if any are ready to be consumed.
*/

void Thread::ReceiveMessages() {
    
    int pos = 0;
    int end = localMail.size();

    while (pos < end) {
        Message &message = localMail[pos];
        
        if (message.messageType == MessageType::ACTION_MESSAGE) {
            __gnu_cxx::hash_map<actorId_t, Actor*>::iterator finder = actorIds.find(message.recipient);
            //Check to see if the recipient is local
            if (finder != actorIds.end()) {
                Actor *foundActor = finder->second;
                
                if (foundActor->actorState == ActorState::WAITING_FOR_ACTION) {
                    foundActor->task = message.task;
                    BOOST_ASSERT(message.numArgs < 5);
                    for (int i=0; i < message.numArgs; ++i) {
                        foundActor->heapStack.push_back(message.arg[i]);
                    }
                    foundActor->actorState = ActorState::ACTIVE;
                    if (foundActor->runQueueRevId != this->runQueueRevId) {
                        runningActors.push_back(foundActor);
                        foundActor->runQueueRevId = this->runQueueRevId;
                    }
                }
                else {
                    //If the actor isn't waiting for action, re-add them to the queue (the mail queue works similarly to
                    //the running actor process queue.  Only the current batch is live at any moment, each cycle clears
                    //the old messages in one go (which is more efficient than picking them out one by one)
                    //localMail.push_back(message);
                    foundActor->actionMessages.push_back(message);
                }
                
            }            
            else {
                //we don't have this person, so send the message out to a mailman
                //printf(">%i", message.recipient ); fflush(stdout);
                outgoingChannel->sendMessage(message);
            }
        }
        
        else if (message.messageType == MessageType::DATA_MESSAGE) {
            __gnu_cxx::hash_map<actorId_t, Actor*>::iterator finder = actorIds.find(message.recipient);
            if (finder != actorIds.end()) {
                Actor *foundActor = finder->second;
                __gnu_cxx::hash_map<int, int>::iterator findHandler = foundActor->dataHandlers.find(message.dataTaskTypeId);

                if (findHandler != foundActor->dataHandlers.end()) {
                    
                    if (foundActor->actorState == ActorState::WAITING_FOR_DATA) {
                        //Set up our CPS to resume into a data receive
                        TypeUnion tu1;
                        tu1.UInt32 = findHandler->second;
                        for (int i=0; i < message.numArgs; ++i) {
                            foundActor->heapStack.push_back(message.arg[i]);
                        }
                        foundActor->heapStack.push_back(tu1);
                        foundActor->actorState = ActorState::ACTIVE;
                        foundActor->isResuming = true;

                        //if this activates the recipient, put it on the active queue
                        //if (foundActor->runQueueRevId != this->runQueueRevId) {
                            runningActors.push_back(foundActor);
                            //foundActor->runQueueRevId = this->runQueueRevId;
                            //}
                    }
                    else {
                        localMail.push_back(message);
                    }
                }
                else {
                    localMail.push_back(message);
                }
            }            
            else {
                //we don't have this person, so send the message out to a mailman
                outgoingChannel->sendMessage(message);
            }
        }
        
        else {
            //The only type of mail that should get into localMail are mail messages themselves.  Messages request
            //a service are handled by the maintenance actor
            std::cout << "ERROR: Unknown message type" << std::endl;
            localMail.push_back(message);
        }
        ++pos;
    }

    //Clean out all messages we've looked through this pass (ones that were rescheduled live further in)
    if (end != 0) {
        localMail.erase(localMail.begin(), localMail.begin() + end);
    }
}

/**
   Schedules an already existing actor onto the current thread;
*/
void Thread::ScheduleExistingActor(Actor *actor) {
    actor->parentThread = this;

    //This is our phonebook for the actors on this thread
    actorIds[actor->actorId] = actor;
    
    if (actor->actorState == ActorState::ACTIVE)
    {
        runningActors.push_back(actor);
    }

    //clear out this actor's receiver cache, as it's no longer valid
    for (int i = 0; i < RECEIVER_CACHE_SIZE; ++i) {
        actor->receiverCache[i].containedId = -1;
    }
    actor->nextCacheIndex = 0;

    actor->runQueueRevId = 0;
    //actor->timesActive = 0;

    Message message;
    message.messageType = MessageType::ADD_ACTOR_ID;
    message.arg[0].UInt32 = threadId;
    message.arg[1].UInt32 = actor->actorId;
    message.numArgs = 2;
    outgoingChannel->sendMessage(message);    
}

/**
   Schedules a new actor on the current thread
*/
void Thread::ScheduleNewActor(Actor *actor) {
    actor->actorId = nextActorId;
    actor->actorState = ActorState::WAITING_FOR_ACTION;
    actor->actorStateBeforeMove = ActorState::WAITING_FOR_ACTION;
    
    ScheduleExistingActor(actor);

    ++nextActorId;
    if ((nextActorId & 0xffffff) == 0xffffff) {
        //we have rolled over, just quit with an error for now
        std::cout << "Exceeded allowed number of actors on scheduler " << this->threadId << std::endl;
        std::cout << "This is currently a fatal error, and we must exit." << std::endl;
        exit(1);
    }
}

void Thread::ScheduleNewIsolatedActor(Actor *actor) {
    //std::cout << "New Isolated: " << nextActorId << std::endl;
    actor->actorId = nextActorId;
    actor->actorState = ActorState::WAITING_FOR_ACTION;
    actor->actorStateBeforeMove = ActorState::WAITING_FOR_ACTION;
    
    ++nextActorId;
    if ((nextActorId & 0xffffff) == 0xffffff) {
        //we have rolled over, just quit with an error for now
        std::cout << "Exceeded allowed number of actors on scheduler " << this->threadId << std::endl;
        std::cout << "This is currently a fatal error, and we must exit." << std::endl;
        exit(1);
    }
    
    Message message;
    message.messageType = MessageType::CREATE_ISOLATED_ACTOR;
    message.arg[0].VoidPtr = (void *)actor;
    message.numArgs = 1;
    this->outgoingChannel->sendMessage(message);    
}

/**
   Removes an actor from the scheduler and passes a message up the chain to remove him from the phonebooks.
   This version does not free the memory the actor was using, for that use 'DeleteActor' instead
*/
void Thread::RemoveActor(Actor *actor) {
    RemoveActor(actor, true);
}

void Thread::RemoveActors(std::vector<Actor*> *actors) {
    std::vector<actorId_t> *toBeDeleted = new std::vector<actorId_t>();

    for (std::vector<Actor*>::iterator iter=actors->begin(), end=actors->end(); iter!=end; ++iter) {
        RemoveActor(*iter, false);
        toBeDeleted->push_back((*iter)->actorId);
    }
    
    Message message;
    message.messageType = MessageType::DELETE_ACTOR_IDS;
    message.arg[0].UInt32 = threadId;
    message.arg[1].VoidPtr = toBeDeleted;
    message.numArgs = 2;
    outgoingChannel->sendMessage(message);
}

void Thread::RemoveActor(Actor *actor, bool sendDeleteMsg) {
    __gnu_cxx::hash_map<actorId_t, Actor*>::iterator iter = actorIds.find(actor->actorId);
    std::vector<Actor*>::iterator iterActor;

    ++this->actorPoolRevId;
    if (this->actorPoolRevId == 0) {
        //if we have gone so long that we roll over, then quickly invalidate our local actors' receiver caches so they don't get false positives
        for (__gnu_cxx::hash_map<actorId_t, Actor*>::iterator resetIter = actorIds.begin(), resetEnd = actorIds.end(); resetIter != resetEnd; ++resetIter) {
            for (int i = 0; i < RECEIVER_CACHE_SIZE; ++i) {
                resetIter->second->receiverCache[i].containedId = -1;
            }
        }
    }

    if (iter != actorIds.end()) {
        actorIds.erase(iter);

        if (sendDeleteMsg) {
            Message message;
            message.messageType = MessageType::DELETE_ACTOR_ID;
            message.arg[0].UInt32 = threadId;
            message.arg[1].UInt32 = actor->actorId;
            message.numArgs = 2;
            outgoingChannel->sendMessage(message);
        }
        int j = 0;
        while (j < runningActors.size()) {
            if (runningActors[j]->actorId == actor->actorId) 
                runningActors.erase(runningActors.begin() + j);
            else 
                ++j;
        }
    }
    else {
        std::cout << "Trying to remove " << actor->actorId << " from " << this->threadId << " but can't find it." << std::endl;
    }    
}

/**
   Remove the actor and completely delete it (for actors that were killed)
*/
void Thread::DeleteActor(Actor *actor) {
    
    RemoveActor(actor);
    delete actor;
}

/**
   Schedules an already existing actor onto the current thread;
*/
void Thread::SendStatus() {
    Message message;
    message.messageType = MessageType::LOAD_STATUS;
    message.arg[0].UInt32 = this->threadId;
    message.arg[1].Int32 = this->numberActiveActors;
    
    message.numArgs = 2;
    
    outgoingChannel->sendMessage(message);    
}

/**
   Moves the actor which has been running the longest to another scheduler.
   Note: Very likely better solutions exist, ones that could reschedule clumps of actors that have been working together,
   but for now this will do.
*/

//JDT: REFACTOR - breaking rebalance until we put in locality calculator
void Thread::MoveHeaviestActor(threadId_t destination, uint32_t count) {
    //std::vector<Actor*>::iterator iter, chosen;
    bool foundOne;
    std::vector<Actor*> *group;
    if (count > 1) {
        group = new std::vector<Actor*>();
    }

    //std::cout << "Moving " << count << std::endl;
    //for (int k = 0; k < count; ++k) {
    //for (iter = runningActors.begin(); iter != runningActors.end(); ++iter) {
    int i = 0;
    while (i < runningActors.size()) {
        if ( (runningActors[i]->actorState == ActorState::ACTIVE) && (runningActors[i]->actorId != 0xffffffff)) {

            //std::cout << "Found one, moving it" << std::endl;
            Actor *chosenActor = runningActors[i];
            chosenActor->actorStateBeforeMove = chosenActor->actorState;
                    
            chosenActor->actorState = ActorState::MOVED;
            chosenActor->parentThread = NULL;


            if (count == 1) {
                //std::cout << "$$ " << count << " $$" << std::endl;
                RemoveActor(chosenActor);
                
                Message message;
                message.messageType = MessageType::PLEASE_RECV_ACTOR;
                message.arg[0].UInt32 = destination;
                message.arg[1].VoidPtr = (void *)chosenActor;
        
                message.numArgs = 2;

                outgoingChannel->sendMessage(message);
                break;
                
            }
            else {
                group->push_back(chosenActor);
                if (group->size() == count) {
                    break;
                }
            }
                
        }
        else {
            ++i;
        }
    }
    
    if (count > 1) {
        RemoveActors(group);

        //std::cout << "@@ " << count << " " << group->size() << " @@" << std::endl;
        Message message;
        message.messageType = MessageType::PLEASE_RECV_ACTORS;
        message.arg[0].UInt32 = destination;
        message.arg[1].VoidPtr = (void *)group;
        
        message.numArgs = 2;
        
        outgoingChannel->sendMessage(message);
    }
    
}

/**
   The rebalancing algorithm the kernel uses to make sure the actors are equitably distributed across scheduler threads
*/
void Thread::TaskRebalance() {
    __gnu_cxx::hash_map<threadId_t, int>::const_iterator iter;
    threadId_t lightest, heaviest;
    int lightest_weight, heaviest_weight;

    lightest = 0;
    heaviest = 0;
    lightest_weight = 99999999;
    heaviest_weight = 0;

    //The rebalance works by looking through the list of schedulers and their number of active actors, and if there is a
    //disparatity greater than a threshhold (currently 2), it asks the more active scheduler to pass one of its actors
    //to the lesser active scheduler.
    
    iter = scheduleWeights->begin();
    while (iter != scheduleWeights->end()) {
        if (iter->second < lightest_weight) {
            lightest = iter->first;
            lightest_weight = iter->second;
        }
        if (iter->second > heaviest_weight) {
            heaviest = iter->first;
            heaviest_weight = iter->second;
        }
        ++iter;
    }
    if ((lightest != heaviest) && (lightest != 0) && (heaviest != 0)) {
        if ((heaviest_weight - lightest_weight) > 1 + (heaviest_weight / 10)) {
            //uint32_t amount = (heaviest_weight - lightest_weight) / 2; //(50 + heaviest_weight - lightest_weight) / 50;
            uint32_t amount = (50 + heaviest_weight - lightest_weight) / 50;
            //std::cout << "Moving: " << amount << " from " << heaviest << " to " << lightest << std::endl;
            
            //if (amount > 500) 
            //  amount = 500;
            
            Message message;
            message.messageType = MessageType::PLEASE_MOVE_ACTOR;
            message.arg[0].UInt32 = lightest;
            message.arg[1].UInt32 = amount;
            message.numArgs = 2;
            (*mailChannelOutgoing)[heaviest]->sendMessage(message);
            iter = scheduleWeights->begin();
            /*
              while (iter != scheduleWeights->end()) {
              //std::cout << iter->first << ":" << iter->second << " ";
              ++iter;
              }
            */
            //std::cout << std::endl;
        }
    }
}

/**
   The mail check loop that mailmen actors use.
*/
void Thread::MailCheck() {
    int pos = 0;
    int end = mailListIncoming->size();

    //FIXME: There has to be a nicer way to keep track of lost messages
    std::vector<Message> lostMessages;

    while (pos < end) {
        MailChannel *mChannel = (*mailListIncoming)[pos];

        //For each channel, check mail.
        if (mChannel->empty() == false) {
           
            std::vector<Message> *incomingRemoteMail = mChannel->recvMessages();
            std::vector<Message>::iterator inMailIter = incomingRemoteMail->begin();

            while (inMailIter != incomingRemoteMail->end()) {
                Message message = *inMailIter;

                if ((message.messageType == MessageType::ACTION_MESSAGE)||(message.messageType == MessageType::DATA_MESSAGE)) {
                    //find recipient
                    __gnu_cxx::hash_map<actorId_t, threadId_t>::iterator finder = mailAddresses->find(message.recipient);
                    //we found who it goes to
                    if (finder != mailAddresses->end()) {

                        //if the actor isn't in transition/limbo, we deliver
                        if (finder->second != 0) {
                            __gnu_cxx::hash_map<threadId_t, MailChannel*> *channelMap = mailChannelOutgoing;
                            MailChannel *outgoing = (*channelMap)[finder->second];
                            outgoing->sendMessage(message);
                        }
                        else {
                            std::cout << "Actor is in LIMBO" << std::endl;
                        }
                    }
                
                    else {                    
                        //can't find anyone, so send the message to myself
                        lostMessages.push_back(message);
                    }
                
                }
                else if (message.messageType == MessageType::ADD_ACTOR_ID) {
                    threadId_t threadId = message.arg[0].UInt32;
                    actorId_t actorId = message.arg[1].UInt32;
                    __gnu_cxx::hash_map<actorId_t, threadId_t> *mailMap = mailAddresses;
                    (*mailMap)[actorId] = threadId;
                }
                else if (message.messageType == MessageType::CREATE_ISOLATED_ACTOR) {
                    //FIXME: This assumes receiver is a KERNEL but doesn't pass it along if it's a MAILMAN
                
                    actorId_t actorId = message.arg[0].UInt32;
                    if (threadType == ThreadType::KERNEL) {
                        bool found = false;

                        Message messageIso;
                        messageIso.messageType = MessageType::PLEASE_RECV_ACTOR;
                        //message.arg[0] gets filled in below
                        messageIso.arg[1] = message.arg[0];  //Pass the isolated actor over to the receive actor message

                        std::vector<ThreadPoolThread>::iterator poolIter = threadPoolThreads->begin();
                        while ( (found == false) && (poolIter != threadPoolThreads->end()) ) {
                            //std::cout << "Inside search loop..." << std::endl;
                            if (poolIter->available == true) {
                                poolIter->available = false;
                                found = true;
                                messageIso.arg[0].UInt32 = poolIter->threadId;
                                //std::cout << "Found: " << poolIter->threadId << std::endl;
                            }
                            ++poolIter;
                        }
                    
                        if (found == false) {
                            ThreadPoolThread tpt;
                            //std::cout << "Not found, creating one... " << actor->mailChannelOutgoing->size() + 1 << std::endl;

                            tpt.threadId = mailChannelOutgoing->size() + 1;// + actor->threadPoolThreads->size();

                            Thread *newThread = new Thread(tpt.threadId, (tpt.threadId-1) * 0x1000000);
                        
                            //By not sending status, we'll take ourselves out of the rebalancer, which we need to do if we're in the thread pool for isolated actors.
                            newThread->sendStatus = false;
                            //std::cout << "Thread ID..." << newThread->threadId << " and " << tpt.threadId << std::endl;

                            tpt.thread = new boost::thread(boost::bind(&Thread::SchedulerLoop, newThread));
                            tpt.available = false;  //we're about to schedule something, so it's not available for new actors yet
                        
                            messageIso.arg[0].UInt32 = newThread->threadId;
                            mailListIncoming->push_back(newThread->outgoingChannel);
                        
                            (*mailChannelOutgoing)[newThread->threadId] = newThread->incomingChannel;

                            threadPoolThreads->push_back(tpt);
                        }

                        (*mailChannelOutgoing)[messageIso.arg[0].UInt32]->sendMessage(messageIso);
                    
                    }
                }
                else if (message.messageType == MessageType::DELETE_ACTOR_ID) {
                    //FIXME: When we move to a mailmen+kernel model instead of just a kernel model, we need to pass up the deleted actor
                    //threadId_t threadId = message.arg[0].UInt32;

                    actorId_t actorId = message.arg[1].UInt32;
                
                    __gnu_cxx::hash_map<actorId_t, threadId_t>::iterator finder = mailAddresses->find(actorId);
                    if (finder != mailAddresses->end()) {
                        threadId_t threadId = finder->second;
                        __gnu_cxx::hash_map<actorId_t, threadId_t> *mailMap = mailAddresses;
                        mailMap->erase(actorId);
                        for (int k = 0; k < threadPoolThreads->size(); ++k) {
                            if ((*threadPoolThreads)[k].threadId == threadId) {
                                (*threadPoolThreads)[k].available = true;
                            }
                        }
                    }
                }
                else if (message.messageType == MessageType::DELETE_ACTOR_IDS) {
                    //FIXME: When we move to a mailmen+kernel model instead of just a kernel model, we need to pass up the deleted actor
                    //threadId_t threadId = message.arg[0].UInt32;
                    std::vector<actorId_t> *actorIds = (std::vector<actorId_t> *)(message.arg[1].VoidPtr);
                    //std::cout << "Kernel: deleting " << actorIds->size() << " actors" << std::endl;
                    for (std::vector<actorId_t>::iterator actorIter = actorIds->begin(), actorEnd = actorIds->end(); actorIter != actorEnd; ++actorIter) {
                        __gnu_cxx::hash_map<actorId_t, threadId_t>::iterator finder = mailAddresses->find(*actorIter);
                        if (finder != mailAddresses->end()) {
                            threadId_t threadId = finder->second;
                            __gnu_cxx::hash_map<actorId_t, threadId_t> *mailMap = mailAddresses;
                            mailMap->erase(*actorIter);
                            for (int k = 0; k < threadPoolThreads->size(); ++k) {
                                if ((*threadPoolThreads)[k].threadId == threadId) {
                                    (*threadPoolThreads)[k].available = true;
                                }
                            }
                        }
                    }
                    delete actorIds;
                }
                else if (message.messageType == MessageType::LOAD_STATUS) {
                    threadId_t threadId = message.arg[0].UInt32;
                    int32_t activeActors = message.arg[1].Int32;

                    //std::cout << "Status: " << threadId << ": " << activeActors << std::endl;
                    (*(scheduleWeights))[threadId] = activeActors;
                    TaskRebalance();
                }
                else {
                    threadId_t threadId = message.arg[0].UInt32;
                    (*(mailChannelOutgoing))[threadId]->sendMessage(message);
                }
                ++inMailIter;
            }

            incomingRemoteMail->clear();

            //Return to sender the messages that I don't have an address for
            std::vector<Message>::iterator it = lostMessages.begin();
            while (it != lostMessages.end()) {
                mChannel->sendMessage(*it);
                ++it;
            }
            lostMessages.clear();
        }
        ++pos;
    }
    
}

/**
   The main scheduler loop for the thread.  Runs through each actor and runs the ready ones.
   Also checks for queued messages, activating them as necessary.
*/

void Thread::SchedulerLoop() {
    int pos; 
    const bool sendStatusUpdate = this->sendStatus;
    
    std::vector<Actor *> deletedActors;
    int slicePos;

    this->previousActiveActors = -1;
    this->numberActiveActors = 0;
    
    pos = 0;
    Actor *thisActor = NULL;
    int sliceMultiplier = 1;
    
    while (this->isRunning) {
        if (this->runningActors[pos]->actorState == ActorState::ACTIVE) {

            thisActor = this->runningActors[pos];

            switch (thisActor->actorId) {
                case (0xffffffff) :
                    //MAINTENANCE LOOP
                    if (this->threadType == ThreadType::KERNEL) {
                        MailCheck();
                        ++this->numberActiveActors;
                    }
                    if (this->numberActiveActors == 0) {
                        boost::xtime xt;
                        boost::xtime_get(&xt, boost::TIME_UTC);
                        xt.nsec += 1000000;
                        boost::thread::sleep(xt);
                    }

                    
                    ++this->runQueueRevId;
                    if (this->runQueueRevId == 0) {
                        //if 0, we've rolled over, so reset everyone
                        for (__gnu_cxx::hash_map<actorId_t, Actor*>::iterator resetIter = this->actorIds.begin(), resetEnd = this->actorIds.end(); resetIter != resetEnd; ++resetIter)
                        {
                            (resetIter->second)->runQueueRevId = 0;
                        }
                        this->runQueueRevId = 1;
                    }
                    
                    
                    //Clean out old slots, using ourselves (the maintenance actor) as the marker.  Actors that follow us were
                    //rescheduled (pushed) in this cycle so they are our active queue for the next cycle.
                    this->runningActors.erase(this->runningActors.begin(), this->runningActors.begin() + (1 + pos));

                    //Pull out any actors that were deleted during the last cycle
                    if (deletedActors.size() > 0) {
                        for (std::vector<Actor *>::iterator deletedIter = deletedActors.begin(), deletedEnd = deletedActors.end(); deletedIter != deletedEnd; ++deletedIter)
                        {
                            DeleteActor(*deletedIter);
                        }
                        deletedActors.clear();
                    }
                    
                    //Sort through incoming system messages (which may also have mail)
                    if (this->incomingChannel->empty() == false) {
                        //std::cout << "Thread checking mail" << std::endl;
                        std::vector<Message> *incomingRemoteMail = this->incomingChannel->recvMessages();
                        std::vector<Message>::iterator inMailIter = incomingRemoteMail->begin();
        
                        while (inMailIter != incomingRemoteMail->end()) {
                            Message message = *inMailIter;
                    
                            if ((message.messageType == MessageType::ACTION_MESSAGE)||(message.messageType == MessageType::DATA_MESSAGE)) {
                                this->localMail.push_back(message);
                            }                
                            else if (message.messageType == MessageType::PLEASE_MOVE_ACTOR) {
                                threadId_t threadId = message.arg[0].UInt32;
                                uint32_t amount = message.arg[1].UInt32;
                                MoveHeaviestActor(threadId, amount);
                            }
                            else if (message.messageType == MessageType::PLEASE_RECV_ACTOR) {
                                Actor *newActor = (Actor *)(message.arg[1].VoidPtr);
                                newActor->actorState = newActor->actorStateBeforeMove; 
                                ScheduleExistingActor(newActor);
                            }
                            else if (message.messageType == MessageType::PLEASE_RECV_ACTORS) {
                                std::vector<Actor*> *newActors = (std::vector<Actor*> *)(message.arg[1].VoidPtr);
                                //std::cout << "New message: " << newActors->size() << std::endl;
                                for(std::vector<Actor*>::iterator newIter = newActors->begin(), newEnd = newActors->end(); newIter != newEnd; ++newIter) {
                                    //std::cout << "***MOVED***" << std::endl;
                                    (*newIter)->actorState = (*newIter)->actorStateBeforeMove; 
                                    ScheduleExistingActor(*newIter);
                                }
                                delete newActors;
                            }
                            ++inMailIter;
                        }
                        incomingRemoteMail->clear();
                    }
                    pos = 0;
                    ReceiveMessages();

                    //Send a status update to the kernel if we've changed number of active actors this cycle.
                    if (sendStatusUpdate) {                        
                        if (this->numberActiveActors != this->previousActiveActors) {
                            SendStatus();
                            this->previousActiveActors = this->numberActiveActors;
                        }
                        
                    }
                    this->numberActiveActors = 0;

                    break;
                default:
                    this->timeSliceEndTime = TIMESLICE_QUANTUM;
                    if (thisActor->actionMessages.size() > TIMESLICE_QUANTUM) {
                        sliceMultiplier = thisActor->actionMessages.size() / TIMESLICE_QUANTUM;
                        this->timeSliceEndTime *= sliceMultiplier;
                    }
                        
                    this->hotActor = NULL;

                    //Running the actor if it's active, as long as it is active and has slice remaining
                    
                    slicePos = -1;
                    while ((thisActor->actorState == ActorState::ACTIVE) && (this->timeSliceEndTime > 0)) {
                        //Run the actor's current task
                        thisActor->task(thisActor);

                        //If it's still active afterward, it's an active actor, so count it
                        if (thisActor->actorState == ActorState::ACTIVE) {
                            thisActor->isResuming = true;
                            ++this->numberActiveActors;
                            //std::cout << "NAA: " << this->numberActiveActors << std::endl;
                        }
                        //If instead it's waiting for an action message, see if we have one to give it
                        else if (thisActor->actorState == ActorState::WAITING_FOR_ACTION) {
                            if (thisActor->actionMessages.size() > slicePos+1) {
                                ++slicePos;
                                Message message = thisActor->actionMessages[slicePos];
                                thisActor->task = message.task;
                                //BOOST_ASSERT(message.numArgs <= 4);
            
                                for (int i=0; i < message.numArgs; ++i) {
                                    thisActor->heapStack.push_back(message.arg[i]);
                                }            
                                thisActor->actorState = ActorState::ACTIVE;

                                if (this->timeSliceEndTime == 0) {
                                    thisActor->runQueueRevId = this->runQueueRevId;
                                    this->runningActors.push_back(thisActor);
                                }
                            }
                        }
                    }
                    
                    if (slicePos >= 0) {
                        thisActor->actionMessages.erase(thisActor->actionMessages.begin(), thisActor->actionMessages.begin() + slicePos  + 1);
                        --slicePos;
                    }

                    //In the case the actor is no longer active, it may have just sent a message.  This will let another
                    //actor immediately go active (called here a 'hot actor'), allowing them to use the rest of the slice.
                    //This idea comes from "Improving IPC by Kernel Design" (l4ka.org/publications/1993/improving-ipc.pdf)
                    
                    if (thisActor->actorState != ActorState::ACTIVE) {
                        
                        if (this->hotActor != NULL) {                            
                            Actor *hotActor = this->hotActor;
                            while ((this->timeSliceEndTime > 0) && (this->hotActor != NULL)) {
                                hotActor = this->hotActor;

                                this->hotActor = NULL;
                                hotActor->task(hotActor);
                                
                                if (hotActor->actorState == ActorState::DELETED) {
                                    deletedActors.push_back(hotActor);
                                }        
                                                        
                            }
                            
                        }
                        
                        
                        if (thisActor->actorState == ActorState::DELETED) {
                            deletedActors.push_back(thisActor);
                        }
                    }
                    ++pos;
                    break;
                
            }
            //If we're active, keep us in the running list
            if ((thisActor->actorState == ActorState::ACTIVE) && (thisActor->runQueueRevId != this->runQueueRevId)) {
                thisActor->runQueueRevId = this->runQueueRevId;
                this->runningActors.push_back(thisActor);
            }
        }
        else {
            ++pos;  //something that was active now isn't, so let's just skip over it.
        }
    }
}

int VM_Main(int argc, char *argv[], void(*task)(Actor *), bool passCmdLine) {
    boost::xtime xt;
    int NUM_SCHED_THREADS = boost::thread::hardware_concurrency();
    
    Thread *thread[NUM_SCHED_THREADS];
    
    //FIXME!!!!!!  This assumes only a certain number of actors will every be created by any one scheduler, and this might be a bad assumption.
    //Also, actor ids are not currently being recycled, but likely should be.
    for (int i = 0; i < NUM_SCHED_THREADS; ++i) {
        thread[i] = new Thread(i+1, i * 0x1000000);
    }

    //Make the kernel thread the first scheduler thread
    //It will do all normal scheduler duties and include kernel duties into them
    thread[0]->threadType = ThreadType::KERNEL;
    thread[0]->mailListIncoming = new std::vector<MailChannel*>();
    thread[0]->mailAddresses = new __gnu_cxx::hash_map<actorId_t, threadId_t>();
    thread[0]->mailChannelOutgoing = new __gnu_cxx::hash_map<threadId_t, MailChannel*>();
    thread[0]->scheduleWeights = new __gnu_cxx::hash_map<threadId_t, int>();
    thread[0]->threadPoolThreads = new std::vector<ThreadPoolThread>();
       
    //connect up channels
    for (int i = 0; i < NUM_SCHED_THREADS; ++i) {
        thread[0]->mailListIncoming->push_back(thread[i]->outgoingChannel);
        (*(thread[0])->mailChannelOutgoing)[thread[i]->threadId] = thread[i]->incomingChannel;
    }
    
    std::vector<std::string> *commandLineArgs = new std::vector<std::string>();
    //Grab commandline args
    for (int i = 1; i < argc; ++i) {
        commandLineArgs->push_back(argv[i]);
    }
    
    //Schedule the main actor (like the main function in the program)
    Actor *mainActor = new Actor();
    thread[0]->ScheduleNewActor(mainActor);

    if (passCmdLine) {
        //And send it the first message
        Message message;
        message.recipient = 0;
        message.numArgs = 1;
        message.messageType = MessageType::ACTION_MESSAGE;
        message.task = task;
        message.arg[0].VoidPtr = commandLineArgs;
        thread[0]->SendMessage(message);   
    }
    else {
        Message message;
        message.recipient = 0;
        message.numArgs = 0;
        message.messageType = MessageType::ACTION_MESSAGE;
        message.task = task;
        thread[0]->SendMessage(message);   
    }
    
    boost::thread *bThread[NUM_SCHED_THREADS];
    for (int i = 1; i < NUM_SCHED_THREADS; ++i) {
        bThread[i] = new boost::thread(boost::bind(&Thread::SchedulerLoop, thread[i]));
    }

    thread[0]->SchedulerLoop();

    //join the threads here.
    for (int i = 1; i < NUM_SCHED_THREADS; ++i) {
        bThread[i]->join();
    }    
    return 0;
}

