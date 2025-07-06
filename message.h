/*
 * Struct describing a Message object plus its member functions.
 * CSF Assignment 5
 * Caroline Zhao
 * czhao67@jhu.edu
 * Miranda Qing
 * mqing2@jhu.edu
 */

#ifndef MESSAGE_H
#define MESSAGE_H

#include <vector>
#include <unordered_set>
#include <string>


// standard message tags (note that you don't need to worry about
// "senduser" or "empty" messages)
#define TAG_ERR       "err"       // protocol error
#define TAG_OK        "ok"        // success response
#define TAG_SLOGIN    "slogin"    // register as specific user for sending
#define TAG_RLOGIN    "rlogin"    // register as specific user for receiving
#define TAG_JOIN      "join"      // join a chat room
#define TAG_LEAVE     "leave"     // leave a chat room
#define TAG_SENDALL   "sendall"   // send message to all users in chat room
#define TAG_SENDUSER  "senduser"  // send message to specific user in chat room
#define TAG_QUIT      "quit"      // quit
#define TAG_DELIVERY  "delivery"  // message delivered by server to receiving client
#define TAG_EMPTY     "empty"     // sent by server to receiving client to indicate no msgs available

struct Message {
  // An encoded message may have at most this many characters,
  // including the trailing newline ('\n'). Note that this does
  // *not* include a NUL terminator (if one is needed to
  // temporarily store the encoded message.)
  static const unsigned MAX_LEN = 255;

  std::string tag;
  std::string data;

  
  /*
  * Default constructor for Message struct. 
  *
  * Returns:
  *   a new Message.
  */
  Message() { }

  /*
  * Non-Default constructor for Message struct. 
  *
  * Parameters:
  *   tag - reference to a string holding the message tag
  *   data - reference to a string holding the message payload
  * 
  * Returns:
  *   a new Message with tag and data set to tag and data.
  */
  Message(const std::string &tag, const std::string &data)
    : tag(tag), data(data) { }

  // TODO: you could add helper functions

  /*
  * Creating a string of this Message.
  * The tag and data are combined, separated by a colon.
  * 
  * Returns:
  *   tag:data (a string)
  */
  std::string strMessage() {
    return tag + ":" + data;
  }

  /*
  * Returns the size of this Message (after it is
  * turned into one single string)
  * 
  * Returns:
  *   the size of the tag:data string
  */
  size_t sizeMessage() {
    std::string msg = strMessage();
    return msg.size();
  }

};

#endif // MESSAGE_H
