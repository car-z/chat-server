/*
 * Implementation of class describing a room object.
 * CSF Assignment 5
 * Caroline Zhao
 * czhao67@jhu.edu
 * Miranda Qing
 * mqing2@jhu.edu
 */

#include "guard.h"
#include "message.h"
#include "message_queue.h"
#include "user.h"
#include "room.h"

/*
 * Default constructor for Room object. 
 *
 * Returns:
 *   a new instance of a Room object
 *   with the mutex initialied.
 */
Room::Room(const std::string &room_name)
  : room_name(room_name) {
  // initialize the mutex
  pthread_mutex_init(&lock, NULL);
}

/*
 * Destructor for a Room object.
 * Ensures that the mutex is destroyed
 */
Room::~Room() {
  // destroy the mutex
  pthread_mutex_destroy(&lock);
}

/*
 * Function to add a user to the room
 *
 * Parameters:
 *   user - pointer to User object to be added to the room
 */
void Room::add_member(User *user) {
  // lock the room mutex before modifying
  Guard guard(lock);
  // add User to the room
  members.insert(user);
}

/*
 * Function to remove a user from the room
 *
 * Parameters:
 *   user - pointer to User object to be removed from the room
 */
void Room::remove_member(User *user) {
  // lock the room mutex before modifying
  Guard guard (lock);
  // remove User from the room
  members.erase(user);
}

/*
 * Function to broadcast a message from the sender to the room
 *
 * Parameters:
 *   sender_username - string representing the username of the sender
 *   message_text - string representing the message to be broadcasted to the room
 */
void Room::broadcast_message(const std::string &sender_username, const std::string &message_text) {
  // lock the room mutex for duration of broadcasting this msg
  Guard guard(lock);

  // get the message to be delivered from server to receivers
  std::string room_name = get_room_name();
  std::string msg_data = room_name + ":" + sender_username + ":" + message_text;
  
  std::set<User *>::iterator u_it;
  for (u_it = members.begin(); u_it != members.end(); u_it++) {
    if ((*u_it)->username != sender_username) {
      Message* msg = new Message(TAG_DELIVERY, msg_data);
      (*u_it)->mqueue.enqueue(msg);
    }
  }
}