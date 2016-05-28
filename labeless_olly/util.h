/* Labeless
 * by Aliaksandr Trafimchuk
 *
 * Source code released under
 * Creative Commons BY-NC 4.0
 * http://creativecommons.org/licenses/by-nc/4.0
 */

#pragma once

#include <deque>
#include <string>

#include "types.h"

struct sockaddr_in;

namespace util {

std::string getOllyDir();
std::string getPluginDir(HINSTANCE hPlugin);
std::string getErrorDir();
std::string inetAddrToString(sockaddr_in* sin);

std::string sformat(const char* fmt, ...);

std::deque<std::string> split(const std::string& s, const std::string& delimitersSet = "\r|\n");

} // util
