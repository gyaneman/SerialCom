#include "SerialCom.h"

int main()
{
	int comPortTable[255];
	Robot::SerialCom com;
	char c;
	int count = 0;
	char buf[1024];

	if (!com.init("COM3", 38400, 255))
	{
		int numOfComPorts = Robot::getSerialPortNumbers(comPortTable, 255);
		for (int i = 0; i < numOfComPorts; i++)
		{
			printf("COM%d\n", comPortTable[i]);
		}
		printf("ƒL[‚ð‰Ÿ‚µ‚Ä‚­‚¾‚³‚¢\n");
		while (getchar());
		return 0;
	}
	
	while (true)
	{
		int len = com.read(buf, 1024);

		if (len != 0)
		{
			printf("%s", buf);
		}

		com.send("b");
		Sleep(100);
	}
	
	if (!com.exit())
	{
		printf("error\n");
		printf("press any key\n");
		while (getchar());
	}

	return 0;
}