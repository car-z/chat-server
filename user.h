/*
 * Class describing a User object.
 * CSF Assignment 5
 * Caroline Zhao
 * czhao67@jhu.edu
 * Miranda Qing
 * mqing2@jhu.edu
 */

#ifndef USER_H
#define USER_H

#include <string>
#include "message_queue.h"

struct User {
  std::string username;
  std::string room;

  // queue of pending messages awaiting delivery
  MessageQueue mqueue;

  User(const std::string &username) : username(username) { }
};

#endif // USER_H
