#pragma once

void byteswap(unsigned char* array, int size, int value);

class KAD_CLASS
{
public:
	std::vector<SSP_KAD> kads;
protected:
	void loadKADs(SSP_PAGE_BUFFER* buffer, int start);
};


//class used to parse next block encryption status page
class SSP_NBES : public KAD_CLASS
{
public:
	SSP_PAGE_NBES nbes;
	SSP_NBES(SSP_PAGE_BUFFER* buffer);
};
//class used to parse data encryption status page
class SSP_DES : public KAD_CLASS
{
public:
	SSP_PAGE_DES des;
	SSP_DES(SSP_PAGE_BUFFER* buffer);
};


//enum for SCSIEncryptOptions.cryptMode
enum { CRYPTMODE_OFF, CRYPTMODE_MIXED, CRYPTMODE_ON, CRYPTMODE_RAWREAD };

//used to pass parameters to SCSIWriteEncryptOptions
class SCSIEncryptOptions
{
public:
	int rdmc;
	bool CKOD;
	int cryptMode;
	unsigned int algorithmIndex;
	std::string cryptoKey;
	std::string keyName;
	SCSIEncryptOptions();
};

//Gets encryption options on the tape drive
SSP_DES* SSPGetDES(HANDLE hDevice);
//Gets the encryption status from the tape volume
SSP_NBES* SSPGetNBES(HANDLE hDevice);
//Writes encryption options to the tape drive
int SCSIWriteEncryptOptions(HANDLE hDevice, SCSIEncryptOptions* eOptions, int *Sense);
//Gets device inquiry
SCSI_PAGE_INQ* SCSIGetInquiry(HANDLE hDevice);

