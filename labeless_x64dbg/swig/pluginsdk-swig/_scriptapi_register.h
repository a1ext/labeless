#ifndef _SCRIPTAPI_REGISTER_H
#define _SCRIPTAPI_REGISTER_H

#include "_scriptapi.h"

namespace Script
{
    namespace Register
    {
        enum RegisterEnum
        {
            DR0,
            DR1,
            DR2,
            DR3,
            DR6,
            DR7,

            EAX,
            AX,
            AH,
            AL,
            EBX,
            BX,
            BH,
            BL,
            ECX,
            CX,
            CH,
            CL,
            EDX,
            DX,
            DH,
            DL,
            EDI,
            DI,
            ESI,
            SI,
            EBP,
            BP,
            ESP,
            SP,
            EIP,

#ifdef _WIN64
            RAX,
            RBX,
            RCX,
            RDX,
            RSI,
            SIL,
            RDI,
            DIL,
            RBP,
            BPL,
            RSP,
            SPL,
            RIP,
            R8,
            R8D,
            R8W,
            R8B,
            R9,
            R9D,
            R9W,
            R9B,
            R10,
            R10D,
            R10W,
            R10B,
            R11,
            R11D,
            R11W,
            R11B,
            R12,
            R12D,
            R12W,
            R12B,
            R13,
            R13D,
            R13W,
            R13B,
            R14,
            R14D,
            R14W,
            R14B,
            R15,
            R15D,
            R15W,
            R15B,
#endif //_WIN64

            CIP,
            CSP,
        }; //RegisterEnum

        extern duint Get(RegisterEnum reg);
        extern bool Set(RegisterEnum reg, duint value);
        extern int Size(); //gets architecture register size in bytes

        extern duint GetDR0();
        extern bool SetDR0(duint value);
        extern duint GetDR1();
        extern bool SetDR1(duint value);
        extern duint GetDR2();
        extern bool SetDR2(duint value);
        extern duint GetDR3();
        extern bool SetDR3(duint value);
        extern duint GetDR6();
        extern bool SetDR6(duint value);
        extern duint GetDR7();
        extern bool SetDR7(duint value);

        extern unsigned int GetEAX();
        extern bool SetEAX(unsigned int value);
        extern unsigned short GetAX();
        extern bool SetAX(unsigned short value);
        extern unsigned char GetAH();
        extern bool SetAH(unsigned char value);
        extern unsigned char GetAL();
        extern bool SetAL(unsigned char value);
        extern unsigned int GetEBX();
        extern bool SetEBX(unsigned int value);
        extern unsigned short GetBX();
        extern bool SetBX(unsigned short value);
        extern unsigned char GetBH();
        extern bool SetBH(unsigned char value);
        extern unsigned char GetBL();
        extern bool SetBL(unsigned char value);
        extern unsigned int GetECX();
        extern bool SetECX(unsigned int value);
        extern unsigned short GetCX();
        extern bool SetCX(unsigned short value);
        extern unsigned char GetCH();
        extern bool SetCH(unsigned char value);
        extern unsigned char GetCL();
        extern bool SetCL(unsigned char value);
        extern unsigned int GetEDX();
        extern bool SetEDX(unsigned int value);
        extern unsigned short GetDX();
        extern bool SetDX(unsigned short value);
        extern unsigned char GetDH();
        extern bool SetDH(unsigned char value);
        extern unsigned char GetDL();
        extern bool SetDL(unsigned char value);
        extern unsigned int GetEDI();
        extern bool SetEDI(unsigned int value);
        extern unsigned short GetDI();
        extern bool SetDI(unsigned short value);
        extern unsigned int GetESI();
        extern bool SetESI(unsigned int value);
        extern unsigned short GetSI();
        extern bool SetSI(unsigned short value);
        extern unsigned int GetEBP();
        extern bool SetEBP(unsigned int value);
        extern unsigned short GetBP();
        extern bool SetBP(unsigned short value);
        extern unsigned int GetESP();
        extern bool SetESP(unsigned int value);
        extern unsigned short GetSP();
        extern bool SetSP(unsigned short value);
        extern unsigned int GetEIP();
        extern bool SetEIP(unsigned int value);

#ifdef _WIN64
        extern unsigned long long GetRAX();
        extern bool SetRAX(unsigned long long value);
        extern unsigned long long GetRBX();
        extern bool SetRBX(unsigned long long value);
        extern unsigned long long GetRCX();
        extern bool SetRCX(unsigned long long value);
        extern unsigned long long GetRDX();
        extern bool SetRDX(unsigned long long value);
        extern unsigned long long GetRSI();
        extern bool SetRSI(unsigned long long value);
        extern unsigned char GetSIL();
        extern bool SetSIL(unsigned char value);
        extern unsigned long long GetRDI();
        extern bool SetRDI(unsigned long long value);
        extern unsigned char GetDIL();
        extern bool SetDIL(unsigned char value);
        extern unsigned long long GetRBP();
        extern bool SetRBP(unsigned long long value);
        extern unsigned char GetBPL();
        extern bool SetBPL(unsigned char value);
        extern unsigned long long GetRSP();
        extern bool SetRSP(unsigned long long value);
        extern unsigned char GetSPL();
        extern bool SetSPL(unsigned char value);
        extern unsigned long long GetRIP();
        extern bool SetRIP(unsigned long long value);
        extern unsigned long long GetR8();
        extern bool SetR8(unsigned long long value);
        extern unsigned int GetR8D();
        extern bool SetR8D(unsigned int value);
        extern unsigned short GetR8W();
        extern bool SetR8W(unsigned short value);
        extern unsigned char GetR8B();
        extern bool SetR8B(unsigned char value);
        extern unsigned long long GetR9();
        extern bool SetR9(unsigned long long value);
        extern unsigned int GetR9D();
        extern bool SetR9D(unsigned int value);
        extern unsigned short GetR9W();
        extern bool SetR9W(unsigned short value);
        extern unsigned char GetR9B();
        extern bool SetR9B(unsigned char value);
        extern unsigned long long GetR10();
        extern bool SetR10(unsigned long long value);
        extern unsigned int GetR10D();
        extern bool SetR10D(unsigned int value);
        extern unsigned short GetR10W();
        extern bool SetR10W(unsigned short value);
        extern unsigned char GetR10B();
        extern bool SetR10B(unsigned char value);
        extern unsigned long long GetR11();
        extern bool SetR11(unsigned long long value);
        extern unsigned int GetR11D();
        extern bool SetR11D(unsigned int value);
        extern unsigned short GetR11W();
        extern bool SetR11W(unsigned short value);
        extern unsigned char GetR11B();
        extern bool SetR11B(unsigned char value);
        extern unsigned long long GetR12();
        extern bool SetR12(unsigned long long value);
        extern unsigned int GetR12D();
        extern bool SetR12D(unsigned int value);
        extern unsigned short GetR12W();
        extern bool SetR12W(unsigned short value);
        extern unsigned char GetR12B();
        extern bool SetR12B(unsigned char value);
        extern unsigned long long GetR13();
        extern bool SetR13(unsigned long long value);
        extern unsigned int GetR13D();
        extern bool SetR13D(unsigned int value);
        extern unsigned short GetR13W();
        extern bool SetR13W(unsigned short value);
        extern unsigned char GetR13B();
        extern bool SetR13B(unsigned char value);
        extern unsigned long long GetR14();
        extern bool SetR14(unsigned long long value);
        extern unsigned int GetR14D();
        extern bool SetR14D(unsigned int value);
        extern unsigned short GetR14W();
        extern bool SetR14W(unsigned short value);
        extern unsigned char GetR14B();
        extern bool SetR14B(unsigned char value);
        extern unsigned long long GetR15();
        extern bool SetR15(unsigned long long value);
        extern unsigned int GetR15D();
        extern bool SetR15D(unsigned int value);
        extern unsigned short GetR15W();
        extern bool SetR15W(unsigned short value);
        extern unsigned char GetR15B();
        extern bool SetR15B(unsigned char value);
#endif //_WIN64

        extern duint GetCIP();
        extern bool SetCIP(duint value);
        extern duint GetCSP();
        extern bool SetCSP(duint value);
    }; //Register
}; //Script

#endif //_SCRIPTAPI_REGISTER_H