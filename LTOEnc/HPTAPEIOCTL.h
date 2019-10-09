#pragma once

void HPEjectTape(HANDLE hDevice);
int HPQueryEncryption(HANDLE hDevice, int *DriveEncryption, int *DriveOutput, int *DriveInput);
int HPQueryEncryptionVolume(HANDLE hDevice, int *status, int *rawreadprotected, unsigned int *numkeyinfo, vector<string> keytype, vector<string> keyname, int *algorithmindex);

#define OP_LOADUNLOAD 0x1B
#define OP_REQ_SENSE 0x3
#define OP_SPIN 0xA2
#define OP_SPOUT 0xB5
#define OP_INQUIRY 0x12

#define SSP_SP_PROTOCOL_TDE 0x20 
#define SSP_KAD_HEAD_LENGTH 4

#define SSP_PAGE_ALLOCATION 8192
#define SSP_DESCRIPTOR_LENGTH 1024
#define SSP_KEY_LENGTH 0x20
#define SSP_UKAD_LENGTH	0x1e

#define RDMC_DEFAULT 0x00
#define RDMC_PROTECT 0x03
#define RDMC_UNPROTECT 0x02
#define DEFAULT_ALGORITHM 0x01

#define KAD_TYPE_UKAD		0x00
#define KAD_TYPE_AKAD		0x01

#define DEFAULT_CEEM 0
#define DEFAULT_KEYSIZE 0x20

extern unsigned char scsi_sense_command[6];
extern unsigned char scsi_eject_command[6];
extern unsigned char scsi_inq_command[6];
extern unsigned char scsi_spin_des_command[12];
extern unsigned char scsi_spin_nbes_command[12];

typedef struct
{
	unsigned char responseCode : 7;
	unsigned char valid : 1;
	unsigned char res_bits_1;
	unsigned char senseKey : 4;
	unsigned char res_bits_2 : 1;
	unsigned char ILI : 1;
	unsigned char EOM : 1;
	unsigned char filemark : 1;
	unsigned char information[4];
	unsigned char addSenseLen;
	unsigned char cmdSpecificInfo[4];
	unsigned char addSenseCode;
	unsigned char addSenseCodeQual;
	unsigned char fieldRepUnitCode;
	unsigned char SKSV : 1;
	unsigned char cd : 1; // control/data 
	unsigned char resvd2 : 2; // reserved 
	unsigned char bpv : 1; // bit pointer valid 
	unsigned char sim : 3; // system information message 
	unsigned char field[2]; // field pointer 
	unsigned char addSenseData[109];
} SCSI_PAGE_SENSE;

typedef struct
{
	unsigned char pageCode[2];
	unsigned char length[2];
	unsigned char buffer[SSP_PAGE_ALLOCATION];
} SSP_PAGE_BUFFER; //generic ssp page buffer

typedef struct
{
	unsigned char pageCode[2];
	unsigned char length[2];
	unsigned char log_obj_num[8];
	unsigned char encryptionStatus : 4;
	unsigned char compressionStatus : 4;
	unsigned char algorithmIndex;
	unsigned char RDMDS : 1;
	unsigned char EMES : 1;
	unsigned char res_bits_1 : 6;
	unsigned char res_bits_2;
} SSP_PAGE_NBES; //next block encryption status page

typedef struct
{
	unsigned char pageCode[2];
	unsigned char length[2];
	unsigned char keyScope : 3;
	unsigned char res_bits_1 : 2;
	unsigned char nexusScope : 3;
	unsigned char encryptionMode;
	unsigned char decryptionMode;
	unsigned char algorithmIndex;
	unsigned char keyInstance[4];
	unsigned char RDMD : 1;
	unsigned char CEEMS : 2;
	unsigned char VCELB : 1;
	unsigned char parametersControl : 3;
	unsigned char res_bits_2 : 1;
	unsigned char res_bits_3;
	unsigned char ASDKCount[2];
	unsigned char res_bits_4[8];

} SSP_PAGE_DES; //device encryption status page

typedef struct
{
	unsigned char type;
	unsigned char authenticated : 3;
	unsigned char res_bits_1 : 5;
	unsigned char descriptorLength[2];
	unsigned char descriptor[SSP_DESCRIPTOR_LENGTH]; //will actually be the size of descriptorLength
} SSP_KAD;

typedef struct
{
	unsigned char peripheralQualifier : 3;
	unsigned char periphrealDeviceType : 5;
	unsigned char RMB : 1;
	unsigned char res_bits_1 : 7;
	unsigned char Version[1];
	unsigned char obs_bits_1 : 2;
	unsigned char NORMACA : 1;
	unsigned char HISUP : 1;
	unsigned char responseDataFormat : 4;
	unsigned char additionalLength[1];
	unsigned char SCCS : 1;
	unsigned char ACC : 1;
	unsigned char TPGS : 2;
	unsigned char threePC : 1;
	unsigned char res_bits_2 : 2;
	unsigned char protect : 1;
	unsigned char obs_bits_2 : 1;
	unsigned char ENCSERV : 1;
	unsigned char VS : 1;
	unsigned char MULTIP : 1;
	unsigned char MCHNGR : 1;
	unsigned char obs_bits_3 : 2;
	unsigned char ADDR16 : 1;
	unsigned char obs_bits_4 : 2;
	unsigned char WBUS16 : 1;
	unsigned char SYNC : 1;
	unsigned char obs_bits_5 : 2;
	unsigned char CMDQUE : 1;
	unsigned char VS2 : 1;
	unsigned char vender[8];
	unsigned char productID[16];
	unsigned char productRev[4];
	unsigned char SN[7];
	unsigned char venderUnique[12];
	unsigned char res_bits_3 : 4;
	unsigned char CLOCKING : 2;
	unsigned char QAS : 1;
	unsigned char IUS : 1;
	unsigned char res_bits_4[1];
	unsigned char versionDescriptor[16];
	unsigned char res_bits_5[22];
	unsigned char copyright[1];
} SCSI_PAGE_INQ; //device inquiry response

typedef struct
{ //structure for setting data encryption
	unsigned char pageCode[2];
	unsigned char length[2];
	unsigned char lock : 1;
	unsigned char res_bits_1 : 4;
	unsigned char scope : 3;
	unsigned char ckorl : 1;
	unsigned char ckorp : 1;
	unsigned char ckod : 1;
	unsigned char sdk : 1;
	unsigned char RDMC : 2;
	unsigned char CEEM : 2;
	unsigned char encryptionMode;
	unsigned char decryptionMode;
	unsigned char algorithmIndex;
	unsigned char keyFormat;
	unsigned char res_bits_2[8];
	unsigned char keyLength[2];
	unsigned char keyData[SSP_KEY_LENGTH];
} SSP_PAGE_SDE;

