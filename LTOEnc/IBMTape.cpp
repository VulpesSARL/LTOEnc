#include <windows.h>
#include <stdio.h>
#include "IBMTAPEIOCTL.h"

/*


	ENCRYPTION_STATUS scEncryptStat;
	DWORD cb;
	if (DeviceIoControl(hDevice, IOCTL_TAPE_GET_ENCRYPTION_STATE, &scEncryptStat, sizeof(ENCRYPTION_STATUS), &scEncryptStat, sizeof(ENCRYPTION_STATUS), &cb, NULL) == 0)
	{
		wprintf(L"DeviceIoControl returned error 0x%X\n", GetLastError());
		CloseHandle(hDevice);
		return(2);
	}
	else
	{
		wprintf(L"ENCRYPTION_STATUS.ucEncryptionCapable = 0x%X\n", scEncryptStat.ucEncryptionCapable);
		switch (scEncryptStat.ucEncryptionState)
		{
			case ENCRYPTION_STATE_OFF:
				wprintf(L"ENCRYPTION_STATUS.ucEncryptionState = OFF\n");
				break;
			case ENCRYPTION_STATE_ON:
				wprintf(L"ENCRYPTION_STATUS.ucEncryptionState = ON\n");
				break;
			case ENCRYPTION_STATE_NA:
				wprintf(L"ENCRYPTION_STATUS.ucEncryptionState = NA\n");
				break;
			default:
				wprintf(L"ENCRYPTION_STATUS.ucEncryptionState = 0x%X\n", scEncryptStat.ucEncryptionState);
				break;
		}

		switch (scEncryptStat.ucEncryptionMethod)
		{
			case ENCRYPTION_METHOD_NONE:
				wprintf(L"ENCRYPTION_STATUS.ucEncryptionMethod = NONE\n");
				break;
			case ENCRYPTION_METHOD_APPLICATION:
				wprintf(L"ENCRYPTION_STATUS.ucEncryptionMethod = APPLICATION\n");
				break;
			case ENCRYPTION_METHOD_CUSTOM:
				wprintf(L"ENCRYPTION_STATUS.ucEncryptionMethod = CUSTOM\n");
				break;
			case ENCRYPTION_METHOD_LIBRARY:
				wprintf(L"ENCRYPTION_STATUS.ucEncryptionMethod = LIBRARY\n");
				break;
			case ENCRYPTION_METHOD_SYSTEM:
				wprintf(L"ENCRYPTION_STATUS.ucEncryptionMethod = SYSTEM\n");
				break;
			case ENCRYPTION_METHOD_UNKNOWN:
				wprintf(L"ENCRYPTION_STATUS.ucEncryptionMethod = UNKNOWN\n");
				break;
			default:
				wprintf(L"ENCRYPTION_STATUS.ucEncryptionMethod = 0x%X\n", scEncryptStat.ucEncryptionMethod);
				break;
		}
	}

	//ENCRYPTION_STATUS scEncryptStat;
	//DeviceIoControl(hDevice, IOCTL_TAPE_SET_ENCRYPTION_STATE, &scEncryptStat, sizeof(ENCRYPTION_STATUS), &scEncryptStat, sizeof(ENCRYPTION_STATUS), &cb, NULL);

	//DATA_KEY scDataKey;
	//DeviceIoControl(hDevice, IOCTL_TAPE_SET_DATA_KEY, &scDataKey, sizeof(DATA_KEY), &scDataKey, sizeof(DATA_KEY), &cb, NULL);


*/