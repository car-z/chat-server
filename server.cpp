/*
 * Implementation of class describing a server object.
 * CSF Assignment 5
 * Caroline Zhao
 * czhao67@jhu.edu
 * Miranda Qing
 * mqing2@jhu.edu
 */

#include <pthread.h>
#include <iostream>
#include <sstream>
#include <memory>
#include <set>
#include <vector>
#include <cctype>
#include <cassert>
#include "message.h"
#include "connection.h"
#include "user.h"
#include "room.h"
#include "guard.h"
#include "server.h"

////////////////////////////////////////////////////////////////////////
// Server implementation data types
////////////////////////////////////////////////////////////////////////

// datatype to encapsualte data to be sent to the worker threads
typedef struct ConnInfo {
  Connection *conn;
  Server *server;
} ConnInfo;

/*
* Helper function to send error message to clients.
*
* Parameters:
*   payload - string holding the error message to send
*   conn - pointer to the Connection object to send the message through
*
* Returns:
*   true if the error message is succesfully sent
*/
bool sendError(std::string payload, Connection *conn) {
  Message error(TAG_ERR, payload);
  return conn->send(error); 
};

/*
* Helper function to send OK message to clients.
*
* Parameters:
*   payload - string holding the OK message to send
*   conn - pointer to the Connection object to send the message through
*
* Returns:
*   true if the OK message is succesfully sent
*/
bool sendOK(std::string payload, Connection *conn) {
  Message okay(TAG_OK, payload);
  return conn->send(okay); 
}

/*
* Helper function to help tear down the client (remove it cleanly from the server)
*
* Parameters:
*   user - pointer to the user (client) which is being removed
*   info - pointer to the ConnInfo struct
*/
void tear_down_client(User* user, ConnInfo* info) {
  // remove the user from the room
  if (!user->room.empty()) {
    Room* room = info->server->find_or_create_room(user->room);
    room->remove_member(user);
  }
}

/*
* Helper function to delete (free) dynamically allocated pointers
*
* Parameters:
*   info - pointer to the ConnInfo struct
*/
void cleanup(ConnInfo* info) {
  if (info != nullptr) {
    if (info->conn != nullptr) {
      info->conn->close();
      delete info->conn;
    }
    delete info;
  }
}

/*
* Helper function to handle possible sender commands when the sender is in a room
*
* Parameters:
*   info - pointer to the ConnInfo struct
*   user - pointer to User object
*   incoming_msg - Message holding the message from the sender
*   room - reference to pointer to the room the sender is in
* 
* Returns:
*   true if the command is succesfully processed
*/
bool handleRoomExists(ConnInfo *info, User *user, Message incoming_msg, Room*& room) {
  if (incoming_msg.tag == TAG_JOIN) {
    room->remove_member(user);
    room = info->server->find_or_create_room(incoming_msg.data);
    room->add_member(user);
    if (!sendOK("joining room", info->conn)) {
      room->remove_member(user);
      return false;
    }
  } else if (incoming_msg.tag == TAG_SENDALL) {
    room->broadcast_message(user->username, incoming_msg.data);
    if (!sendOK("broadcasting message", info->conn)) {
      return false;
    }  
  } else if (incoming_msg.tag == TAG_LEAVE) {
    room->remove_member(user);
    room = nullptr;
    if (!sendOK("leaving the room", info->conn)) {
      return false;
    }
  } else {
    if (!sendError("invalid message", info->conn)) {
      return false;
    }
  }
  return true;
}

/*
* Helper function to handle possible sender commands when the sender is not in a room
*
* Parameters:
*   info - pointer to the ConnInfo struct
*   user - pointer to User object
*   incoming_msg - Message holding the message from the sender
*   room - reference to pointer to the room the sender is in
* 
* Returns:
*   true if the command is succesfully processed
*/
bool handleRoomDoesNotExist(ConnInfo *info, User *user, Message incoming_msg, Room*& room) {
  if (incoming_msg.tag == TAG_JOIN) {
    room = info->server->find_or_create_room(incoming_msg.data);
    room->add_member(user);
    if (!sendOK("joining room", info->conn)) {
      room->remove_member(user);
      return false;
    }
  } else {
    sendError("You must join a room first", info->conn);
  }
  return true;
}

/*
* Helper function to handle an error receiving a message from a sender client
*
* Parameters:
*   info - pointer to the ConnInfo struct
* 
* Returns:
*   true if the error is succesfully processed and the server can continue
*/
bool handleErrorSender(ConnInfo *info) {
  if (info->conn->get_last_result() == Connection::EOF_OR_ERROR) {
    sendError("There is an error", info->conn);
    return false;
  } else if (info->conn->get_last_result() == Connection::INVALID_MSG) {
    sendError("The message is invalid.", info->conn);
    return false;
  }
  sendError("Server failed to receive message", info->conn);
  return true;
}

////////////////////////////////////////////////////////////////////////
// Client thread functions
////////////////////////////////////////////////////////////////////////

/*
* Client thread function to handle server-sender relationship
*
* Parameters:
*   info - pointer to the ConnInfo struct
*   user - pointer to the User object for this client
*/
void chat_with_sender(ConnInfo *info, User *user) {
  Room* room = nullptr;
  Message incoming_msg;
  // infinite loop unless error
  while (1) {
    // IF ERROR RECEIVING MESSAGE
    if (!(info->conn->receive(incoming_msg))) {
      if (!handleErrorSender(info)) {
        cleanup(info);
        return;
      }
    } else {
      // NO ERROR RECEIVING MESSAGE
      if (incoming_msg.tag == TAG_QUIT) {
        sendOK("quitting", info->conn);
        if (room != nullptr) {
          room->remove_member(user);
          room = nullptr;
        }
        cleanup(info);
        return;
      } else if (incoming_msg.tag == TAG_ERR) {
          sendError(incoming_msg.data, info->conn);
          cleanup(info);
          return;
      } else if (room != nullptr) {
        if (!handleRoomExists(info, user, incoming_msg, room)) {
          cleanup(info);
          return;
        }
      } else {
        //SENDER IS NOT IN A ROOM
        if (!handleRoomDoesNotExist(info, user, incoming_msg, room)) {
          cleanup(info);
          return;
        }
      }
    }
  }
}

/*
* Client thread function to handle server-receiver relationship
*
* Parameters:
*   info - pointer to the ConnInfo struct
*   user - pointer to the User object for this client
*/
void chat_with_receiver(ConnInfo *info, User *user) {
  Message msg;
  // IF ERROR RECEIVING MESSAGE
  if (!(info->conn->receive(msg))) {
    sendError("failed to join room", info->conn);
    tear_down_client(user, info);
    user = nullptr;
    cleanup(info);
    return;
  } else if (msg.tag != TAG_JOIN) {
    // NEED TO JOIN ROOM BEFORE ALL OTHER OPERATIONS
    sendError("Need to join room first", info->conn);
    tear_down_client(user,info);
    user = nullptr;
    cleanup(info);
    return;
  } else {
      // join command is called:
      user->room = msg.data;
      // finding pointer to room that this receiver is in
      Room* room = info->server->find_or_create_room(msg.data);
      // adding user to the room
      room->add_member(user);

      if (!sendOK("succesfully joined room.", info->conn)) {
        tear_down_client(user,info);
        user = nullptr;
        cleanup(info);
        return;
      };

      while(1) {
      // take a message off the message queue
      Message *msg = user->mqueue.dequeue(); 
      // if a message exists
      if (msg != nullptr) {
        // send message
        if (!info->conn->send(*msg)) {
          // ERROR SENDING MESSAGE
          tear_down_client(user, info);
          user = nullptr;
          cleanup(info);
          return;
        }
        delete msg;
      }
    }
  }
};

namespace {

/*
* Main function run by every client thread
*
* Parameters:
*   arg - generic pointer to any data to be passed to thread
*/
void *worker(void *arg) {
  pthread_detach(pthread_self());

  ConnInfo* info = static_cast<ConnInfo*>(arg);

  Message msg;
  
  if (!(info->conn->receive(msg))) {
    sendError("failed to login", info->conn);
    cleanup(info);
    return nullptr;
  }
  // First message must be a login
  if (msg.tag == TAG_SLOGIN || msg.tag == TAG_RLOGIN) {
    if(!sendOK("logged in", info->conn)) {
      cleanup(info);
      return nullptr;
    }
  } else {
    sendError("Must login first", info->conn);
    cleanup(info);
    return nullptr;
  }

  User* user = new User(msg.data);

  // start functions for sender and receiver clients 
  if (msg.tag == TAG_SLOGIN) {
    chat_with_sender(info, user);
    info = nullptr;
  } else if (msg.tag == TAG_RLOGIN) {
    chat_with_receiver(info, user);
    info = nullptr;
  }

  if (user != nullptr) {
    delete user;
  }
  if (info != nullptr) {
    cleanup(info);
  }
  return nullptr;
}

}

////////////////////////////////////////////////////////////////////////
// Server member function implementation
////////////////////////////////////////////////////////////////////////

/*
 * Non-Defefault constructor for Server object. 
 *
 * Parameters:
 *  port - port number that server will run on
 *
 * Returns:
 *   a new instance of a Server object
 *   which runs on specified port and with the
 *   mutex initialized.
 */
Server::Server(int port)
  : m_port(port)
  , m_ssock(-1) {
  pthread_mutex_init(&m_lock, NULL);
}

/*
 * Destructor for a Server object.
 * Insures that the mutex is destroyed too.
 */
Server::~Server() {
  pthread_mutex_destroy(&m_lock);
}

/*
 * Opens a listening socket on  server's specified port.
 *
 * Returns:
 *   true if the listening socket was successfully opened.
 */
bool Server::listen() {
  std::string portString = std::to_string(m_port);
  m_ssock = open_listenfd(portString.c_str());
  if (m_ssock < 0) {
    return false;
  }
  return true;
}

/*
 * Accepts incoming client connections and creates a thread for each new one.
 */
void Server::handle_client_requests() {  
  while (1){
    // call accept, returns a fd of a TCP socket that the server can use to communicate w client
    int clientfd = accept(m_ssock, NULL, NULL);
    if (clientfd < 0) {
      std::cerr << "Error accepting client connection" << std::endl;
      return;
    }

    // dynamically allocate object that will be passed to thread
    ConnInfo *info = new ConnInfo;
    info->conn = new Connection(clientfd);
    // all connections share one server (this one)
    info->server = this;
    
    // create a thread for the accepted client connection 
    pthread_t thr_id;
    if (pthread_create(&thr_id, NULL, worker, info) != 0) {
      std::cerr << "thread creation failed" << std::endl;
      delete info;
      return;
    }
  }
}

/*
 * Finds an existing room in the server by name or creates a new one if it does not exist.
 *
 * Parameters:
 *    room_name - string holding name of the room to search for
 *
 * Returns:
 *    a pointer to the room which was found / created with the specified room_name
 */
Room *Server::find_or_create_room(const std::string &room_name) {
  // lock the server mutex before modifying
  Guard guard(m_lock);

  // search for the indicated chat room
  std::map<std::string, Room*>::iterator room_it = m_rooms.find(room_name);
  if (room_it != m_rooms.end()) {
    return room_it->second; // if the Room exists, return the pointer to it
  } else { // else create a new Room with this name
    m_rooms[room_name] = new Room(room_name);
    return m_rooms[room_name];
  }
}