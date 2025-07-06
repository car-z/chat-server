/*
 * Class representing connection between client and server.
 * CSF Assignment 5
 * Caroline Zhao
 * czhao67@jhu.edu
 * Miranda Qing
 * mqing2@jhu.edu
 */

#include <sstream>
#include <cctype>
#include <cassert>
#include "csapp.h"
#include "message.h"
#include "connection.h"
#include <iostream>
#include "client_util.h"

const std::unordered_set<std::string> tags = {
    TAG_ERR, TAG_OK, TAG_SLOGIN, TAG_RLOGIN, TAG_JOIN, TAG_LEAVE, TAG_SENDALL, TAG_SENDUSER, TAG_QUIT, TAG_DELIVERY, TAG_EMPTY
};

  /*
  * Checks if the provided string of a Message is a valid Message.
  *
  * Parameters:
  *   message - a reference to a string representing the tag and data of a Message (in tag:data format)
  * 
  * Returns:
  *   true if this Message is a valid message
  */
  bool validMessage(std::string& message) {
    size_t length = message.size();

    // message must not be more than max_len bytes (one char = one byte)
    if (length > Message::MAX_LEN) {
      return false;
    }

    // message must end at new line delimiter
    bool hasRF = length >= 2 && message[length - 2] == '\r' && message[length - 1] == '\n';
    bool hasNL = length >= 1 && message[length - 1] == '\n';
    if (!hasRF && !hasNL) {
      return false;
    }

    // message must be single line of text with no new line delimiters contained within
    std::string msgShort;
    if (hasRF) {
      msgShort = message.substr(0, length-2);
    } else {
      msgShort = message.substr(0, length-1);
    }
    if (msgShort.find('\n') != std::string::npos || msgShort.find('\r') != std::string::npos) {
      return false;
    }

    // message must have colon separator for tag and payload
    size_t indexColon = msgShort.find(':');
    if (indexColon == std::string::npos) {
      return false;
    }

    std::string tag = msgShort.substr(0,indexColon);
    if (tags.find(tag) == tags.end()) {
      return false;
    }

    return true;
  }

/*
 * Default constructor for Connection object. 
 *
 * Returns:
 *   a new instance of a Connection object
 *   with the file descriptor 3set to -1
 *   and the last result set to SUCCESS.
 */
Connection::Connection()
  : m_fd(-1)
  , m_last_result(SUCCESS) {
}

/*
 * Non-Default constructor for Connection object. 
 *
 * Parameters:
 *   fd - open file descriptor (an integer)
 * 
 * Returns:
 *   a new instance of a Connection object
 *   with the file descriptor set to fd
 *   and the last result set to SUCCESS.
 */
Connection::Connection(int fd)
  : m_fd(fd)
  , m_last_result(SUCCESS) {
  rio_readinitb(&m_fdbuf, m_fd);  
}

/*
 * Connect to a server via specified hostname and port number.
 *
 * Parameters:
 *   hostname - reference to the hostname
 *   port - integer representing the port number
 *
 * Returns:
 *    true if succesfully opened connection, false otherwise
 */
bool Connection::connect(const std::string &hostname, int port) {
  std::string port_str = std::to_string(port);
  m_fd = open_clientfd(hostname.c_str(), port_str.c_str());
  if (m_fd <= 0) {
    std::cerr << "Failed to connect to server" << std::endl;
    return false;
  }
  rio_readinitb(&m_fdbuf, m_fd);
  return true;
}

/*
 * Destructor for a Connection object.
 * Insures that the file descriptor is closed.
 */
Connection::~Connection() {
  if (is_open()) {
    close();
  }
}

/*
 * Returns true if the connection is open.
 *
 * Returns:
 *   true if the connection is open,
 *   false if the connection is closed.
 */
bool Connection::is_open() const {
  return m_fd > 0;
}

/*
 * Closes the connection, if open.
 */
void Connection::close() {
  bool open = is_open();
  if (open) {
    Close(m_fd);
  }
  m_fd = -1;
}

/*
 * Function to facilitate sending a message across the connection
 * and sets m_last_result appropriately.
 *
 * Parameters:
 *   msg - reference to the Message object being sent.
 * 
 * Returns:
 *   true if message was succesfully sent
 */
bool Connection::send(Message &msg) {
  if (!is_open()) {
    m_last_result = EOF_OR_ERROR;
    return false;
  }
  ssize_t result = rio_writen(m_fd, msg.strMessage().c_str(), msg.sizeMessage());
  if ( result != static_cast<ssize_t>(msg.sizeMessage())) {
    m_last_result = EOF_OR_ERROR;
    return false;
  }
  result = rio_writen(m_fd, "\n", 1);
  if (result != 1) {
    m_last_result = EOF_OR_ERROR;
    return false;
  }
  m_last_result = SUCCESS;
  return true;
}

/*
 * Function to facilitate receiving a message across the connection
 * and sets m_last_result appropriately.
 *
 * Parameters:
 *   msg - reference to the Message object to store received tag and data.
 * 
 * Returns:
 *   true if message was succesfully received
 */
bool Connection::receive(Message &msg) {
  char buf[1000];
  ssize_t n = rio_readlineb(&m_fdbuf, buf, sizeof(buf));
  if (n <= 0) {
    m_last_result = EOF_OR_ERROR;
    return false;
  }
  buf[n] = '\0';
  std::string response(buf);
  if (!validMessage(response)) {
    m_last_result = INVALID_MSG;
    return false;
  }
  size_t index = response.find(':');
  msg.tag = trim(response.substr(0, index));
  msg.data = trim(response.substr(index + 1));

  m_last_result = SUCCESS;
  return true;
}