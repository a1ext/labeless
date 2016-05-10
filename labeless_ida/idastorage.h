/* Labeless
 * by Aliaksandr Trafimchuk
 *
 * Source code released under
 * Creative Commons BY-NC 4.0
 * http://creativecommons.org/licenses/by-nc/4.0
 */

#pragma once

struct ExternSegData;
struct Settings;

namespace storage {


bool loadExternSegData(ExternSegData& result);
bool storeExternSegData(const ExternSegData& value);

bool loadDbSettings(Settings& result);
bool storeDbSettings(const Settings& result);


} // storage
