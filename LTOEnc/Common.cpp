#include <Windows.h>
#include <ntddscsi.h>
#include <string>
#include <locale>
#include <codecvt>

using namespace std;

#include "Common.h"

typedef struct
{
	SCSI_PASS_THROUGH_DIRECT spt;
	ULONG                    pad;
	UCHAR                    senseBuffer[SPT_SENSE_BUFFER_LENGTH];
} SPTTfrType;

int QuerySCSI(HANDLE hDevice, UCHAR DataIn, UCHAR CdbLength, UCHAR *Cdb, ULONG DataTransferLength, void *DataTransfer, int Timeout, UCHAR SenseOutput[SPT_SENSE_BUFFER_LENGTH], int *SCSIStatus)
{
	SPTTfrType          sptio;
	memset(&sptio, 0, sizeof(sptio));

	sptio.spt.Length = sizeof(SCSI_PASS_THROUGH_DIRECT);
	sptio.spt.CdbLength = CdbLength;
	sptio.spt.DataTransferLength = DataTransferLength;
	sptio.spt.DataIn = DataIn;// SCSI_IOCTL_DATA_IN; //Host Read
	sptio.spt.TimeOutValue = Timeout;
	sptio.spt.DataBuffer = DataTransfer;
	sptio.spt.SenseInfoOffset = offsetof(SPTTfrType, senseBuffer);
	sptio.spt.SenseInfoLength = SPT_SENSE_BUFFER_LENGTH;

	for (int i = 0; i < CdbLength; i++)
	{
		sptio.spt.Cdb[i] = Cdb[i];
	}

	DWORD len;
	int status = DeviceIoControl(hDevice, IOCTL_SCSI_PASS_THROUGH_DIRECT, &sptio, sizeof(SPTTfrType), &sptio, sizeof(SPTTfrType), &len, NULL);

	if (status == 0)
		return(GetLastError());

	memcpy(SenseOutput, sptio.senseBuffer, SPT_SENSE_BUFFER_LENGTH);

	*SCSIStatus = sptio.spt.ScsiStatus;

	//success
	return(0);
}

string CString(char *buffer, int offset)
{
	int len = 0;
	int off = offset;
	do
	{
		if (buffer[off] == 0)
			break;
		off++;
		len++;
	} while (1);

	if (len == 0)
		return("");

	char *c = (char*)malloc(len + 1);
	if (c == NULL)
		return("");
	memcpy(c, &buffer[offset], len + 1);
	string str(c);
	free(c);
	return (str);
}

wstring GetErrorAsString(DWORD error)
{
	if (error == 0)
		return (L""); //No error message has been recorded

	LPWSTR messageBuffer = nullptr;
	size_t size = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, error, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPWSTR)&messageBuffer, 0, NULL);

	wstring message(messageBuffer, size);

	//Free the buffer.
	LocalFree(messageBuffer);

	return (message);
}

wstring s2ws(const string& str)
{
	using convert_typeX = codecvt_utf8<wchar_t>;
	wstring_convert<convert_typeX, wchar_t> converterX;

	return converterX.from_bytes(str);
}

string ws2s(const wstring& wstr)
{
	using convert_typeX = codecvt_utf8<wchar_t>;
	wstring_convert<convert_typeX, wchar_t> converterX;

	return converterX.to_bytes(wstr);
}