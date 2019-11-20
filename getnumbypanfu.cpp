
#include "getnumbypanfu.h"
#include <winioctl.h>
#include <stdio.h>
#include <initguid.h>
#include <string.h>
#include <Setupapi.h>
#include <iostream>
#include <vector>
#include <map>

#define DISK_PATH_LEN 1024

DWORD GetPhysicalDriveFromPartitionLetter(wchar_t letter, std::map<std::wstring, ULONG>& sigmap);

/**
* find all drivers and show removeable drivers
*/
void FindAllDrivers(std::map<std::wstring,ULONG>& sigmap)
{	
	DWORD dwNumBytesForDriveStrings;//实际存储驱动器号的字符串长度
	HANDLE hHeap;
	LPWSTR lp;

	//获得实际存储驱动器号的字符串长度
	dwNumBytesForDriveStrings = GetLogicalDriveStrings(0, NULL) * sizeof(TCHAR);

	//如果字符串不为空，则表示有正常的驱动器存在
	if (dwNumBytesForDriveStrings != 0) {
		//分配字符串空间
		hHeap = GetProcessHeap();
		lp = (LPWSTR)HeapAlloc(hHeap, HEAP_ZERO_MEMORY, dwNumBytesForDriveStrings);

		//获得标明所有驱动器的字符串
		GetLogicalDriveStrings(HeapSize(hHeap, 0, lp), lp);

		//将驱动器一个个放到下拉框中  
		wchar_t s[40];
		while (*lp != 0) {
			UINT nDriveType = GetDriveType(lp);
			if (nDriveType == DRIVE_REMOVABLE) {
				//sprintf(s, "%s (%c:) %d MB", "可移动驱动器", lp[0], getTotalSpace(lp));
				//驱动器是这样的 c:\ d:\ 所以只取第一个字符
				DWORD index = GetPhysicalDriveFromPartitionLetter(lp[0],sigmap);

				int k = 0;
				k++;
			}
			lp = wcschr(lp, 0) + 1;			
		}
	}

}


/******************************************************************************
* Function: get disk's physical number from its drive letter
*           e.g. C-->0 (C: is on disk0)
* input: letter, drive letter
* output: N/A
* return: Succeed, disk number
*         Fail, -1
******************************************************************************/
DWORD GetPhysicalDriveFromPartitionLetter(wchar_t letter, std::map<std::wstring, ULONG>& sigmap)
{
	HANDLE hDevice;               // handle to the drive to be examined
	BOOL result;                 // results flag
	DWORD readed;                   // discard results
	STORAGE_DEVICE_NUMBER number;   //use this to get disk numbers

	wchar_t path[DISK_PATH_LEN];
	swprintf(path, L"\\\\.\\%c:", letter);
	hDevice = CreateFile(path, // drive to open
		GENERIC_READ | GENERIC_WRITE,    // access to the drive
		FILE_SHARE_READ | FILE_SHARE_WRITE,    //share mode
		NULL,             // default security attributes
		OPEN_EXISTING,    // disposition
		0,                // file attributes
		NULL);            // do not copy file attribute
	if (hDevice == INVALID_HANDLE_VALUE) // cannot open the drive
	{
		//fwprintf(stderr, L"CreateFile() Error: %ld\n", GetLastError());
		int er = GetLastError();
		return DWORD(-1);
	}

	result = DeviceIoControl(
		hDevice,                // handle to device
		IOCTL_STORAGE_GET_DEVICE_NUMBER, // dwIoControlCode
		NULL,                            // lpInBuffer
		0,                               // nInBufferSize
		&number,           // output buffer
		sizeof(number),         // size of output buffer
		&readed,       // number of bytes returned
		NULL      // OVERLAPPED structure
	);
	if (!result) // fail
	{
		//fprintf(stderr, "IOCTL_STORAGE_GET_DEVICE_NUMBER Error: %ld\n", GetLastError());
		int er = GetLastError();
		(void)CloseHandle(hDevice);
		return (DWORD)-1;
	}
	//printf("%d %d %d\n\n", number.DeviceType, number.DeviceNumber, number.PartitionNumber);
	//get signature
	
	DRIVE_LAYOUT_INFORMATION_EX layOutInfo[20];
	memset(&layOutInfo, 0, sizeof(DRIVE_LAYOUT_INFORMATION_EX) * 20);
	DWORD bytesReturned;

	if (DeviceIoControl(hDevice, IOCTL_DISK_GET_DRIVE_LAYOUT_EX, NULL, 0, &layOutInfo, sizeof(DRIVE_LAYOUT_INFORMATION_EX) * 20, &bytesReturned, NULL) == 0)
	{		
		DWORD error;
		error = GetLastError();
		HRESULT hRe = HRESULT_FROM_WIN32(error);
		char errorData[10];
		//sprintf(errorData, "%x", hRe);
	}
	int partitionCount = layOutInfo[0].PartitionCount;
	ULONG signature = 0;
	for (int i = 0; i < partitionCount; i++)
	{
		if (0 != layOutInfo[i].Mbr.Signature)
		{
			signature = layOutInfo[i].Mbr.Signature;
		}
	}
	wchar_t lt[2] = { letter,0 };
	std::wstring tmp = lt;
	sigmap[tmp]=signature;
	//end get signature;

	(void)CloseHandle(hDevice);
	return number.DeviceNumber;
}
/*
* get totol  space of disk
* return : MB
*/
int getTotalSpace(const wchar_t* lpRootPathName)
{
	unsigned long long available, total, free;
	if (GetDiskFreeSpaceEx(lpRootPathName, (ULARGE_INTEGER*)&available, (ULARGE_INTEGER*)&total, (ULARGE_INTEGER*)&free)) {
		return total >> 20;
	}
	else {
		return -1;
	}
}