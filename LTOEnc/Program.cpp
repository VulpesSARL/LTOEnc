#include <windows.h>
#include <stdio.h>
#include <ntddtape.h>
#include <string>
#include <algorithm>
#include <vector>
#include <random>
#include <iostream>
#include <sstream>
#include <fstream>
#include <iomanip>

#ifdef _DEBUG
#include <crtdbg.h>
#endif

using namespace std;

#include "IBMTAPEIOCTL.h"
#include "HPTAPEIOCTL.h"
#include "Common.h"
#include "Encryption.h"
#include "Keyfile.h"

int ShowvolumeStatus(HANDLE hDevice, bool WithRewind);
int ShowDriveStatus(HANDLE hDevice);

int wmain(int argc, wchar_t *argv[])
{
	wprintf(L"Vulpes LTOEnc - LTO Tape Encryption setup\n   based on STENC (https://github.com/scsitape/stenc)\n\nVulpes SARL, Luxembourg - https://vulpes.lu\n\n");

#ifdef _DEBUG
	wprintf(L"*** Debug Mode enabled! ***\n");
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF | _CRTDBG_CHECK_CRT_DF | _CRTDBG_DELAY_FREE_MEM_DF | _CRTDBG_CHECK_ALWAYS_DF | _CRTDBG_CHECK_EVERY_16_DF);
	_CrtSetReportMode(_CRT_ERROR | _CRT_WARN, _CRTDBG_MODE_DEBUG);

#ifndef _WIN64
	wprintf(L"SZ SCSI_PAGE_SENSE = 0x%X\n", sizeof(SCSI_PAGE_SENSE));
	wprintf(L"SZ SSP_PAGE_BUFFER = 0x%X\n", sizeof(SSP_PAGE_BUFFER));
	wprintf(L"SZ SSP_PAGE_NBES = 0x%X\n", sizeof(SSP_PAGE_NBES));
	wprintf(L"SZ SSP_PAGE_DES = 0x%X\n", sizeof(SSP_PAGE_DES));
	wprintf(L"SZ SSP_KAD = 0x%X\n", sizeof(SSP_KAD));
	wprintf(L"SZ SCSI_PAGE_INQ = 0x%X\n", sizeof(SCSI_PAGE_INQ));
	wprintf(L"SZ SSP_PAGE_SDE = 0x%X\n", sizeof(SSP_PAGE_SDE));
#else
	wprintf(L"SZ SCSI_PAGE_SENSE = 0x%zX\n", sizeof(SCSI_PAGE_SENSE));
	wprintf(L"SZ SSP_PAGE_BUFFER = 0x%zX\n", sizeof(SSP_PAGE_BUFFER));
	wprintf(L"SZ SSP_PAGE_NBES = 0x%zX\n", sizeof(SSP_PAGE_NBES));
	wprintf(L"SZ SSP_PAGE_DES = 0x%zX\n", sizeof(SSP_PAGE_DES));
	wprintf(L"SZ SSP_KAD = 0x%zX\n", sizeof(SSP_KAD));
	wprintf(L"SZ SCSI_PAGE_INQ = 0x%zX\n", sizeof(SCSI_PAGE_INQ));
	wprintf(L"SZ SSP_PAGE_SDE = 0x%zX\n", sizeof(SSP_PAGE_SDE));
#endif
#endif

	if (sizeof(SCSI_PAGE_SENSE) != 0x7F) { wprintf(L"Compiling error! Wrong struct SZ!\n"); return(255); }
	if (sizeof(SSP_PAGE_BUFFER) != 0x2004) { wprintf(L"Compiling error! Wrong struct SZ!\n"); return(255); }
	if (sizeof(SSP_PAGE_NBES) != 0x10) { wprintf(L"Compiling error! Wrong struct SZ!\n"); return(255); }
	if (sizeof(SSP_PAGE_DES) != 0x18) { wprintf(L"Compiling error! Wrong struct SZ!\n"); return(255); }
	if (sizeof(SSP_KAD) != 0x404) { wprintf(L"Compiling error! Wrong struct SZ!\n"); return(255); }
	if (sizeof(SCSI_PAGE_INQ) != 0x60) { wprintf(L"Compiling error! Wrong struct SZ!\n"); return(255); }
	if (sizeof(SSP_PAGE_SDE) != 0x34) { wprintf(L"Compiling error! Wrong struct SZ!\n"); return(255); }


	int action = 0; // 0 = status, 1 = setting encryption, 2 = generating key
	int TapeNumber = 0;
	SCSIEncryptOptions drvOptions;
	wstring keyfile = L"";
	wstring keydesc = L"";
	int KeyLength = 0;
	bool SkipMediaCheck = false;
	bool ShowVolumeStatus = false;
	memset(&drvOptions, 0, sizeof(drvOptions));

	drvOptions.CKOD = true;

	for (int i = 1; i < argc; i++)
	{
		wstring thisCmd = argv[i];
		StrToLower(thisCmd);
		wstring nextCmd = L"";
		if (i + 1 < argc)
		{
			if (wcsncmp(argv[i + 1], L"/", 1) != 0)
				nextCmd = argv[i + 1];
		}

		if (thisCmd == L"/?")
		{
			wprintf(L"Usage:\n\n");
			wprintf(L"  /?                This text\n");
			wprintf(L"  /t <num>          Use Tape drive # (default is 0 if not specified)\n");
			wprintf(L"  /e <param>        Enables / Disables Encryption\n");
			wprintf(L"      param:        on    - Enables encryption, does not read unencrypted data\n");
			wprintf(L"                    mixed - Enables encryption, still reads unencrypted data\n");
			wprintf(L"                    rread - Enables encryption, reads encrypted data as raw\n");
			wprintf(L"                    off   - Disables encryption, does not read encrypted data\n");
			wprintf(L"  /k <file>         Specifies the keyfile\n");
			wprintf(L"  /kd <text>        Specifies the key description\n");
			wprintf(L"  /protect <param>  Sets the protection\n");
			wprintf(L"      param:        on    - protects encrypted data from being raw-read\n");
			wprintf(L"                    off   - does not protect encrypted data from being raw-read\n");
			wprintf(L"  /nckod            Drive won't clear encryption key when ejecting media\n");
			wprintf(L"  /g <keylength>    Generates a new keyfile\n");
			wprintf(L"      keylength:    Size in Bits (e.g. 256)\n");
			wprintf(L"  /s                Skip Media check\n");
			wprintf(L"  /volumestatus     Show volume status\n");
			return(1);
		}

		if (thisCmd == L"/t")
		{
			if (nextCmd == L"")
			{
				wprintf(L"Missing Tape Number.\n");
				return(1);
			}
			TapeNumber = _wtoi(&nextCmd[0]);
			i++;
		}

		if (thisCmd == L"/e")
		{
			if (nextCmd == L"")
			{
				wprintf(L"Missing parameter.\n");
				return(1);
			}
			if (nextCmd == L"on")
			{
				drvOptions.cryptMode = CRYPTMODE_ON;
			}
			else if (nextCmd == L"mixed")
			{
				drvOptions.cryptMode = CRYPTMODE_MIXED;
			}
			else if (nextCmd == L"rread")
			{
				drvOptions.cryptMode = CRYPTMODE_RAWREAD;
			}
			else if (nextCmd == L"off")
			{
				drvOptions.cryptMode = CRYPTMODE_OFF;
			}
			else
			{
				wprintf(L"Unknown parameter: %s\n", &nextCmd[0]);
				return(1);
			}
			i++;
			action = 1;
		}

		if (thisCmd == L"/k")
		{
			if (nextCmd == L"")
			{
				wprintf(L"Missing keyfile.\n");
				return(1);
			}
			keyfile = nextCmd;
			i++;
		}

		if (thisCmd == L"/kd")
		{
			if (nextCmd == L"")
			{
				wprintf(L"Missing description.\n");
				return(1);
			}
			keydesc = nextCmd;
			if (keydesc.size() > SSP_UKAD_LENGTH)
			{
				wprintf(L"Description is too long, maximum %i characters.\n", SSP_UKAD_LENGTH);
				return(1);
			}
			i++;
		}

		if (thisCmd == L"/protect")
		{
			if (nextCmd == L"")
			{
				wprintf(L"Missing parameter.\n");
				return(1);
			}
			if (nextCmd == L"on")
			{
				drvOptions.rdmc = RDMC_PROTECT;
			}
			else if (nextCmd == L"off")
			{
				drvOptions.rdmc = RDMC_UNPROTECT;
			}
			else
			{
				wprintf(L"Unknown parameter: %s\n", &nextCmd[0]);
				return(1);
			}
			i++;
		}

		if (thisCmd == L"/nckod")
		{
			drvOptions.CKOD = false;
		}

		if (thisCmd == L"/s")
		{
			SkipMediaCheck = true;
		}

		if (thisCmd == L"/g")
		{
			if (nextCmd == L"")
			{
				wprintf(L"Missing key length parameter.\n");
				return(1);
			}
			KeyLength = _wtoi(&nextCmd[0]);
			if (KeyLength < 8)
			{
				wprintf(L"Minimum length must be 8 Bits.\n");
				return(1);
			}
			if (KeyLength > SSP_KEY_LENGTH * 8)
			{
				wprintf(L"Length is too long. Maximum %i Bits.\n", SSP_KEY_LENGTH * 8);
				return(1);
			}
			if (KeyLength % 8 != 0)
			{
				wprintf(L"Length must be divisible by 8.\n");
				return(1);
			}
			KeyLength /= 8;
			action = 2;
			i++;
		}

		if (thisCmd == L"/volumestatus")
		{
			ShowVolumeStatus = true;
		}
	}

	if (action == 2)
	{
		if (keyfile == L"")
		{
			wprintf(L"Keyfile not specified (/k).\n");
			return(1);
		}

		random_device rd;
		mt19937 mt(rd());
		uniform_int_distribution<int> dist(0, 255);

		wstringstream randomkey;
		for (int i = 0; i < KeyLength; i++)
		{
			randomkey << HEX(dist(mt));
		}
		randomkey << endl;

		ofstream kf;
		kf.open(keyfile, ios::trunc);
		if (!kf.is_open())
		{
			wprintf(L"Cannot create/open file %s.\n", &keyfile[0]);
			return(1);
		}
		kf << ws2s(randomkey.str()) << ws2s(keydesc);
		kf.close();
		wprintf(L"Keyfile %s created.\n", &keyfile[0]);
		return(0);
	}

	WCHAR Tape[20];
	swprintf_s(Tape, sizeof(Tape) / 2, L"\\\\.\\Tape%i", TapeNumber);

	wprintf(L"Using: %s\n", Tape);

	HANDLE hDevice = CreateFile(Tape, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
	if (hDevice == INVALID_HANDLE_VALUE)
	{
		wstring err = GetErrorAsString(GetLastError());
		wprintf(L"Cannot open device, Error 0x%X - %s\n", GetLastError(), &err[0]);
		return(5);
	}

	STORAGE_PROPERTY_QUERY storagePropertyQuery;
	memset(&storagePropertyQuery, 0, sizeof(STORAGE_PROPERTY_QUERY));
	storagePropertyQuery.PropertyId = StorageDeviceProperty;
	storagePropertyQuery.QueryType = PropertyStandardQuery;

	STORAGE_DESCRIPTOR_HEADER storageDescriptorHeader;
	DWORD dwBytesReturned = 0;
	if (!DeviceIoControl(hDevice, IOCTL_STORAGE_QUERY_PROPERTY, &storagePropertyQuery, sizeof(STORAGE_PROPERTY_QUERY), &storageDescriptorHeader, sizeof(STORAGE_DESCRIPTOR_HEADER), &dwBytesReturned, NULL))
	{
		wprintf(L"Cannot get device name\n");
		CloseHandle(hDevice);
		return(5);
	}

	STORAGE_DEVICE_DESCRIPTOR *pDeviceDescriptor = (STORAGE_DEVICE_DESCRIPTOR*)malloc(storageDescriptorHeader.Size);
	if (pDeviceDescriptor == NULL)
	{
		wprintf(L"Cannot alloc memory\n");
		CloseHandle(hDevice);
		return(5);
	}
	memset(pDeviceDescriptor, 0, storageDescriptorHeader.Size);

	if (!DeviceIoControl(hDevice, IOCTL_STORAGE_QUERY_PROPERTY, &storagePropertyQuery, sizeof(STORAGE_PROPERTY_QUERY), pDeviceDescriptor, storageDescriptorHeader.Size, &dwBytesReturned, NULL))
	{
		wprintf(L"Cannot get device name\n");
		CloseHandle(hDevice);
		return(5);
	}

	string Vendor = CString((char*)pDeviceDescriptor, pDeviceDescriptor->VendorIdOffset);
	string Product = CString((char*)pDeviceDescriptor, pDeviceDescriptor->ProductIdOffset);

	wprintf(L" >> %S %S\n\n", &Vendor[0], &Product[0]);

	free(pDeviceDescriptor);

	if (SkipMediaCheck == false)
	{
		DWORD tapestatus = GetTapeStatus(hDevice);
		tapestatus = GetTapeStatus(hDevice); //run it twice, fist call may be an I/O issue

		if (tapestatus != 0)
		{
			if (tapestatus == 1110)
			{
				tapestatus = PrepareTape(hDevice, TAPE_LOAD, false);
				if (tapestatus != 0)
				{
					wstring err = GetErrorAsString(GetLastError());
					wprintf(L"PrepareTape returned error 0x%X - %s\n", tapestatus, &err[0]);
					CloseHandle(hDevice);
					return(5);
				}
			}
			else
			{
				wstring err = GetErrorAsString(GetLastError());
				wprintf(L"Tapestatus returned error 0x%X - %s\n", tapestatus, &err[0]);
				CloseHandle(hDevice);
				return(5);
			}
		}
	}

	if (ShowVolumeStatus == true)
	{
		if (ShowvolumeStatus(hDevice, true) != 0)
			return(6);
	}
	if (ShowDriveStatus(hDevice) != 0)
		return(6);

	if (action == 1)
	{
		Keyinfo ki;
		string keyInput;
		string keyDesc;
		ifstream myfile(keyfile);
		if (myfile.is_open())
		{
			getline(myfile, keyInput);
			getline(myfile, keyDesc);
			myfile.close();
			ki.load(keyInput);
			if (!ki.valid)
			{
				wprintf(L"Keyfile %s is invalid.\n", &keyfile[0]);
				return(5);
			}
			drvOptions.keyName = keyDesc;
		}
		else
		{
			wprintf(L"Cannot load Keyfile %s.\n", &keyfile[0]);
			return(5);
		}

		drvOptions.cryptoKey.assign(ki.key, ki.keySize);

		wprintf(L"Changing encryption parameters ... ");

		int sensekey;
		int res = SCSIWriteEncryptOptions(hDevice, &drvOptions, &sensekey);
		if (res != 0)
		{
			if (res == 0x28A)
			{
				wprintf(L"\nCannot write encryption status: Sense Info: ");
				switch (sensekey)
				{
				case 0x1: wprintf(L"RECOVERED ERROR"); break;
				case 0x2: wprintf(L"NOT READY"); break;
				case 0x3: wprintf(L"MEDIUM ERROR"); break;
				case 0x4: wprintf(L"HARDWARE ERROR"); break;
				case 0x5: wprintf(L"ILLEGAL REQUEST"); break;
				case 0x6: wprintf(L"UNIT ATTENTION"); break;
				case 0x7: wprintf(L"DATA PROTECT"); break;
				case 0x8: wprintf(L"BLANK CHECK"); break;
				case 0xB: wprintf(L"COMMAND ABORTED"); break;
				case 0xD: wprintf(L"VOLUME OVERFLOW"); break;
				default:  wprintf(L"Unknown error: 0x%X", sensekey); break;
				}
				wprintf(L"\n");
			}
			else
			{
				wstring err = GetErrorAsString(res);
				wprintf(L"\nCannot write encryption status: 0x%X - %s\n", res, &err[0]);
			}
			CloseHandle(hDevice);
			return(6);
		}
		else
		{
			wprintf(L"OK\n");
		}

		if (ShowDriveStatus(hDevice) != 0)
			return(6);
		if (ShowvolumeStatus(hDevice, false) != 0)
			return(6);
	}

	CloseHandle(hDevice);

	return(0);
}

int ShowDriveStatus(HANDLE hDevice)
{
	int DE, DI, DO = 0;
	int res = HPQueryEncryption(hDevice, &DE, &DO, &DI);
	if (res != 0)
	{
		wstring err = GetErrorAsString(res);
		wprintf(L"Cannot query drive encryption status: 0x%X - %s\n", res, &err[0]);
		CloseHandle(hDevice);
		return(6);
	}

	wstring DEText = L"";
	switch (DE)
	{
	case DriveEncryption_On:
		DEText = L"On"; break;
	case DriveEncryption_Off:
		DEText = L"Off"; break;
	case DriveEncryption_Mixed:
		DEText = L"Mixed"; break;
	case DriveEncryption_RawRead:
		DEText = L"RawRead"; break;
	default:
		DEText = L"Unknwon"; break;
	}

	wstring DOText = L"";
	switch (DO)
	{
	case DriveOutput_Disabled:
		DOText = L"Disabled"; break;
	case DriveOutput_Decrypt:
		DOText = L"Decrypt"; break;
	case DriveOutput_Mixed:
		DOText = L"Mixed"; break;
	case DriveOutput_Raw:
		DOText = L"Raw"; break;
	default:
		DOText = L"Unknown"; break;
	}

	wstring DIText = L"";
	switch (DI)
	{
	case DriveInput_NotEncrypting:
		DIText = L"Disabled"; break;
	case DriveInput_Encrypting:
		DIText = L"Enabled"; break;
	default:
		DIText = L"Unknown"; break;
	}

	wprintf(L"Drive status:           %s\n", &DEText[0]);
	wprintf(L"Reading encrypted data: %s\n", &DOText[0]);
	wprintf(L"Writing encrypted data: %s\n", &DIText[0]);
	wprintf(L"\n");

	return(0);
}

int ShowvolumeStatus(HANDLE hDevice, bool WithRewind)
{
	int sta, rawp, algo;
	unsigned int numkeyinfo;
	vector<string> keytype;
	vector<string> keynames;
	int res = HPQueryEncryptionVolume(hDevice, &sta, &rawp, &numkeyinfo, keytype, keynames, &algo);
	if (res != 0)
	{
		wstring err = GetErrorAsString(res);
		wprintf(L"Cannot query media encryption status: 0x%X - %s\n", res, &err[0]);
		CloseHandle(hDevice);
		return(1);
	}

	if (sta == MediaEncryption_Cannot_Determine && WithRewind == true)
	{
		res = SetTapePosition(hDevice, TAPE_REWIND, 0, 0, 0, false);
		if (res != 0)
		{
			wstring err = GetErrorAsString(res);
			wprintf(L"Cannot rewind tape: 0x%X - %s\n", res, &err[0]);
			CloseHandle(hDevice);
			return(1);
		}

		res = HPQueryEncryptionVolume(hDevice, &sta, &rawp, &numkeyinfo, keytype, keynames, &algo);
		if (res != 0)
		{
			wstring err = GetErrorAsString(res);
			wprintf(L"Cannot query media encryption status (2): 0x%X - %s\n", res, &err[0]);
			CloseHandle(hDevice);
			return(1);
		}
	}

	switch (sta)
	{
	case MediaEncryption_Unknown:
		wprintf(L"Media Status: Unknown\n");
		break;
	case MediaEncryption_Cannot_Determine:
		wprintf(L"Media Status: Cannot determine\n");
		break;
	case MediaEncryption_NotEncrypted:
		wprintf(L"Media Status: Not encrypted\n");
		break;
	case MediaEncryption_Encrypted_Can_Decrypt:
		wprintf(L"Media Status: Encrypted - can be decrypted\n");
		if (rawp == 1)
			wprintf(L" >> protected from raw-read!\n");
		if (algo > 0)
			wprintf(L"Algorithm index: %i\n", algo);
		break;
	case MediaEncryption_Encrypted_Cannot_Decrypt:
		wprintf(L"Media Status: Encrypted - can NOT be decrypted\n");
		if (rawp == 1)
			wprintf(L" >> protected from raw-read!\n");
		if (algo > 0)
			wprintf(L"Algorithm index: %i\n", algo);
		if (numkeyinfo > 0)
		{
			wprintf(L"Keys:\n");
			for (unsigned int i = 0; i < numkeyinfo; i++)
			{
				wprintf(L" %S - %S\n", keytype[i].c_str(), keynames[i].c_str());
			}
		}
		break;
	}
	return(0);
}
