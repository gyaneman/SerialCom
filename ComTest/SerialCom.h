#pragma once
#include <Windows.h>
#include <cstdio>

#define _CRT_SECURE_NO_WARNINGS


namespace Robot{
#define DEVICES_BUF_SIZE 1024
#define RECEIVE_BUF_SIZE 1024
	int getSerialPortNumbers(int *comPortTable, int num_max);

	class SerialCom
	{
	private:
		char portName[128];
		int portNum = 0;
		int boundRate = 9600;
		int writeBufSize = 0;
		int receiveBufLength = 0;
		HANDLE portHandle;
		char receiveBuffer[RECEIVE_BUF_SIZE];
		BYTE *writeBuffer;
		HANDLE threadHandle;
		int threadId = 0;
		HANDLE mutexHandle;
		bool threadExitFlag;
		static DWORD WINAPI threadFunc(LPVOID pthis);
		int threadMain();
	public:
		SerialCom();
		SerialCom(char *_portName, int _boundRate);
		SerialCom(char *_portName, int _boundRate, int _writeBufSize);
		~SerialCom();

		bool init(char *_portName, int _boundRate, int _writeBufSize);
		bool exit();
		void send(char *_transString);
		int read(char *_buf, int _bufSize);
		void clearReadBuffer();
		void testCom(char *retBuf, int retBufSize);
	};

}