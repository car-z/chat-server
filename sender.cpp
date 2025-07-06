/*
* Main function and helper functions for the Sender.
* CSF Assignment 5
* Caroline Zhao
* czhao67@jhu.edu
* Miranda Qing
* mqing2@jhu.edu
*/

#include <iostream>
#include <string>
#include <sstream>
#include <stdexcept>
#include "csapp.h"
#include "message.h"
#include "connection.h"
#include "client_util.h"

/*
 * Function to handle message responses from server.
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
* Function to handle a sender joining a room and associated errors.
*
* Parameters:
*   room - reference to string representing name of room to be joined.
*   connection - reference to Connection object encapsulating connection between server and client
*   ss - reference to stringstream to read room name from
*/
void handleJoin(std::string& room, Connection& connection, std::stringstream& ss) {
  std::string newRoom;
  ss >> newRoom;
  room = newRoom;
  Message join_msg(TAG_JOIN, room);
  if(!connection.send(join_msg)) {
    std::cerr << "Failed to send join request" << std::endl;
    connection.close();
    exit(1);
  }
}

/*
* Function to handle a sender leaving a room and associated errors.
*
* Parameters:
*   room - reference to string representing name of room to be left.
*   connection - reference to Connection object encapsulating connection between server and client
*/
void handleLeave(std::string& room, Connection& connection) {
  room = "";
  Message leave_msg(TAG_LEAVE, "");
  if(!connection.send(leave_msg)) {
    std::cerr << "Failed to send leave request" << std::endl;
    connection.close();
    exit(1);
  }
}

/*
* Function to handle a sender quitting the server and associated errors.
*
* Parameters:
*   connection - reference to Connection object encapsulating connection between server and client
*/
void handleQuit(Connection& connection) {
  Message quit_msg(TAG_QUIT, "");
  if(!connection.send(quit_msg)) {
    std::cerr << "Failed to send quit request" << std::endl;
    connection.close();
    exit(1);
  }
}

/*
* Function to handle a command being sent from the sender.
*
* Parameters:
*   tag - reference to string representing the tag of the command
*   room - reference to string representing room that sender is in
*   connection - reference to Connection object
*   ss - reference to stringstream to read from
*/
void handleCommand(const std::string& tag, std::string& room, Connection& connection, std::stringstream& ss) {
  if (tag == "/join") {
    handleJoin(room, connection, ss);
  } else if (tag == "/leave") {
    handleLeave(room, connection);
  } else if (tag == "/quit") {
    handleQuit(connection);
    connection.close();
    exit(0);
  } else {
    std::cerr << "Invalid command." << std::endl;
  }
}

/*
* Function to handle a message being sent from the sender.
*
* Parameters:
*   message - constant reference to string representing message to be sent
*   connection - reference to Connection object
*/
void handleMessage(const std::string& message, Connection& connection) {
  Message msg(TAG_SENDALL, message);
  if(!connection.send(msg)) {
    std::cerr << "Failed to send message" << std::endl;
    connection.close();
    exit(1);
  }
}


/*
* Main Function which runs the sender client of the server.
*
* Parameters:
*   argc - integer identifying how many arguments were passed in on command line
*   argv - array of C strings where each argument is an individiual string
*
* Returns:
*   0 if the sender runs succesfully
*   1 if the sender throws an error
*/
int main(int argc, char **argv) {
  if (argc != 4) {
    std::cerr << "Usage: ./sender [server_address] [port] [username]\n";
    return 1;
  }

  std::string room = "";

  Connection connection;
  if(!(connection.connect(argv[1], std::stoi(argv[2])))) {
    std::cerr << "Failed to connect to server" << std::endl;
    return 1;
  }

  // handling sender login
  Message slogin_msg(TAG_SLOGIN, argv[3]);
  if(!connection.send(slogin_msg)) {
    std::cerr << "Failed to send slogin request" << std::endl;
    connection.close();
    return 1;
  }

  // check the server's response
  if (!handleResponse(connection)) {
    connection.close();
    return 1;
  }

  // only proceeds here if okay signal sent!

  // loop reading commands from user, sending messages to server as appropriate
  while (1){
    std::string message;
    std::getline(std::cin, message);
    std::stringstream ss(message);
    std::string tag;
    ss >> tag;

    // COMMAND
    if (!tag.empty() && tag[0] == '/') {
      handleCommand(tag, room, connection, ss);
    } else if (!tag.empty()) {
      //MESSAGE
      handleMessage(message, connection);
    }
      
    bool isError = handleResponse(connection);

  }
  return 0;
}