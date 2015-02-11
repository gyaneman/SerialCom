#include "SerialCom.h"


namespace Robot{
	SerialCom::SerialCom()
	{
	}

	SerialCom::SerialCom(char *_portName, int _boundRate)
	{
		init(_portName, _boundRate, 255);
	}

	SerialCom::SerialCom(char *_portName, int _boundRate, int _writeBufSize)
	{
		init(_portName, _boundRate, _writeBufSize);
	}

	SerialCom::~SerialCom()
	{
		exit();
	}


	bool SerialCom::init(char *_portName, int _boundRate, int _writeBufSize)
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
		cto.ReadIntervalTimeout = 100;
		cto.ReadTotalTimeoutMultiplier = 0;
		cto.ReadTotalTimeoutConstant = 50;
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
		DWORD rb = 0;

		while (ReadFile(portHandle, buffer, bufLength, &readBytes, NULL))
		{
			if (readBytes > 0){
				WaitForSingleObject(mutexHandle, 0);

				if (receiveBufLength + 1 >= RECEIVE_BUF_SIZE){
					printf("receive buf overflow...\n");
					receiveBuffer[0] = '\0';
				}

				memcpy_s(receiveBuffer + receiveBufLength, RECEIVE_BUF_SIZE - receiveBufLength, buffer, readBytes);
				receiveBufLength = receiveBufLength + readBytes;

				exitFlag = threadExitFlag;
				ReleaseMutex(mutexHandle);
				if (exitFlag) break;
			}
		}
		return 0;
	}

	DWORD WINAPI SerialCom::threadFunc(LPVOID pthis)
	{
		((SerialCom*)pthis)->threadMain();
		ExitThread(0);
	}

	int SerialCom::read(char *_buf, int _bufSize)
	{
		WaitForSingleObject(mutexHandle, 0);
		//printf("%d\n", receiveBufLength);
		memcpy_s(_buf, _bufSize, receiveBuffer, receiveBufLength);
		_buf[receiveBufLength] = '\0';
		int ret = receiveBufLength;
		receiveBufLength = 0;
		ReleaseMutex(mutexHandle);
		return ret;
	}

	void SerialCom::clearReadBuffer()
	{
		while (receiveBufLength != 0)
		{
			receiveBufLength = 0;
		}
	}

	void SerialCom::send(char *_transString)
	{
		DWORD writeBytes = 0;
		int index = 0;
		DWORD toWriteBytes = strlen(_transString);
		//printf("%d\n", toWriteBytes);
		while (toWriteBytes > 0)
		{
			WriteFile(portHandle, _transString + index, toWriteBytes, &writeBytes, NULL);
			index += writeBytes;
			toWriteBytes -= writeBytes;
		}
		return;
	}

	bool SerialCom::exit()
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
		strncpy_s(retBuf, retBufSize, buffer, readBytes + 1);
	}

	/* この関数を使うことでPCの空いているCOMポートがわかるお */
	int getSerialPortNumbers(int *_comPortTable, int _num_max)
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
					_comPortTable[comPorts++] = atoi(p + 3);
					if (comPorts >= _num_max)
						break;
				}

				p += strlen(p) + 1;
			}
		}

		return comPorts;
	}
}