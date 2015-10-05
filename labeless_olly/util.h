/* Labeless
 * by Aliaksandr Trafimchuk
 *
 * Source code released under
 * Creative Commons BY-NC 4.0
 * http://creativecommons.org/licenses/by-nc/4.0
 */

#pragma once

#include <string>

#include "types.h"

struct sockaddr_in;

namespace util {

std::string getOllyDir();
std::string getPluginDir(HINSTANCE hPlugin);
std::string inetAddrToString(sockaddr_in* sin);

} // util
