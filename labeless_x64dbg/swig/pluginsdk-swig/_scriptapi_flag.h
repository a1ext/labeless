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

        %rename(Flag_Get) Get;
        extern bool Get(FlagEnum flag);
        
        %rename(Flag_Set) Set;
        extern bool Set(FlagEnum flag, bool value);

        %rename(Flag_GetZF) GetZF;
        extern bool GetZF();

        %rename(Flag_SetZF) SetZF;
        extern bool SetZF(bool value);

        %rename(Flag_GetOF) GetOF;
        extern bool GetOF();

        %rename(Flag_SetOF) SetOF;
        extern bool SetOF(bool value);

        %rename(Flag_GetCF) GetCF;
        extern bool GetCF();

        %rename(Flag_SetCF) SetCF;
        extern bool SetCF(bool value);

        %rename(Flag_GetPF) GetPF;
        extern bool GetPF();
        
        %rename(Flag_SetPF) SetPF;
        extern bool SetPF(bool value);
        
        %rename(Flag_GetSF) GetSF;
        extern bool GetSF();
        
        %rename(Flag_SetSF) SetSF;
        extern bool SetSF(bool value);
        
        %rename(Flag_GetTF) GetTF;
        extern bool GetTF();
        
        %rename(Flag_SetTF) SetTF;
        extern bool SetTF(bool value);
        
        %rename(Flag_GetAF) GetAF;
        extern bool GetAF();
        
        %rename(Flag_SetAF) SetAF;
        extern bool SetAF(bool value);
        
        %rename(Flag_GetDF) GetDF;
        extern bool GetDF();
        
        %rename(Flag_SetDF) SetDF;
        extern bool SetDF(bool value);
        
        %rename(Flag_GetIF) GetIF;
        extern bool GetIF();
        
        %rename(Flag_SetIF) SetIF;
        extern bool SetIF(bool value);
    };
};

#endif //_SCRIPTAPI_FLAG_H