#SerialCom

Windowsでシリアル通信を行うプログラムです. 言語はC++.  
バグあるかもね.

##開発環境
VisualStudio2013

##使い方
SerialCom.hとSerialCom.cppを適当なところにおいてください. 

##ドキュメント
ただし適当です.

--------------------------------------
```c++
Robot::SerialCom( const char *_portName, const char *_boundRate)
```

####内容
SerialComオブジェクトの初期設定を行います.

####引数  
* \_portName...使用するポートの名前(例: "COM3")  
* \_boundRate...バウンドレート(例: 9600)  

--------------------------------------
```c++
void Robot::SerialCom::send(char *transString)
```
####内容
SerialComコンストラクタで設定したCOMポートに文字列を送信します.
####引数
* transString...送信する文字列(終端はNULLでないと死にます)

####戻り値  
なし

----------------------------------------
```c++
int Robot::SerialCom::read(char *buf, int bufSize)
```
####内容
SerialComコンストラクタで設定したCOMポートから送信されたデータを取得します.

####引数
* buf...受信したデータを格納するメモリのアドレス
* bufSize...bufのサイズ

####戻り値
bufに格納したデータのバイト数

-------------------------
```c++
void SerialCom::clearReadBuffer()
```
####内容
受信用バッファをクリアします.

####引数
なし

####戻り値
なし

-----------------------------------------
```c++
int Robot::getSerialPortNumbers(int *comPortTable, int num_max)
```
####内容
使用できるCOMポートナンバーを取得します.

####引数
* comPortTable...使用できるポートナンバーを格納する配列のポインタ
* num_max...comPortTableに格納するデータ数の上限

####戻り値
使用可能なCOMポートの数


##使用例
使用可能なポートを調べる例.
```c++
#include <cstdio>
#include "SerialCom.h"

int main()
{
    int comPortTable[255];
    int numOfComPorts = Robot::getSerialPortNumbers(comPortTable, 255);
    for (int i = 0; i < numOfComPorts; i++)
    {
        printf("COM%d\n", comPortTable[i]);
    }
    printf("キーを押してください\n");
    while (getchar());
    return 0;
}
```

受信プログラムの例.
```c++
#include <cstdio>
#include <Windows.h>
#include "SerialCom.h"

int main()
{
    Robot::SerialCom com("COM3", 9600);
    char buf[255];
    int count = 0;
    while(count < 10)
    {
        int len = com.read(buf, 255);
        if(len != 0)
        {
            printf("%s", buf);
        }
        count++;
        Sleep(100);
    }
    com.exit();
    return 0;
}
```

送信プログラムの例.
```c++
#include <cstdio>
#include <Windows.h>
#include "SerialCom.h"

int main()
{
    Robot::Serial com("COM3", 9600);
    char buf[255];
    int count = 0;
    while(count < 10)
    {
        sprintf(buf, "%d", count);
        com.send(buf);
    }
    com.exit();
    return 0;
}
```
