#define _CRT_SECURE_NO_WARNINGS
#include <cstdio>
#include <string>
#include <Windows.h>

#define DEVICES_BUF_SIZE 65535
#define RECEIVE_BUF_SIZE 1024

namespace SerialCom
{
	class SerialCom
	{
	private:
		char portName[128];
		int portNum = 0;
		int boundRate = 9600;
		int receiveBufLength = 0;
		int writeBufSize = 0;
		HANDLE portHandle;
		char receiveBuffer[RECEIVE_BUF_SIZE];
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
		int threadMain();
		int getData(char *buf, int bufSize);
		void testCom(char *retBuf, int retBufSize);
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
		threadExitFlag = false;
		mutexHandle = CreateMutex(NULL, TRUE, NULL);
		ResumeThread(threadHandle);

		return true;
	}

	int SerialCom::threadMain()
	{
		bool exitFlag;
		const DWORD bufLength = 1024;
		char buffer[bufLength];
		DWORD readBytes = 0;
		
		while (ReadFile(portHandle, buffer, bufLength, &readBytes, NULL))
		{
			if (readBytes > 0){
				WaitForSingleObject(mutexHandle, 0);
				buffer[readBytes] = '\0';
				receiveBufLength = receiveBufLength + readBytes - 1;
				if (receiveBufLength + 1 >= RECEIVE_BUF_SIZE){
					printf("receive buf overflow...\n");
					receiveBufLength = readBytes - 1;
					receiveBuffer[0] = '\0';
				}
				strcat(receiveBuffer, buffer);
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
		((SerialCom*)pthis)->threadMain();
		ExitThread(0);
	}

	int SerialCom::getData(char *buf, int bufSize)
	{
		WaitForSingleObject(mutexHandle, 0);
		strncpy_s(buf, bufSize, receiveBuffer, RECEIVE_BUF_SIZE);
		receiveBuffer[0] = '\0';
		receiveBufLength = 0;
		ReleaseMutex(mutexHandle);
		return receiveBufLength;
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
		}
		else{
			printf("ポートを閉じることができませんでした. \n");
			return false;
		}
		CloseHandle(mutexHandle);
		WaitForSingleObject(mutexHandle, 0);
		threadExitFlag = true;
		ReleaseMutex(mutexHandle);

		return true;
	}

	void SerialCom::testCom(char *retBuf, int retBufSize)
	{
		char buffer[1024];
		DWORD toReadBytes = 1024;
		DWORD readBytes;
		ReadFile(portHandle, buffer, toReadBytes, &readBytes, NULL);
		buffer[readBytes] = '\0';
		printf("%s\n", buffer);
		strncpy_s(retBuf, retBufSize, buffer, readBytes+1);
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
	char c;
	char buf[1024];
	printf("program running\n");
	if (com.initComPort("COM3", 9600, 255))
	{
		printf("init true\n");
	}
	
	while ((c = getchar()) != 'q')
	{
		com.getData(buf, 1024);
		//com.testCom(buf, 1024);
		printf("%s", buf);
	}
	
	if (com.exitComPort())
	{
		printf("nomally exited\n");
	}

	return 0;
}