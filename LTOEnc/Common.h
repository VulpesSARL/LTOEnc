#pragma once

#define SPT_SENSE_BUFFER_LENGTH 64

#define BSINTTOCHAR(x) (unsigned char)((x & 0xff000000)>>24), (unsigned char)((x & 0x00ff0000)>>16),(unsigned char)((x & 0x0000ff00)>>8),(unsigned char)(x & 0x000000ff)
#define BSSHORT(x) ((unsigned short)( (x[0]<<8) + x[1] ))
#define HEX(x) right<<setw(2)<< setfill(L'0') << hex << (int)(x)<<setfill(L' ')
#define StrToLower(a) std::transform(a.begin(), a.end(), a.begin(), [](wchar_t c) { return towlower(c); })


int QuerySCSI(HANDLE hDevice, UCHAR DataIn, UCHAR CdbLength, UCHAR *Cdb, ULONG DataTransferLength, void *DataTransfer, int Timeout, UCHAR SenseOutput[SPT_SENSE_BUFFER_LENGTH], int *SCSIStatus);
string CString(char *buffer, int offset);
wstring GetErrorAsString(DWORD error);

wstring s2ws(const string& str);
string ws2s(const wstring& wstr);

#define DriveEncryption_Unknown 0
#define DriveEncryption_On 1
#define DriveEncryption_Mixed 2
#define DriveEncryption_RawRead 3
#define DriveEncryption_Off 4

#define DriveOutput_Unknown 0
#define DriveOutput_Disabled 1
#define DriveOutput_Raw 2
#define DriveOutput_Decrypt 3
#define DriveOutput_Mixed 4

#define DriveInput_Unknown 0
#define DriveInput_NotEncrypting 1
#define DriveInput_Encrypting 2

#define MediaEncryption_Unknown 0
#define MediaEncryption_NotEncrypted 1
#define MediaEncryption_Encrypted_Can_Decrypt 2
#define MediaEncryption_Encrypted_Cannot_Decrypt 3
#define MediaEncryption_Cannot_Determine 4

