#include "laserurg.h"
#include "itrsystem.h"
#include "iostream"

using namespace std;

unsigned char _start_cmd[20] = {0x4d, 0x44, /*MS*/
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
const int MAX_DATALENGTH = 768;
int _data1[MAX_DATALENGTH];
int _data2[MAX_DATALENGTH];
int _length1;
int _length2;
int *_data;
int *_length;

bool _data_which;
int StepA;
int StepB;
int StepC;

void LaserInit(char *dev, int baudrate)
{
    unsigned char tmp = 0;
    _sp.Init(dev, baudrate);
    _sp.Send(_reset_cmd, _reset_cmd_length);//RESET THE URG

    _sp.Receive(&tmp, 1);
    while (tmp != 0x0a)
    {
        _sp.Receive(&tmp, 1);
    }
    _sp.Receive(&tmp, 1);
    while (tmp != 0x0a)
    {
        _sp.Receive(&tmp, 1);
    }
    _sp.Receive(&tmp, 1);
    while (tmp != 0x0a)
    {
        _sp.Receive(&tmp, 1);
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
    unsigned char tmp[128];
    int index = 0;
    int count_lf = 0;

    while (1)
    {
        _sp.Receive(tmp, 2);
        if (tmp[0] == 'M' && tmp[1] == 'D')
        {
            while (tmp[0] != 0x0a)
            {
                _sp.Receive(tmp, 1);
            }
            _sp.Receive(tmp, 4);//读取99blf
            if (tmp[0] == '9' && tmp[1] == '9' && tmp[2] == 'b')
            {
                while (tmp[0] != 0x0a)
                {/* timestamp*/
                    _sp.Receive(tmp, 1);
                }

                if (_data_which)
                {
                    _data = _data1;
                    _length = &_length1;
                }
                else
                {
                    _data = _data2;
                    _length = &_length2;
                }
                index = 0;
                count_lf = 0;
                *_length = 0;

                _sp.Receive(tmp, 1);
                while (1)/*read data -include sum*/
                {
                    while (tmp[0] != 0x0a)
                    {
                        count_lf = 0;
                        *(_data + index) = tmp[0];
                        index++;
                        _sp.Receive(tmp, 1);
                    }
                    count_lf++;
                    if (count_lf == 2)
                        break;
                    else
                        _sp.Receive(tmp, 1);
                }
                /*decode */
                S32 factor[3] = {0};
                int k = 0;
                for (S32 j = 0; j < index; j++)
                {
                    if (j % 65 != 64)
                    {
                        factor[k] = _data[j] - 0x30;
                        k++;
                        if (k % 3 == 0)
                        {
                            _data[*_length] = factor[2] + (factor[1]) << 6 + (factor[0]) << 12;
                            (*_length)++;
                            k = 0;
                        }
                    }
                    else
                    {;//do nothing with SUM
                    }
                    index++;
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
                _sp.Send(_reset_cmd, _reset_cmd_length);//RESET THE URG
                _sp.Receive(tmp, 1);
                while (tmp[0] != 0x0a)
                {
                    _sp.Receive(tmp, 1);
                }
            }
        }
        else
        {
            printf("cmd received is not started with MD\n");
            _sp.Send(_reset_cmd, _reset_cmd_length);//RESET THE URG
            _sp.Receive(tmp, 1);
            while (tmp[0] != 0x0a)
            {
                _sp.Receive(tmp, 1);
            }
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
