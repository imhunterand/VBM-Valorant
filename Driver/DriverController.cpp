#include <Windows.h>
#include <iostream>
#include <intrin.h>
#include <vector>
#include <fstream>
#include <stdint.h>
#include <string>
#include <filesystem>

#include <Dwmapi.h> 
#include <TlHelp32.h>
#include <Psapi.h>

#include "DriverController.h"

unsigned __int32 DriverController::GetProcessPidByName(const wchar_t* ProcessName)
{
	PROCESSENTRY32 pt;
	HANDLE hsnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	pt.dwSize = sizeof(PROCESSENTRY32);
	if (Process32First(hsnap, &pt)) {
		do {
			if (!lstrcmpi(pt.szExeFile, ProcessName)) {
				CloseHandle(hsnap);
				return pt.th32ProcessID;
			}
		} while (Process32Next(hsnap, &pt));
	}
	CloseHandle(hsnap);
	return 0;
}

bool DriverController::ResetProcessId(const wchar_t* TargetProcessName)
{
	TargetProcessPid = GetProcessPidByName(TargetProcessName);

	if (!TargetProcessPid)
		return false;

	return true;
}

DriverController::DriverController(unsigned int TargetProcessPid)
{
	DriverHandle = INVALID_HANDLE_VALUE;

	this->TargetProcessPid = TargetProcessPid;

	if (!this->TargetProcessPid) {

		printf("Target Process Does not Exist... did you mean to do this?\n");
	}

	DriverHandle = CreateFile(TEXT("\\\\.\\\ACPI_ROOT_OBJECT"), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
}

DriverController::DriverController(const wchar_t* TargetProcessName)
{
	DriverHandle = INVALID_HANDLE_VALUE;

	TargetProcessPid = GetProcessPidByName(TargetProcessName);

	if (!TargetProcessPid) {

		printf("Target Process Does not Exist... did you mean to do this?\n");
	}

	DriverHandle = CreateFile(TEXT("\\\\.\\\ACPI_ROOT_OBJECT"), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
}

DriverController::~DriverController()
{
	if (DriverHandle != INVALID_HANDLE_VALUE)
		CloseHandle(DriverHandle);
}

bool DriverController::WriteProcessMemory(unsigned __int64 Address, void* Buffer, unsigned __int32 Length)
{
	if (!Address || !Buffer || !Length)
		return false;

	unsigned char* TempAllocationBuffer = (unsigned char*)malloc(Length);

	WRITE_PROCESS_MEMORY DriverCall;

	DriverCall.Filter = 0xDEADBEEFCAFEBEEF;
	DriverCall.ControlCode = WRITE_PROCESS_MEMORY_IOCTL;

	DriverCall.ProcessAddress = Address;
	DriverCall.InBuffer = (unsigned __int64)TempAllocationBuffer;
	DriverCall.Length = Length;
	DriverCall.ProcessId = TargetProcessPid;

	memcpy(TempAllocationBuffer, Buffer, Length);

	DWORD BytesOut = 0;

	if (DeviceIoControl(DriverHandle, IOCTL_DISK_GET_DRIVE_GEOMETRY, &DriverCall, sizeof(DriverCall), &DriverCall, sizeof(DriverCall), &BytesOut, 0)) {

		free(TempAllocationBuffer);
		return true;
	}

	free(TempAllocationBuffer);
	return false;
}

bool DriverController::ReadProcessMemory(unsigned __int64 Address, void* Buffer, unsigned __int32 Length)
{
	if (!Address || !Buffer || !Length)
		return false;

	unsigned char* TempAllocationBuffer = (unsigned char*)malloc(Length);

	READ_PROCESS_MEMORY DriverCall;
	DriverCall.Filter = 0xDEADBEEFCAFEBEEF;
	DriverCall.ControlCode = READ_PROCESS_MEMORY_IOCTL;

	DriverCall.ProcessAddress = Address;
	DriverCall.OutBuffer = (unsigned __int64)TempAllocationBuffer;
	DriverCall.Length = Length;
	DriverCall.ProcessId = TargetProcessPid;

	DWORD BytesOut = 0;

	if (DeviceIoControl(DriverHandle, IOCTL_DISK_GET_DRIVE_GEOMETRY, &DriverCall, sizeof(DriverCall), &DriverCall, sizeof(DriverCall), &BytesOut, 0)) {

		memcpy(Buffer, TempAllocationBuffer, Length);

		free(TempAllocationBuffer);
		return true;
	}

	free(TempAllocationBuffer);
	return false;
}



unsigned long long DriverController::ReadUInt64(unsigned long long Address)
{
	unsigned long long buffer = 0;

	this->ReadProcessMemory((unsigned long long)Address, (void*)&buffer, 8);

	return (buffer);
}

unsigned short DriverController::ReadUInt16(unsigned long long Address)
{
	unsigned short buffer = 0;

	this->ReadProcessMemory((unsigned long long)Address, (void*)&buffer, 2);

	return (buffer);
}

unsigned int  DriverController::ReadUInt32(unsigned long long Address)
{
	unsigned int  buffer = 0;

	this->ReadProcessMemory((unsigned long long)Address, (void*)&buffer, 4);

	return (buffer);
}

float DriverController::ReadFloat(unsigned long long Address)
{
	float buffer = 0;

	this->ReadProcessMemory((unsigned long long)Address, (void*)&buffer, 4);

	return (buffer);
}

NTSTATUS ProcessReadWriteMemory(PEPROCESS SourceProcess, PVOID SourceAddress, PEPROCESS TargetProcess, PVOID TargetAddress, SIZE_T Size)
{
	SIZE_T Bytes = 0;

	if (NT_SUCCESS(MmCopyVirtualMemory(SourceProcess, SourceAddress, TargetProcess, TargetAddress, Size, UserMode, &Bytes)))
		return STATUS_SUCCESS;
	else
		return STATUS_ACCESS_DENIED;
}

void DriverController::WriteByte(unsigned long long Address, unsigned char data)
{
	this->WriteProcessMemory((unsigned long long)Address, (void*)&data, 1);
}

void DriverController::WriteUInt32(unsigned long long Address, unsigned int data)
{
	this->WriteProcessMemory((unsigned long long)Address, (void*)&data, 4);
}

	this->WriteProcessMemory((unsigned long long)Address, (void*)&data, 8);
}

		if (game_window) {
			RECT client;
			GetClientRect(game_window, &client);
			GetWindowRect(game_window, &window_rect);
			screen_width = window_rect.right - window_rect.left;
			screen_height = window_rect.bottom - window_rect.top;
			overlay_hwnd = CreateWindowEx(NULL,
				window_name,
				window_name,
				WS_POPUP | WS_VISIBLE,
				window_rect.left, window_rect.top, screen_width, screen_height,
				NULL,
				NULL,
				NULL,
				NULL);

			MARGINS margins = { -1 };
			DwmExtendFrameIntoClientArea(overlay_hwnd, &margins);
			SetWindowLong(overlay_hwnd, GWL_EXSTYLE, WS_EX_LAYERED | WS_EX_TRANSPARENT);
		}

		directx_init(overlay_hwnd);
		MSG msg;
		while (true) {
			while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			if (msg.message == WM_QUIT)
				exit(0);

			std::this_thread::sleep_for(std::chrono::milliseconds(120));
		}
	}
}

int PIDManager::GetProcessIdByName(LPCTSTR szProcess)//ע��Ҫ��exe��׺
{
	int dwRet = -1;
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	PROCESSENTRY32 pe32;
	pe32.dwSize = sizeof(PROCESSENTRY32);
	Process32First(hSnapshot, &pe32);
	do
	{
		if (_tcsicmp(pe32.szExeFile, szProcess) == 0)
		{
			dwRet = pe32.th32ProcessID;
			break;
		}
	} while (Process32Next(hSnapshot, &pe32));
	CloseHandle(hSnapshot);
	return dwRet;
}
