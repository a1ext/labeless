#ifndef _SCRIPTAPI_FLAG_H
#define _SCRIPTAPI_FLAG_H

#include "_scriptapi.h"

namespace Script
{
    namespace Flag
    {
        enum FlagEnum
        {
            ZF,
            OF,
            CF,
            PF,
            SF,
            TF,
            AF,
            DF,
            IF
        };

        extern bool Get(FlagEnum flag);
        extern bool Set(FlagEnum flag, bool value);

        extern bool GetZF();
        extern bool SetZF(bool value);
        extern bool GetOF();
        extern bool SetOF(bool value);
        extern bool GetCF();
        extern bool SetCF(bool value);
        extern bool GetPF();
        extern bool SetPF(bool value);
        extern bool GetSF();
        extern bool SetSF(bool value);
        extern bool GetTF();
        extern bool SetTF(bool value);
        extern bool GetAF();
        extern bool SetAF(bool value);
        extern bool GetDF();
        extern bool SetDF(bool value);
        extern bool GetIF();
        extern bool SetIF(bool value);
    };
};

#endif //_SCRIPTAPI_FLAG_H