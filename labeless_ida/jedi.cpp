/* Labeless
* by Aliaksandr Trafimchuk
*
* Source code released under
* Creative Commons BY-NC 4.0
* http://creativecommons.org/licenses/by-nc/4.0
*/

#include "jedi.h"
#include "types.h"


namespace jedi {

State::State()
	: state(RS_DONE)
{
}

Request::Request()
	: zline(0)
	, zcol(0)
	, rcv(nullptr)
	, comp_type(CT_Unknown)
{
}

SignatureMatch::SignatureMatch()
	: argIndex(-1)
{
}

} // jedi
