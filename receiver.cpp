/*
 * Main function and helper functions for the Receiver.
 * CSF Assignment 5
 * Caroline Zhao
 * czhao67@jhu.edu
 * Miranda Qing
 * mqing2@jhu.edu
 */

#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>
#include "csapp.h"
#include "message.h"
#include "connection.h"
#include "client_util.h"
#include "user.h"

/*
 * Function to handle message request (responses) from server.
 *
 * Parameters:
 *   connection - Connection object representing conection between receiver and server
 * 
 * Returns:
 *   true if response from server is OK (no errors)
 */
bool handleResponse(Connection &connection) {
  Message response;
  if (!connection.receive(response)) {
    if (connection.get_last_result() == Connection::INVALID_MSG) {
      std::cerr << "Invalid message" << std::endl;
    } else {
      std::cerr << "Failed to receive response from the server" << std::endl;
    }
    return false;
  }
  if (response.tag == TAG_ERR) {
    std::cerr << response.data << std::endl;
    return false;
  } 
  if (response.tag != TAG_OK) {
    std::cerr << "Failed to receive OK from server" << std::endl;
    return false;
  }
  return true;
}

/*
 * Function to handle a delivering a message to receiver.
 *
 * Parameters:
 *   payload - string representing the payload of the delivery request 
 *   room_name - string representing the name of the room
 */
void handleDelivery(std::string payload, std::string room_name) {
  size_t nextColon = payload.find(':');
  std::string deliveryRoom = payload.substr(0, nextColon);
  std::string msgContent = payload.substr(nextColon+1);
  if (deliveryRoom == room_name) {
    nextColon = msgContent.find(':');
    std::string sender = msgContent.substr(0, nextColon);
    std::string messageText = msgContent.substr(nextColon+1);
    std::cout << sender << ": " << messageText << std::endl;
  }
}

/*
 * Function to handle main loop for receiver.
 *
 * Parameters:
 *   connection - Connection object representing conection between receiver and server
 *   room_name - string representing the name of the room the receiver is in
 * 
 * Returns:
 *   true if message is sucesfully received
 */
bool handleLoop(Connection& connection, std::string room_name) {
  Message msg;
  if(!connection.receive(msg)) {
    std::cerr << "Connection closed or error in reading message." << std::endl;
    return false;
  }
  if (msg.tag == TAG_ERR) {
    std::cerr << msg.data << std::endl;
    connection.close();
    return false;
  } else if (msg.tag == TAG_DELIVERY) {
    handleDelivery(msg.data, room_name);
  } else {
    std::cerr << "Unexpected message tag: " << msg.tag << std::endl;
    connection.close();
    return false;
  }
  return true;
}

/*
 * Main Function which runs the receiver client of the server. 
 *
 * Parameters:
 *   argc - integer identifying how many arguments were passed in on command line
 *   argv - array of C strings where each argument is an individiual string
 *
 * Returns:
 *   0 if the receiver runs succesfully
 *   1 if the receiver throws an error
 */
int main(int argc, char **argv) {
  if (argc != 5) {
    std::cerr << "Usage: ./receiver [server_address] [port] [username] [room]\n";
    return 1;
  }

  std::string room_name = argv[4];

  // creating new connection object
  Connection connection;
  if(!(connection.connect(argv[1], std::stoi(argv[2])))) {
    std::cerr << "Failed to connect to server" << std::endl;
    return 1;
  }

  // send rlogin request
  Message rlogin_msg(TAG_RLOGIN, argv[3]);
  if(!connection.send(rlogin_msg)) {
    std::cerr << "Failed to send rlogin request" << std::endl;
    connection.close();
    return 1;
  }

  // check the server's response
  if (!handleResponse(connection)) {
    connection.close();
    return 1;
  }

  // send rjoin request
  Message rjoin_msg(TAG_JOIN, room_name);
  if(!connection.send(rjoin_msg)) {
    std::cerr << "Failed to send rjoin request" << std::endl;
    connection.close();
    return 1;
  }

  // check the server's response
  if (!handleResponse(connection)) {
    connection.close();
    return 1;
  }

  // loop waiting for messages from server
  while (1) {
    if (!handleLoop(connection, room_name)) {
      return 1;
    }
  }

  return 0;
}
