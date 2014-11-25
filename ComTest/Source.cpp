#include <cstdio>
#include <algorithm>
#include <queue>
#include <string>
#include <iostream>
#include <Windows.h>

#define DEVICES_BUF_SIZE 65535

namespace SerialCom
{
	class SerialCom
	{
	private:
		char portName[128];
		int portNum = 0;
		int boundRate = 9600;
		int readBufSize = 0;
		int writeBufSize = 0;
		HANDLE portHandle;
		BYTE *readBuffer;
		BYTE *writeBuffer;
		HANDLE threadHandle;
		int threadId = 0;
		HANDLE mutexHandle;
		bool threadExitFlag;
	public:
		bool initComPort(char *_portName, int _boundRate, int _writeBufSize);
		bool startComPort();
		bool exitComPort();
		int sendComData(char *transString);
		int readComData(char *readBuf);
		static DWORD WINAPI threadFunc(LPVOID pthis);
		int threadRoutin();
		//bool setPortName(string _portName);
		//bool setBoundRate(int _boundRate);
		//bool setBufSize(int _bufSize);
	};

	bool SerialCom::initComPort(char *_portName, int _boundRate, int _writeBufSize)
	{
		strncpy_s(portName, _portName, 128);
		boundRate = _boundRate;
		writeBufSize = _writeBufSize;

		//portNum = atoi(portName + 3);

		if ((portHandle = CreateFile((LPCSTR)portName, 
			GENERIC_READ | GENERIC_WRITE, 
			0, NULL, OPEN_EXISTING, 0, NULL))
			== INVALID_HANDLE_VALUE)
		{
			printf("%sを開くことができません. ポートの名前を確認してください. \n", portName);
			return false;
		}

		DCB config;
		GetCommState(portHandle, &config);
		config.BaudRate = boundRate;
		SetCommState(portHandle, &config);

		COMMTIMEOUTS cto;
		GetCommTimeouts(portHandle, &cto);
		cto.ReadIntervalTimeout = 1000;
		cto.ReadTotalTimeoutMultiplier = 0;
		cto.ReadTotalTimeoutConstant = 1000;
		cto.WriteTotalTimeoutMultiplier = 0;
		cto.WriteTotalTimeoutConstant = 0;
		SetCommTimeouts(portHandle, &cto);

		threadHandle = CreateThread(NULL, 0, SerialCom::threadFunc, (LPVOID)this, CREATE_SUSPENDED, (LPDWORD)&threadId);
		//mutexHandle = CreateMutex(NULL, TRUE, NULL);
		//threadExitFlag = false;

		return true;
	}

	int SerialCom::threadRoutin()
	{
		bool exitFlag;
		const DWORD bufLength = 1024;
		BYTE buffer[bufLength];
		DWORD readBytes = 0;
		
		while (ReadFile(portHandle, buffer, bufLength, &readBytes, NULL))
		{
			if (readBytes > 0){
				readBuffer = new BYTE[readBytes];
				readBufSize = readBytes;
				WaitForSingleObject(mutexHandle, 0);
				memcpy_s(readBuffer, readBytes, buffer, bufLength);
				exitFlag = threadExitFlag;
				ReleaseMutex(mutexHandle);
				if (exitFlag)
					break;
			}
		}
		return 0;
	}

	DWORD WINAPI SerialCom::threadFunc(LPVOID pthis)
	{
		printf("ok!\n");
		ExitThread(0);
		//((SerialCom*)pthis)->threadRoutin();
		ExitThread(0);
	}

	bool SerialCom::exitComPort()
	{
		if (threadHandle != NULL)
		{
			CloseHandle(threadHandle);
		}
		if (portHandle == NULL)
		{
			printf("ポートを閉じることができませんでした. \n");
			return false;
		}
		if (CloseHandle(portHandle))
		{
			portHandle = NULL;
			return true;
		}
		else{
			printf("ポートを閉じることができませんでした. \n");
			return false;
		}
		CloseHandle(mutexHandle);
		WaitForSingleObject(mutexHandle, 0);
		threadExitFlag = true;
		ReleaseMutex(mutexHandle);
	}

	int getSerialPortNumbers(int *comPortTable, int num_max)
	{
		char devicesBuff[DEVICES_BUF_SIZE];
		int comPorts = 0;
		HMODULE h;

		if (((h = GetModuleHandle("kernel32.dll")) != NULL) &&
			(GetProcAddress(h, "QueryDosDeviceA") != NULL) &&
			(QueryDosDevice(NULL, devicesBuff, DEVICES_BUF_SIZE) != 0))
		{
			char *p = devicesBuff;
			while (*p != '\0')
			{
				if (strncmp(p, "COM", 3) == 0 && p[3] != '\0')
				{
					comPortTable[comPorts++] = atoi(p + 3);
					if (comPorts >= num_max)
						break;
				}

				p += strlen(p) + 1;
			}
		}

		return comPorts;
	}

}


int main()
{
	int comPortTable[255];
	int numOfComPorts = SerialCom::getSerialPortNumbers(comPortTable, 255);
	SerialCom::SerialCom com;
	printf("program running\n");
	if (com.initComPort("COM4", 9600, 255))
	{
		printf("init true\n");
	}

	if (com.exitComPort())
	{
		printf("exit true\n");
	}

	for (int i = 0; i < numOfComPorts; i++)
	{
		printf("%d\n", comPortTable[i]);
	}

	while (true);
	return 0;
}