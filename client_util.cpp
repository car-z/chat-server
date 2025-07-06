/*
 * Implementatino of string utility functions for the client.
 * CSF Assignment 5
 * Caroline Zhao
 * czhao67@jhu.edu
 * Miranda Qing
 * mqing2@jhu.edu
 */

#include <iostream>
#include <string>
#include "connection.h"
#include "message.h"
#include "client_util.h"

// string trim functions shamelessly stolen from
// https://www.techiedelight.com/trim-string-cpp-remove-leading-trailing-spaces/

const std::string WHITESPACE = " \n\r\t\f\v";

/*
 * Removes leading whitespace in a string, if there is any.
 *
 * Parameters:
 *   s - reference to a string
 *
 * Returns:
 *   a string with the leading whitespace removed. 
 *   if the string is all whitespace, then an empty string is
 *   returned.
 */
std::string ltrim(const std::string &s) {
  size_t start = s.find_first_not_of(WHITESPACE);
  return (start == std::string::npos) ? "" : s.substr(start);
}
 
/*
 * Removes trailing whitespace in a string, if there is any.
 *
 * Parameters:
 *   s - reference to a string
 *
 * Returns:
 *   a string with the trailing whitespace removed. 
 *   if the string is all whitespace, then an empty string is
 *   returned.
 */
std::string rtrim(const std::string &s) {
  size_t end = s.find_last_not_of(WHITESPACE);
  return (end == std::string::npos) ? "" : s.substr(0, end + 1);
}
 
/*
 * Removes leading and trailing whitespace in a string, if there is any.
 *
 * Parameters:
 *   s - reference to a string
 *
 * Returns:
 *   a string with the leading and trailing whitespace removed. 
 *   if the string is all whitespace, then an empty string is
 *   returned.
 */
std::string trim(const std::string &s) {
  return rtrim(ltrim(s));
}
