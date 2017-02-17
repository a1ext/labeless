/* Labeless
* by Aliaksandr Trafimchuk
*
* Source code released under
* Creative Commons BY-NC 4.0
* http://creativecommons.org/licenses/by-nc/4.0
*/

#pragma once

#include <string>
#include <QString>

namespace util {
namespace idapython {

bool init();
bool runScript(const std::string& script, std::string& externObj, std::string& error);
bool setResultObject(const std::string& obj, std::string& error);


namespace github {

struct ReleaseInfo
{
	QString tag; // tag_name
	QString name; // name
	QString url; // html_url
};

bool getLatestRelease(ReleaseInfo& ri, std::string& error);

} // github
} // idapython
} // util
