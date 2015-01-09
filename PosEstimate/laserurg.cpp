#include "laserurg.h"
#include "itrsystem.h"
#include "iostream"

using namespace std;

unsigned char _start_cmd[20] = {0x4d, 0x44, /*MD*/
        0x30, 0x30, 0x30, 0x30,/*start step 0*/
        0x30, 0x37, 0x36, 0x38, /*end step 768*/
        0x30, 0x30, /*cluster counter 1*/
        0x30, /*scan interval 0*/
        0x30, 0x30,/*number of scan 0*/
        0x0A};
/*lf*/
const int _start_cmd_length = 16;
unsigned char _reset_cmd[4] = "RS\n";
const int _reset_cmd_length = 3;

Process onRec = NULL;
pthread_t tid;

itr_system::SerialPort _sp;
const int MAX_DATALENGTH = 769;
int _data1[MAX_DATALENGTH * 3];
int _data2[MAX_DATALENGTH * 3];
int _length1;
int _length2;
int *_data;
int *_length;

bool _data_which;
int StepA;
int StepB;
int StepC;

const int BUFFER_LENGTH = 128;
unsigned char CHAR_BUFFER[BUFFER_LENGTH];
int read_length = 0;

void LaserInit(char *dev, int baudrate)
{

    _sp.Init(dev, baudrate);
    _sp.Send(_reset_cmd, _reset_cmd_length);//RESET THE URG

    read_length = _sp.ReadLine(CHAR_BUFFER, BUFFER_LENGTH);
    while (!(CHAR_BUFFER[0] == 'R' && CHAR_BUFFER[1] == 'S'))//or set time delay 250ms
    {
        _sp.ReadLine(CHAR_BUFFER, BUFFER_LENGTH);
    }
    read_length = _sp.ReadLine(CHAR_BUFFER, BUFFER_LENGTH);
    read_length = _sp.ReadLine(CHAR_BUFFER, BUFFER_LENGTH);
    _sp.Send((unsigned char *) "SCIP2.0\n", 8);
    int i = 3;
    while (i--)
    {
        read_length = _sp.ReadLine(CHAR_BUFFER, BUFFER_LENGTH);
    }

    _data_which = true;
    _length1 = 0;
    _length2 = 0;
    onRec = NULL;
    StepA = 0;
    StepB = 384;
    StepC = 768;
}

void LaserSetProcess(Process process)
{
    onRec = process;
}

void *WorkThread(void *)
{
    /*cmd to start urg */
    _sp.Send(_start_cmd, _start_cmd_length);
    int index = 0, i = 0, j = 0, k = 0;

    int factor[3] = {0};

    while (1)
    {
        read_length = _sp.ReadLine(CHAR_BUFFER, BUFFER_LENGTH);
        if (CHAR_BUFFER[0] == 'M' && CHAR_BUFFER[1] == 'D')
        {
            //read_length = _sp.ReadLine(CHAR_BUFFER, BUFFER_LENGTH);
            read_length = _sp.ReadLine(CHAR_BUFFER, BUFFER_LENGTH);//读取99blf

            if (CHAR_BUFFER[0] == '9' && CHAR_BUFFER[1] == '9' && CHAR_BUFFER[2] == 'b')
            {
                read_length = _sp.ReadLine(CHAR_BUFFER, BUFFER_LENGTH);//Time Stamp

                index = 0;
                read_length = _sp.ReadLine(CHAR_BUFFER, BUFFER_LENGTH);
                //shuang huan chong chuli
                if (_data_which)
                {
                    _data = _data1;
                    _length = &_length1;
                } else
                {
                    _data = _data2;
                    _length = &_length2;
                }
                while (read_length > 1)/*read data -include sum*/
                {
                    if (read_length <= 3)
                    {
                        printf("data error 02:\n");
                        break;
                    }
                    else {
                        for (i = 0; i < read_length - 2; i++) {
                            _data[index] = CHAR_BUFFER[i];
                            index++;
                        }
                        read_length = _sp.ReadLine(CHAR_BUFFER, BUFFER_LENGTH);
                    }

                }

                if (index > MAX_DATALENGTH * 3)
                    printf("MAX_DATALENGTH not enough!\n");

                /*decode */
                k = 0;
                *_length = 0;
                for (j = 0; j < index; j++)
                {
                    factor[k] = _data[j] - 0x30;
                    k++;
                    if (k % 3 == 0)
                    {
                        _data[*_length] = factor[2] | (factor[1] << 6) | (factor[0] << 12);
                        (*_length)++;
                        k = 0;
                    }
                }
                _data_which = !_data_which;

                if (onRec != NULL)
                {
                    onRec(_data, *_length);
                }
                //return _data *_length
            }
            else
            {
                printf("status is not 99b\n");
                if (CHAR_BUFFER[0] == '0' && CHAR_BUFFER[1] == '0' && CHAR_BUFFER[2] == 'P')
                    printf("status if 00P\n");
                // read_length = _sp.ReadLine(CHAR_BUFFER, BUFFER_LENGTH);
                /*   _sp.Send(_reset_cmd, _reset_cmd_length);//RESET THE URG
                   _sp.Receive(CHAR_BUFFER, 1);
                   while (CHAR_BUFFER[0] != 0x0a)
                   {
                       _sp.Receive(CHAR_BUFFER, 1);
                   }*/
            }
        }
        else
        {
            printf("cmd received is not started with MD\n");
            /*   _sp.Send(_reset_cmd, _reset_cmd_length);//RESET THE URG
               _sp.Receive(CHAR_BUFFER, 1);
               while (CHAR_BUFFER[0] != 0x0a)
               {
                   _sp.Receive(CHAR_BUFFER, 1);
               }*/
        }
    }
}

void LaserStart()
{
    pthread_create(&tid, NULL, WorkThread, NULL);
}

void LaserStop()
{
    _sp.Close();
    pthread_cancel(tid);
}
