#include <Windows.h>
#include <stdio.h>
#include <ntddscsi.h>
#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>
#include <bitset>
#include <vector>

using namespace std;

#include "HPTAPEIOCTL.h"
#include "Common.h"
#include "Encryption.h"


SCSIEncryptOptions::SCSIEncryptOptions()
{
	cryptMode = CRYPTMODE_OFF;
	algorithmIndex = DEFAULT_ALGORITHM;
	cryptoKey = "";
	CKOD = false;
	keyName = "";
	rdmc = RDMC_DEFAULT;
}

SSP_NBES::SSP_NBES(SSP_PAGE_BUFFER* buffer)
{
	memset(&nbes, 0, sizeof(SSP_PAGE_NBES));
	memcpy(&nbes, buffer, sizeof(SSP_PAGE_NBES));
	loadKADs(buffer, sizeof(SSP_PAGE_NBES));

}
SSP_DES::SSP_DES(SSP_PAGE_BUFFER* buffer)
{
	memset(&des, 0, sizeof(SSP_PAGE_DES));
	memcpy(&des, buffer, sizeof(SSP_PAGE_DES));
	loadKADs(buffer, sizeof(SSP_PAGE_DES));
}

void KAD_CLASS::loadKADs(SSP_PAGE_BUFFER* buffer, int start)
{
	char* rawbuff = (char*)buffer;
	int length = BSSHORT(buffer->length) + 4;
	int pos = start;
	while (pos < length)
	{
		SSP_KAD kad;
		memset(&kad, 0, sizeof(SSP_KAD));
		memcpy(&kad, rawbuff + pos, SSP_KAD_HEAD_LENGTH);
		pos += SSP_KAD_HEAD_LENGTH;
		if (pos >= length)break;
		unsigned short kadDesLen = BSSHORT(kad.descriptorLength);
		if (kadDesLen > 0)
		{
			memcpy(&kad.descriptor, rawbuff + pos, kadDesLen);
			pos += kadDesLen;
		}
		else pos++;
		kads.push_back(kad);
	}
}

//Gets encryption options on the tape drive
SSP_DES* SSPGetDES(HANDLE hDevice)
{
	SSP_PAGE_BUFFER buffer;
	memset(&buffer, 0, sizeof(SSP_PAGE_BUFFER));

	UCHAR SenseInfo[SPT_SENSE_BUFFER_LENGTH];
	int SCSIStatus;

	int res = QuerySCSI(hDevice, SCSI_IOCTL_DATA_IN, sizeof(scsi_spin_des_command), scsi_spin_des_command, sizeof(buffer), &buffer, 10, SenseInfo, &SCSIStatus);
	if (res != 0)
		return(NULL);
	SSP_DES* status = new SSP_DES(&buffer);
	return (status);
}

SSP_NBES* SSPGetNBES(HANDLE hDevice)
{
	SSP_PAGE_BUFFER  buffer;
	memset(&buffer, 0, sizeof(SSP_PAGE_BUFFER));

	UCHAR SenseInfo[SPT_SENSE_BUFFER_LENGTH];
	int SCSIStatus;

	int res = QuerySCSI(hDevice, SCSI_IOCTL_DATA_IN, sizeof(scsi_spin_nbes_command), scsi_spin_nbes_command, sizeof(buffer), &buffer, 10, SenseInfo, &SCSIStatus);
	if (res != 0)
		return(NULL);

	SSP_NBES* status = new SSP_NBES(&buffer);
	return (status);
}

int SCSIWriteEncryptOptions(HANDLE hDevice, SCSIEncryptOptions* eOptions, int *Sense)
{
	char buffer[1024];
	memset(&buffer, 0, 1024);

	*Sense = 0;

	SSP_PAGE_SDE options;
	//copy the template over the options
	memset(&options, 0, sizeof(SSP_PAGE_SDE));
	byteswap((unsigned char*)&options.pageCode, 2, 0x10);
	int pagelen = sizeof(SSP_PAGE_SDE);
	options.scope = 2; //all IT nexus = 10b
	options.RDMC = eOptions->rdmc;
	options.ckod = (eOptions->CKOD) ? 1 : 0;
	options.CEEM = DEFAULT_CEEM;
	options.algorithmIndex = 1; //eOptions->algorithmIndex;
	//set the specific options
	switch (eOptions->cryptMode)
	{
		case CRYPTMODE_ON: //encrypt, read only encrypted data
			options.encryptionMode = 2;
			options.decryptionMode = 2;
			break;
		case CRYPTMODE_MIXED: //encrypt, read all data
			options.encryptionMode = 2;
			options.decryptionMode = 3;
			break;
		case CRYPTMODE_RAWREAD:
			options.encryptionMode = 2;
			options.decryptionMode = 1;
			break;
		default:
			byteswap((unsigned char*)options.keyLength, 2, DEFAULT_KEYSIZE);
			eOptions->cryptoKey = ""; //blank the key
			eOptions->keyName = ""; //blank the key name, not supported when turned off
			break;
	}

	if (eOptions->cryptoKey != "")
	{
		//byte swap the keylength
		byteswap((unsigned char*)&options.keyLength, 2, (int)eOptions->cryptoKey.size());
		//copy the crypto key into the options
		eOptions->cryptoKey.copy((char*)&options.keyData, eOptions->cryptoKey.size(), 0);
	}
	//create the key descriptor
	if (eOptions->keyName != "")
	{
		SSP_KAD kad;
		memset(&kad, 0, sizeof(kad));
		kad.type = 0x00;
		kad.authenticated = 0;
		//set the descriptor length to the length of the keyName
		byteswap((unsigned char*)&kad.descriptorLength, 2, (int)eOptions->keyName.size());

		//get the size of the kad object
		int kadlen = (int)(eOptions->keyName.size() + SSP_KAD_HEAD_LENGTH);
		//increment the SPOUT page len
		pagelen += kadlen;
		//increase the page size
		eOptions->keyName.copy((char*)&kad.descriptor, eOptions->keyName.size(), 0);
		//copy the kad after the SDE command
		memcpy(&buffer[sizeof(SSP_PAGE_SDE)], &kad, kadlen);
	}
	//update the pagelen in options
	byteswap((unsigned char*)&options.length, 2, pagelen - 4); //set the page length, minus the length and pageCode

	//copy the options to the beginning of the buffer
	memcpy(&buffer, &options, sizeof(SSP_PAGE_SDE));

	unsigned char spout_sde_command[12] = { OP_SPOUT, SSP_SP_PROTOCOL_TDE, 0, 0x10, 0, 0, BSINTTOCHAR(pagelen), 0, 0 };

	UCHAR SenseInfo[SPT_SENSE_BUFFER_LENGTH];
	int SCSIStatus;

	int res = QuerySCSI(hDevice, SCSI_IOCTL_DATA_OUT, sizeof(spout_sde_command), spout_sde_command, sizeof(SSP_PAGE_SDE), &buffer, 10, SenseInfo, &SCSIStatus);
	if (res == 0)
	{
		if (SCSIStatus == 0x2) //Check Condition
		{
			SCSI_PAGE_SENSE pg;
			memset(&pg, 0x0, sizeof(pg));
			memcpy(&pg, &SenseInfo[0], SPT_SENSE_BUFFER_LENGTH);			
			*Sense = pg.senseKey;
			return(0x28A);
		}
	}
	else
	{
		return(res);
	}

	return(0);
}

//Sends and inquiry to the device
SCSI_PAGE_INQ* SCSIGetInquiry(HANDLE hDevice)
{
	SCSI_PAGE_INQ* status = new SCSI_PAGE_INQ;
	memset(status, 0, sizeof(SCSI_PAGE_INQ));

	UCHAR SenseInfo[SPT_SENSE_BUFFER_LENGTH];
	int SCSIStatus;

	int res = QuerySCSI(hDevice, SCSI_IOCTL_DATA_IN, sizeof(scsi_inq_command), scsi_inq_command, sizeof(status), &status, 10, SenseInfo, &SCSIStatus);
	if (res != 0)
		return(NULL);
	return(status);
}

void byteswap(unsigned char* array, int size, int value)
{
	switch (size)
	{
		case 2:
			array[0] = (unsigned char)((value & 0xff00) >> 8);
			array[1] = (unsigned char)(value & 0x00ff);
			break;
		case 4:
			array[0] = (unsigned char)((value & 0xff000000) >> 24);
			array[1] = (unsigned char)((value & 0x00ff0000) >> 16);
			array[2] = (unsigned char)((value & 0x0000ff00) >> 8);
			array[3] = (unsigned char)(value & 0x000000ff);
			break;
		default:
			break;
	}
}
