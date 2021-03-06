#pragma once

#define IOCTL_TAPE_GET_ENCRYPTION_STATE CTL_CODE(IOCTL_TAPE_BASE, 0x0820, METHOD_BUFFERED, FILE_READ_ACCESS)

typedef struct _ENCRYPTION_STATUS
{
	UCHAR ucEncryptionCapable; //Set this field as a boolean based on the capability of the drive
	UCHAR ucEncryptionMethod; //Set this field to one of the defines METHOD_* below
	UCHAR ucEncryptionState; //Set this field to one of the #defines STATE_* below
	UCHAR aucReserved[13];
} ENCRYPTION_STATUS, *PENCRYPTION_STATUS;

#define ENCRYPTION_METHOD_NONE 0  //Only used in GET_ENCRYPTION_STATE
#define ENCRYPTION_METHOD_LIBRARY 1 //Only used in GET_ENCRYPTION_STATE
#define ENCRYPTION_METHOD_SYSTEM 2 //Only used in GET_ENCRYPTION_STATE
#define ENCRYPTION_METHOD_APPLICATION 3 //Only used in GET_ENCRYPTION_STATE
#define ENCRYPTION_METHOD_CUSTOM 4 //Only used in GET_ENCRYPTION_STATE
#define ENCRYPTION_METHOD_UNKNOWN 5 //Only used in GET_ENCRYPTION_STATE

#define ENCRYPTION_STATE_OFF 0 //Used in GET/SET_ENCRYPTION_STATE
#define ENCRYPTION_STATE_ON 1 //Used in GET/SET_ENCRYPTION_STATE
#define ENCRYPTION_STATE_NA 2 //Only used in GET_ENCRYPTION_STATE

#define IOCTL_TAPE_SET_ENCRYPTION_STATE CTL_CODE(IOCTL_TAPE_BASE, 0x0821, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)


#define IOCTL_TAPE_SET_DATA_KEY CTL_CODE(IOCTL_TAPE_BASE, 0x0822, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)

#define DATA_KEY_INDEX_LENGTH 12
#define DATA_KEY_RESERVED1_LENGTH 15
#define DATA_KEY_LENGTH 32
#define DATA_KEY_RESERVED2_LENGTH 48
typedef struct _DATA_KEY
{
	UCHAR aucDataKeyIndex[DATA_KEY_INDEX_LENGTH];
	UCHAR ucDataKeyIndexLength;
	UCHAR aucReserved1[DATA_KEY_RESERVED1_LENGTH];
	UCHAR aucDataKey[DATA_KEY_LENGTH];
	UCHAR aucReserved2[DATA_KEY_RESERVED2_LENGTH];
} DATA_KEY, *PDATA_KEY;

