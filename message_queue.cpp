/*
 * Implementation of class describing a queue of Messages waiting to be delivered to a receiver
 * CSF Assignment 5
 * Caroline Zhao
 * czhao67@jhu.edu
 * Miranda Qing
 * mqing2@jhu.edu
 */

#include <cassert>
#include <ctime>
#include "message_queue.h"
#include "guard.h"
#include "message.h"

/*
 * Default constructor for MessageQueue object. 
 *
 * Returns:
 *   a new instance of a MessageQueue object
 *   with the mutex and semaphore initialied.
 */
MessageQueue::MessageQueue() {
  // initialize the mutex
  pthread_mutex_init(&m_lock, NULL);
  // initialize the semaphore
  sem_init(&m_avail, 0, 0);
}

/*
 * Destructor for a MessageQueue object.
 * Ensures that the mutex and semaphore are destroyed
 * and that all pointers in queue are freed.
 */
MessageQueue::~MessageQueue() {
  // destroy the mutex and the semaphore
  pthread_mutex_destroy(&m_lock);
  sem_destroy(&m_avail);
  //  free pointers in queue
  std::deque<Message *>::iterator msg_it;
  for (msg_it = m_messages.begin(); msg_it != m_messages.end(); msg_it++){
    delete (*msg_it);
  }
}

/*
 * Function to add a Message to the MessageQueue
 *
 * Parameters:
 *   msg - pointer to Message object
 */
void MessageQueue::enqueue(Message *msg) {
  // lock the mqueue mutex before modifying it
  Guard guard(m_lock);
  // put the specified message on the queue
  m_messages.push_back(msg);
  
  // be sure to notify any thread waiting for a message to be
  // available by calling sem_post
  sem_post(&m_avail);
}

/*
 * Function to remove a Message from the MessageQueue
 *
 * Returns:
 *   a pointer to the removed Message object
 */
Message *MessageQueue::dequeue() {
  struct timespec ts;

  // get the current time using clock_gettime:
  // we don't check the return value because the only reason
  // this call would fail is if we specify a clock that doesn't
  // exist
  clock_gettime(CLOCK_REALTIME, &ts);

  // compute a time one second in the future
  ts.tv_sec += 1;

  // call sem_timedwait to wait up to 1 second for a message to be available
  int msg_wait = sem_timedwait(&m_avail, &ts);
  // return nullptr if no message is available
  if (msg_wait == -1) {
    return nullptr;
  }

  Message *msg = nullptr;
  // lock the mqueue mutex before modifying it
  Guard guard(m_lock);
  // remove the next message from the queue, return it
  msg = m_messages.front();
  m_messages.pop_front();
  return msg;
}
