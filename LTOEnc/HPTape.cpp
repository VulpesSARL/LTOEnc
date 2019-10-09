#include <windows.h>
#include <stdio.h>
#include <ntddscsi.h>

#include <vector>
#include <string>

using namespace std;

#include "HPTAPEIOCTL.h"
#include "Common.h"
#include "Encryption.h"

unsigned char scsi_sense_command[6] = { OP_REQ_SENSE, 0, 0, 0, sizeof(SCSI_PAGE_SENSE), 0 };
unsigned char scsi_eject_command[6] = { OP_LOADUNLOAD, 1, 0, 0, 0, 0 };
unsigned char scsi_spin_des_command[12] = { OP_SPIN, SSP_SP_PROTOCOL_TDE, 0, 0x20, 0, 0, BSINTTOCHAR(sizeof(SSP_PAGE_BUFFER)), 0, 0 };
unsigned char scsi_spin_nbes_command[12] = { OP_SPIN, SSP_SP_PROTOCOL_TDE, 0, 0x21, 0, 0, BSINTTOCHAR(sizeof(SSP_PAGE_BUFFER)), 0, 0 };
unsigned char scsi_inq_command[6] = { OP_INQUIRY, 0, 0, 0, sizeof(SCSI_PAGE_INQ), 0 };



void HPEjectTape(HANDLE hDevice)
{
	//No use here, testing only

	UCHAR SenseInfo[SPT_SENSE_BUFFER_LENGTH];
	int SCSIStatus;
	int res = QuerySCSI(hDevice, SCSI_IOCTL_DATA_UNSPECIFIED, sizeof(scsi_eject_command), scsi_eject_command, 0, NULL, 10, SenseInfo, &SCSIStatus);
}

int HPQueryEncryption(HANDLE hDevice, int *DriveEncryption, int *DriveOutput, int *DriveInput)
{
	*DriveEncryption = DriveEncryption_Unknown;
	*DriveOutput = DriveOutput_Unknown;
	*DriveInput = DriveInput_Unknown;

	SSP_DES *opt = SSPGetDES(hDevice);
	if (opt == NULL)
		return (GetLastError());

	if ((int)opt->des.encryptionMode == 0x2 && //encrypt
		(int)opt->des.decryptionMode == 0x2 //read only encrypted data
		)
		*DriveEncryption = DriveEncryption_On;
	if ((int)opt->des.encryptionMode == 0x2 && //encrypt
		(int)opt->des.decryptionMode == 0x3 //read encrypted and unencrypted
		)
		*DriveEncryption = DriveEncryption_Mixed;

	if ((int)opt->des.encryptionMode == 0x2 && //encrypt
		(int)opt->des.decryptionMode == 0x1 //read encrypted and unencrypted
		)
		*DriveEncryption = DriveEncryption_RawRead;

	if ((int)opt->des.encryptionMode == 0x0 && //encrypt
		(int)opt->des.decryptionMode == 0x0  //read encrypted and unencrypted
		)
		*DriveEncryption = DriveEncryption_Off;

	switch ((int)opt->des.decryptionMode)
	{
		case 0x0:
			*DriveOutput = DriveOutput_Disabled;
			break;
		case 0x1:
			*DriveOutput = DriveOutput_Raw;
			break;
		case 0x2:
			*DriveOutput = DriveOutput_Decrypt;
			break;
		case 0x3:
			*DriveOutput = DriveOutput_Mixed;
			break;
	}

	switch ((int)opt->des.encryptionMode)
	{
		case 0x0:
			*DriveInput = DriveInput_NotEncrypting;
			break;
		case 0x2:
			*DriveInput = DriveInput_Encrypting;
			break;
	}

	return(0);
}

int HPQueryEncryptionVolume(HANDLE hDevice, int *status, int *rawreadprotected, unsigned int *numkeyinfo, vector<string> keytype, vector<string> keyname, int *algorithmindex)
{
	*status = MediaEncryption_Unknown;
	*rawreadprotected = 0;
	*numkeyinfo = 0;
	*algorithmindex = 0;

	SSP_NBES *opt = SSPGetNBES(hDevice);
	if (opt == NULL)
		return (GetLastError());

	switch (opt->nbes.encryptionStatus)
	{
		case 0x1:
			*status = MediaEncryption_Cannot_Determine; break;
		case 0x3:
			*status = MediaEncryption_NotEncrypted; break;
		case 0x5:
			*status = MediaEncryption_Encrypted_Can_Decrypt;
			if (opt->nbes.RDMDS == 1)
				*rawreadprotected = 1;
			break;
		case 0x6:
			*status = MediaEncryption_Encrypted_Cannot_Decrypt;
			if (opt->kads.size() > 0)
			{
				*numkeyinfo = (int)opt->kads.size();
				for (unsigned int i = 0; i < opt->kads.size(); i++)
				{
					switch (opt->kads[i].type)
					{
						case KAD_TYPE_UKAD:
							keytype.push_back("uKAD");
							keyname.push_back(string((const char*)&opt->kads[i].descriptor, BSSHORT(opt->kads[i].descriptorLength)));
							break;
						case KAD_TYPE_AKAD:
							keytype.push_back("aKAD");
							keyname.push_back(string((const char*)&opt->kads[i].descriptor, BSSHORT(opt->kads[i].descriptorLength)));
							break;
					}
				}
			}
			if (opt->nbes.RDMDS == 1)
				*rawreadprotected = 1;
			break;
	}

	if (opt->nbes.algorithmIndex != 0)
	{
		*algorithmindex = opt->nbes.algorithmIndex;
	}

	return(0);
}

