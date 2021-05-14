#include <Windows.h>
#include <iostream>
#include <map>
#include <vector>
#include <tchar.h>
#include "ThreadLockHandle.h"

using namespace std;

CThreadLock g_cs;
HANDLE g_hEvent;
map<LPTSTR, BOOL>g_monitor_usb_task;

DWORD WINAPI ThreadFunc_USBDriverMonitor(LPVOID lpParam);
DWORD WINAPI ThreadFunc_USBDriverPackage(LPVOID lpParam);

int main()
{
	g_hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	CreateThread(NULL, 0, ThreadFunc_USBDriverMonitor, NULL, 0, 0);
	CreateThread(NULL, 0, ThreadFunc_USBDriverPackage, NULL, 0, 0);
	while (true)
	{
		Sleep(60000);
	}
	return 0;
}

DWORD WINAPI ThreadFunc_USBDriverMonitor(LPVOID lpParam)
{
	while (true)
	{
		vector<LPTSTR>m_tmp_usb_task;
		BOOL isNew = FALSE;
		TCHAR DriverString[MAX_PATH];
		LPTSTR pDriver = NULL;
		DWORD dwDriverStringLen = sizeof(DriverString) / sizeof(TCHAR);
		ZeroMemory(DriverString, dwDriverStringLen);
		GetLogicalDriveStrings(dwDriverStringLen, DriverString);
		pDriver = DriverString;
		for (DWORD dwOffset = 1; *pDriver != '\0'; pDriver += _tcslen(pDriver) + 1)
		{
			if (GetDriveType(pDriver) == DRIVE_REMOVABLE)
			{
				
				m_tmp_usb_task.push_back(pDriver);
				if (g_monitor_usb_task.find(pDriver) == g_monitor_usb_task.end())
				{
					isNew = TRUE;
					if (GetVolumeInformation(pDriver, 0, 0, 0, 0, 0, 0, 0))
					{
						CThreadLockHandle localHandle(g_cs);
						g_monitor_usb_task[pDriver] = FALSE;
					}
				}
			}
		}
		for (map<LPTSTR, BOOL>::iterator it = g_monitor_usb_task.begin(); it != g_monitor_usb_task.end();)
		{
			BOOL isInTmpTask = FALSE;
			for (auto item : m_tmp_usb_task)
			{
				if (_tcscmp(item, it->first) == 0)
				{
					isInTmpTask = TRUE;
					break;
				}
			}
			if (!isInTmpTask && it->second)
			{
				CThreadLockHandle localHandle(g_cs);
				g_monitor_usb_task.erase(it++);
			}
			else
			{
				it++;
			}
		}
		m_tmp_usb_task.clear();
		if (g_monitor_usb_task.size() && isNew)
		{
			isNew = FALSE;
			for (auto item : g_monitor_usb_task)
			{
				_tprintf(_T("%s "), item.first);
			}
			_tprintf(_T("\n"));
			SetEvent(g_hEvent);
		}
		Sleep(1000);
	}
	return 0;
}

DWORD WINAPI ThreadFunc_USBDriverPackage(LPVOID lpParam)
{
	while (true)
	{
		WaitForSingleObject(g_hEvent, INFINITE);
		Sleep(1000);
		map<LPTSTR, UINT>m_tmp_usb_task;
		for (auto item : g_monitor_usb_task)
		{
			if (!item.second)
			{
				m_tmp_usb_task[item.first] = 0;
			}
		}

		for (map<LPTSTR, UINT>::iterator it = m_tmp_usb_task.begin(); it != m_tmp_usb_task.end();)
		{
			_tprintf(_T("[Done] %s\n"), it->first);
			it++;
		}

		for (auto item : m_tmp_usb_task)
		{
			for (map<LPTSTR, BOOL>::iterator it = g_monitor_usb_task.begin(); it != g_monitor_usb_task.end();)
			{
				if (_tcscmp(item.first, it->first) == 0)
				{
					CThreadLockHandle localHandle(g_cs);
					it->second = TRUE;
				}
				it++;
			}
		}
		m_tmp_usb_task.clear();
		BOOL isAllDone = TRUE;
		for (auto item : g_monitor_usb_task)
		{
			if (!item.second)
			{
				isAllDone = FALSE;
			}
		}
		if (isAllDone)
		{
			ResetEvent(g_hEvent);
		}
	}
	return 0;
}
