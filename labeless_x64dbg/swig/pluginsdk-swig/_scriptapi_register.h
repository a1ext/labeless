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
            CAX,
            CBX,
            CCX,
            CDX,
            CDI,
            CSI,
            CBP,
            CFLAGS
        }; //RegisterEnum

        %rename(Register_Get) Get;
        extern duint Get(RegisterEnum reg);

        %rename(Register_Set) Set;
        extern bool Set(RegisterEnum reg, duint value);

        %rename(Register_Size) Size;
        extern int Size(); //gets architecture register size in bytes

        
        %rename(Register_GetDR0) GetDR0;
        extern duint GetDR0();
        
        %rename(Register_SetDR0) SetDR0;
        extern bool SetDR0(duint value);
        
        %rename(Register_GetDR1) GetDR1;
        extern duint GetDR1();
        
        %rename(Register_SetDR1) SetDR1;
        extern bool SetDR1(duint value);
        
        %rename(Register_GetDR2) GetDR2;
        extern duint GetDR2();
        
        %rename(Register_SetDR2) SetDR2;
        extern bool SetDR2(duint value);
        
        %rename(Register_GetDR3) GetDR3;
        extern duint GetDR3();
        
        %rename(Register_SetDR3) SetDR3;
        extern bool SetDR3(duint value);
        
        %rename(Register_GetDR6) GetDR6;
        extern duint GetDR6();
        
        %rename(Register_SetDR6) SetDR6;
        extern bool SetDR6(duint value);
        
        %rename(Register_GetDR7) GetDR7;
        extern duint GetDR7();
        
        %rename(Register_SetDR7) SetDR7;
        extern bool SetDR7(duint value);

        
        %rename(Register_GetEAX) GetEAX;
        extern unsigned int GetEAX();
        
        %rename(Register_SetEAX) SetEAX;
        extern bool SetEAX(unsigned int value);
        
        %rename(Register_GetAX) GetAX;
        extern unsigned short GetAX();
        
        %rename(Register_SetAX) SetAX;
        extern bool SetAX(unsigned short value);
        
        %rename(Register_GetAH) GetAH;
        extern unsigned char GetAH();
        
        %rename(Register_SetAH) SetAH;
        extern bool SetAH(unsigned char value);
        
        %rename(Register_GetAL) GetAL;
        extern unsigned char GetAL();
        
        %rename(Register_SetAL) SetAL;
        extern bool SetAL(unsigned char value);
        
        %rename(Register_GetEBX) GetEBX;
        extern unsigned int GetEBX();
        
        %rename(Register_SetEBX) SetEBX;
        extern bool SetEBX(unsigned int value);
        
        %rename(Register_GetBX) GetBX;
        extern unsigned short GetBX();
        
        %rename(Register_SetBX) SetBX;
        extern bool SetBX(unsigned short value);
        
        %rename(Register_GetBH) GetBH;
        extern unsigned char GetBH();
        
        %rename(Register_SetBH) SetBH;
        extern bool SetBH(unsigned char value);
        
        %rename(Register_GetBL) GetBL;
        extern unsigned char GetBL();
        
        %rename(Register_SetBL) SetBL;
        extern bool SetBL(unsigned char value);
        
        %rename(Register_GetECX) GetECX;
        extern unsigned int GetECX();
        
        %rename(Register_SetECX) SetECX;
        extern bool SetECX(unsigned int value);
        
        %rename(Register_GetCX) GetCX;
        extern unsigned short GetCX();
        
        %rename(Register_SetCX) SetCX;
        extern bool SetCX(unsigned short value);
        
        %rename(Register_GetCH) GetCH;
        extern unsigned char GetCH();
        
        %rename(Register_SetCH) SetCH;
        extern bool SetCH(unsigned char value);
        
        %rename(Register_GetCL) GetCL;
        extern unsigned char GetCL();
        
        %rename(Register_SetCL) SetCL;
        extern bool SetCL(unsigned char value);
        
        %rename(Register_GetEDX) GetEDX;
        extern unsigned int GetEDX();
        
        %rename(Register_SetEDX) SetEDX;
        extern bool SetEDX(unsigned int value);
        
        %rename(Register_GetDX) GetDX;        
        extern unsigned short GetDX();

        %rename(Register_SetDX) SetDX;
        extern bool SetDX(unsigned short value);

        %rename(Register_GetDH) GetDH;
        extern unsigned char GetDH();
        %rename(Register_SetDH) SetDH;
        extern bool SetDH(unsigned char value);

        %rename(Register_GetDL) GetDL;
        extern unsigned char GetDL();

        %rename(Register_SetDL) SetDL;
        extern bool SetDL(unsigned char value);

        %rename(Register_GetEDI) GetEDI;
        extern unsigned int GetEDI();

        %rename(Register_SetEDI) SetEDI;
        extern bool SetEDI(unsigned int value);

        %rename(Register_GetDI) GetDI;
        extern unsigned short GetDI();

        %rename(Register_SetDI) SetDI;
        extern bool SetDI(unsigned short value);

        %rename(Register_GetESI) GetESI;
        extern unsigned int GetESI();

        %rename(Register_SetESI) SetESI;
        extern bool SetESI(unsigned int value);

        %rename(Register_GetSI) GetSI;
        extern unsigned short GetSI();

        %rename(Register_SetSI) SetSI;
        extern bool SetSI(unsigned short value);

        %rename(Register_GetEBP) GetEBP;
        extern unsigned int GetEBP();

        %rename(Register_SetEBP) SetEBP;
        extern bool SetEBP(unsigned int value);

        %rename(Register_GetBP) GetBP;
        extern unsigned short GetBP();

        %rename(Register_SetBP) SetBP;
        extern bool SetBP(unsigned short value);

        %rename(Register_GetESP) GetESP;
        extern unsigned int GetESP();

        %rename(Register_SetESP) SetESP;
        extern bool SetESP(unsigned int value);

        %rename(Register_GetSP) GetSP;
        extern unsigned short GetSP();

        %rename(Register_SetSP) SetSP;
        extern bool SetSP(unsigned short value);

        %rename(Register_GetEIP) GetEIP;
        extern unsigned int GetEIP();

        %rename(Register_SetEIP) SetEIP;
        extern bool SetEIP(unsigned int value);

#ifdef _WIN64
        
        %rename(Register_GetRAX) GetRAX;
        extern unsigned long long GetRAX();

        %rename(Register_SetRAX) SetRAX;
        extern bool SetRAX(unsigned long long value);

        %rename(Register_GetRBX) GetRBX;
        extern unsigned long long GetRBX();

        %rename(Register_SetRBX) SetRBX;
        extern bool SetRBX(unsigned long long value);

        %rename(Register_GetRCX) GetRCX;
        extern unsigned long long GetRCX();

        %rename(Register_SetRCX) SetRCX;
        extern bool SetRCX(unsigned long long value);

        %rename(Register_GetRDX) GetRDX;
        extern unsigned long long GetRDX();

        %rename(Register_SetRDX) SetRDX;
        extern bool SetRDX(unsigned long long value);

        %rename(Register_GetRSI) GetRSI;
        extern unsigned long long GetRSI();

        %rename(Register_SetRSI) SetRSI;
        extern bool SetRSI(unsigned long long value);

        %rename(Register_GetSIL) GetSIL;
        extern unsigned char GetSIL();

        %rename(Register_SetSIL) SetSIL;
        extern bool SetSIL(unsigned char value);

        %rename(Register_GetRDI) GetRDI;
        extern unsigned long long GetRDI();

        %rename(Register_SetRDI) SetRDI;
        extern bool SetRDI(unsigned long long value);

        %rename(Register_GetDIL) GetDIL;
        extern unsigned char GetDIL();

        %rename(Register_SetDIL) SetDIL;
        extern bool SetDIL(unsigned char value);

        %rename(Register_GetRBP) GetRBP;
        extern unsigned long long GetRBP();

        %rename(Register_SetRBP) SetRBP;
        extern bool SetRBP(unsigned long long value);

        %rename(Register_GetBPL) GetBPL;
        extern unsigned char GetBPL();

        %rename(Register_SetBPL) SetBPL;
        extern bool SetBPL(unsigned char value);

        %rename(Register_GetRSP) GetRSP;
        extern unsigned long long GetRSP();

        %rename(Register_SetRSP) SetRSP;
        extern bool SetRSP(unsigned long long value);

        %rename(Register_GetSPL) GetSPL;
        extern unsigned char GetSPL();

        %rename(Register_SetSPL) SetSPL;
        extern bool SetSPL(unsigned char value);

        %rename(Register_GetRIP) GetRIP;
        extern unsigned long long GetRIP();

        %rename(Register_SetRIP) SetRIP;
        extern bool SetRIP(unsigned long long value);

        %rename(Register_GetR8) GetR8;
        extern unsigned long long GetR8();

        %rename(Register_SetR8) SetR8;
        extern bool SetR8(unsigned long long value);

        %rename(Register_GetR8D) GetR8D;
        extern unsigned int GetR8D();

        %rename(Register_SetR8D) SetR8D;
        extern bool SetR8D(unsigned int value);

        %rename(Register_GetR8W) GetR8W;
        extern unsigned short GetR8W();

        %rename(Register_SetR8W) SetR8W;
        extern bool SetR8W(unsigned short value);

        %rename(Register_GetR8B) GetR8B;
        extern unsigned char GetR8B();

        %rename(Register_SetR8B) SetR8B;
        extern bool SetR8B(unsigned char value);

        %rename(Register_GetR9) GetR9;
        extern unsigned long long GetR9();

        %rename(Register_SetR9) SetR9;
        extern bool SetR9(unsigned long long value);

        %rename(Register_GetR9D) GetR9D;
        extern unsigned int GetR9D();

        %rename(Register_SetR9D) SetR9D;
        extern bool SetR9D(unsigned int value);

        %rename(Register_GetR9W) GetR9W;
        extern unsigned short GetR9W();

        %rename(Register_SetR9W) SetR9W;
        extern bool SetR9W(unsigned short value);

        %rename(Register_GetR9B) GetR9B;
        extern unsigned char GetR9B();

        %rename(Register_SetR9B) SetR9B;
        extern bool SetR9B(unsigned char value);

        %rename(Register_GetR10) GetR10;
        extern unsigned long long GetR10();

        %rename(Register_SetR10) SetR10;
        extern bool SetR10(unsigned long long value);

        %rename(Register_GetR10D) GetR10D;
        extern unsigned int GetR10D();

        %rename(Register_SetR10D) SetR10D;
        extern bool SetR10D(unsigned int value);

        %rename(Register_GetR10W) GetR10W;
        extern unsigned short GetR10W();

        %rename(Register_SetR10W) SetR10W;
        extern bool SetR10W(unsigned short value);

        %rename(Register_GetR10B) GetR10B;
        extern unsigned char GetR10B();

        %rename(Register_SetR10B) SetR10B;
        extern bool SetR10B(unsigned char value);

        %rename(Register_GetR11) GetR11;
        extern unsigned long long GetR11();

        %rename(Register_SetR11) SetR11;
        extern bool SetR11(unsigned long long value);

        %rename(Register_GetR11D) GetR11D;
        extern unsigned int GetR11D();

        %rename(Register_SetR11D) SetR11D;
        extern bool SetR11D(unsigned int value);

        %rename(Register_GetR11W) GetR11W;
        extern unsigned short GetR11W();

        %rename(Register_SetR11W) SetR11W;
        extern bool SetR11W(unsigned short value);

        %rename(Register_GetR11B) GetR11B;
        extern unsigned char GetR11B();

        %rename(Register_SetR11B) SetR11B;
        extern bool SetR11B(unsigned char value);

        %rename(Register_GetR12) GetR12;
        extern unsigned long long GetR12();

        %rename(Register_SetR12) SetR12;
        extern bool SetR12(unsigned long long value);

        %rename(Register_GetR12D) GetR12D;
        extern unsigned int GetR12D();

        %rename(Register_SetR12D) SetR12D;
        extern bool SetR12D(unsigned int value);

        %rename(Register_GetR12W) GetR12W;
        extern unsigned short GetR12W();

        %rename(Register_SetR12W) SetR12W;
        extern bool SetR12W(unsigned short value);

        %rename(Register_GetR12B) GetR12B;
        extern unsigned char GetR12B();

        %rename(Register_SetR12B) SetR12B;
        extern bool SetR12B(unsigned char value);

        %rename(Register_GetR13) GetR13;
        extern unsigned long long GetR13();

        %rename(Register_SetR13) SetR13;
        extern bool SetR13(unsigned long long value);

        %rename(Register_GetR13D) GetR13D;
        extern unsigned int GetR13D();

        %rename(Register_SetR13D) SetR13D;
        extern bool SetR13D(unsigned int value);

        %rename(Register_GetR13W) GetR13W;
        extern unsigned short GetR13W();

        %rename(Register_SetR13W) SetR13W;
        extern bool SetR13W(unsigned short value);

        %rename(Register_GetR13B) GetR13B;
        extern unsigned char GetR13B();

        %rename(Register_SetR13B) SetR13B;
        extern bool SetR13B(unsigned char value);

        %rename(Register_GetR14) GetR14;
        extern unsigned long long GetR14();

        %rename(Register_SetR14) SetR14;
        extern bool SetR14(unsigned long long value);

        %rename(Register_GetR14D) GetR14D;
        extern unsigned int GetR14D();

        %rename(Register_SetR14D) SetR14D;
        extern bool SetR14D(unsigned int value);

        %rename(Register_GetR14W) GetR14W;
        extern unsigned short GetR14W();

        %rename(Register_SetR14W) SetR14W;
        extern bool SetR14W(unsigned short value);

        %rename(Register_GetR14B) GetR14B;
        extern unsigned char GetR14B();

        %rename(Register_SetR14B) SetR14B;
        extern bool SetR14B(unsigned char value);

        %rename(Register_GetR15) GetR15;
        extern unsigned long long GetR15();

        %rename(Register_SetR15) SetR15;
        extern bool SetR15(unsigned long long value);

        %rename(Register_GetR15D) GetR15D;
        extern unsigned int GetR15D();

        %rename(Register_SetR15D) SetR15D;
        extern bool SetR15D(unsigned int value);

        %rename(Register_GetR15W) GetR15W;
        extern unsigned short GetR15W();

        %rename(Register_SetR15W) SetR15W;
        extern bool SetR15W(unsigned short value);

        %rename(Register_GetR15B) GetR15B;
        extern unsigned char GetR15B();

        %rename(Register_SetR15B) SetR15B;
        extern bool SetR15B(unsigned char value);
#endif //_WIN64

        %rename(Register_GetCAX) GetCAX;
        extern duint GetCAX();
        
        %rename(Register_SetCAX) SetCAX;
        extern bool SetCAX(duint value);
        
        %rename(Register_GetCBX) GetCBX;
        extern duint GetCBX();
        
        %rename(Register_SetCBX) SetCBX;
        extern bool SetCBX(duint value);
        
        %rename(Register_GetCCX) GetCCX;
        extern duint GetCCX();
        
        %rename(Register_SetCCX) SetCCX;
        extern bool SetCCX(duint value);
        
        %rename(Register_GetCDX) GetCDX;
        extern duint GetCDX();
        
        %rename(Register_SetCDX) SetCDX;
        extern bool SetCDX(duint value);
        
        %rename(Register_GetCDI) GetCDI;
        extern duint GetCDI();
        
        %rename(Register_SetCDI) SetCDI;
        extern bool SetCDI(duint value);
        
        %rename(Register_GetCSI) GetCSI;
        extern duint GetCSI();
        
        %rename(Register_SetCSI) SetCSI;
        extern bool SetCSI(duint value);
        
        %rename(Register_GetCBP) GetCBP;
        extern duint GetCBP();
        
        %rename(Register_SetCBP) SetCBP;
        extern bool SetCBP(duint value);
        
        %rename(Register_GetCSP) GetCSP;
        extern duint GetCSP();
        
        %rename(Register_SetCSP) SetCSP;
        extern bool SetCSP(duint value);
        
        %rename(Register_GetCIP) GetCIP;
        extern duint GetCIP();
        
        %rename(Register_SetCIP) SetCIP;
        extern bool SetCIP(duint value);
        
        %rename(Register_GetCFLAGS) GetCFLAGS;
        extern duint GetCFLAGS();
        
        %rename(Register_SetCFLAGS) SetCFLAGS;
        extern bool SetCFLAGS(duint value);
    }; //Register
}; //Script

#endif //_SCRIPTAPI_REGISTER_H