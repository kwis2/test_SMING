#include <user_config.h>
#include <SmingCore/SmingCore.h>
#include <stddef.h>
#include "meas.h"
#include "PP_smartConfig.h"
//#include "http_serwer.h"
//#include "ds18b20.h"

#define meas_data_timer     10000
#define check_conn_timer    2000
#define send_data_timer     20000

#define SmartConfig

void sendData();

Timer connTimer;
Timer procTimer;
Timer measTimer;

char file_name[25];
file_t file_wynik;

FTPServer PProbesftp;
HttpServer PProbesServer;
int sensorValue = 0;

cnct_status connectStatus = CNCT_INIT;
int http_wyslane_start = 0;
int http_wyslane_pomiary = 0;


int get_last_fname(char *fname);
void get_fname(char *fname, int cnt);

void connectOk()
{
    Serial.println("I'm CONNECTED");

    PProbesftp.listen(21);
    PProbesftp.addUser("pprobes_admin", "pass_pprobes_admin"); // FTP account

//    startPProbesSerwer();
}

void check_conn()
{
    dbg_prnt("                                       check conn");
    if(WifiStation.isConnected())
    {
        connTimer.stop();
        if(connectStatus == CNCT_TRY_CONNECT)
        {
            connectOk();
        }
        connectStatus = CNCT_CONNECTED;
        procTimer.initializeMs(send_data_timer, sendData).start(); // every 10 seconds
    }
}

void DS18B20_get_T(void)
{
    Serial.println("odczyt T");
    uint8_t adres = 0;

    float temp = 123.34;

    char bufor[30], *pbuf = bufor;
    uint8_t buf_cnt;
    buf_cnt = sprintf(pbuf, "t");
    pbuf = bufor + buf_cnt;
    buf_cnt += sprintf_flo(pbuf, temp, 3);
    pbuf = bufor + buf_cnt;
    sprintf(pbuf, "______");

    bufor[9] = '\n';
    buf_cnt = dane_dlugosc;

    get_last_fname(file_name);

    file_wynik = fileOpen(file_name, eFO_CreateIfNotExist | eFO_WriteOnly | eFO_Append);
    fileWrite(file_wynik, bufor, buf_cnt);
    fileClose(file_wynik);
}

void measData()
{
    Serial.println("init T");
    DS18B20_get_T();
}

void sendData()
{
    //String cfg;
    Serial.println("tick ");

    if(WifiStation.isConnected())
    {
        connectStatus = CNCT_CONNECTED;
    }
    else
    {
        connectStatus = CNCT_CONN_LOST;
        procTimer.stop();
        connTimer.initializeMs(check_conn_timer, check_conn).start(); // every 1 seconds
        return;
    }
    // Read our sensor value :)
    sensorValue++;

    get_fname(file_name, 1);
    Serial.println(file_name);
    file_wynik = fileOpen(file_name, eFO_ReadOnly);
    char fbufor[max_file_size + 100], *pbufor_dane = fbufor;

    pbufor_dane += sprintf(fbufor, "test_", system_get_chip_id());

    int frcnt = fileRead(file_wynik, pbufor_dane, max_file_size);

    fileClose(file_wynik);

    *(pbufor_dane + frcnt) = 0;

    Serial.println(frcnt);
    Serial.println(fbufor);

    if(frcnt < 0)
    {
        return;
    }


    Serial.println(fbufor);
    http_wyslane_pomiary = trim_bufor(pbufor_dane, '\n', frcnt);
    if(http_wyslane_pomiary == 0)
    {
        fileDelete(file_name);
        Serial.print("delete ");
        Serial.println(file_wynik);
        char tmp_file_name[25];

        int tmp_file_nr = 1;
        while(true)
        {
            tmp_file_nr++;
            get_fname(tmp_file_name, tmp_file_nr - 1);
            get_fname(file_name, tmp_file_nr);

            if(fileExist(file_name))
            {
                Serial.print("rename ");
                Serial.print(file_name);
                Serial.print(" --> ");
                Serial.println(tmp_file_name);
                fileRename(file_name, tmp_file_name);
            }
            else
            {
                return;
            }
        }

    }
    Serial.println(fbufor);
    trim_bufor_dupl(pbufor_dane, '_', frcnt);
    Serial.print(http_wyslane_pomiary);
    Serial.print("   ");
    Serial.println(http_wyslane_start);
    Serial.println(fbufor);
}

void init()
{
    spiffs_mount(); // Mount file system, in order to work with files

    Serial.begin(SERIAL_BAUD_RATE); // 115200 by default
    Serial.systemDebugOutput(false); // Debug output to serial

    WifiAccessPoint.enable(false);
    WifiStation.enable(true);

    measTimer.initializeMs(meas_data_timer, measData).start(); // every 30 seconds

#ifdef SmartConfig
    if(fileExist(cfg_fname))
    {
        String cfg;
        String ssid, password;
        connectStatus = CNCT_FILE_EXIST;

        cfg = fileGetContent(cfg_fname);

        ssid = cfg.substring(0, cfg.indexOf(' '));

        password = cfg.substring(cfg.indexOf(' ') + 1, cfg.length());

        WifiStation.config(ssid, password);
        connectStatus = CNCT_TRY_CONNECT;
    }
    else
    {
        WifiStation.smartConfigStart(SCT_EspTouch, smartConfigCallback);
        connectStatus = CNCT_SMARTCONFIG;
    }
#else

    String ssid, password;
    ssid = "xxxxx";
    password = "yyyyy";
    WifiStation.config(ssid, password);
    connectStatus = CNCT_TRY_CONNECT;
#endif

    //procTimer.initializeMs(10000, sendData).start(); // every 10 seconds
    connTimer.initializeMs(check_conn_timer, check_conn).start(); // every 1 seconds

    //Serial.println(WifiStation.getIP().toString());
}

uint8_t sprintf_flo(char *bfr, float liczba, uint8_t zaokr)
{
    float mnoz = 1;
    uint8_t i;
    for(i = 0; i < zaokr; i++)
    {
        mnoz *= 10;
    }
    int itempH = liczba, itempL;
    float temp;
    temp = liczba - itempH;
    temp *= mnoz;
    itempL = temp + 0.5;
    i = sprintf(bfr, "%d.%03d", itempH, itempL);
    return (i);
}

void get_fname(char *fname, int cnt)
{
    sprintf(fname, "%s_%03d.dat", data_file_name, cnt);
}

int get_last_fname(char *fname)
{
    int cntr = 0, fsize;
    do
    {
        cntr++;
        get_fname(fname, cntr);
        //sprintf(fname, "%s_%03d.dat", data_file_name, cntr);
        Serial.println(fname);
        fsize = fileGetSize(fname);
        if(fsize > 32000)
        {
            fsize = 0;
        }
        // fileGetSize()
    } while(fsize > max_file_size - dane_dlugosc); //ograniczeni do max_file_size
    return (cntr);
}

int trim_bufor(char *bufor_adres, char znak2rmv, int lngth) //usuwa znaki znak2rmv z bufor_adres, zwraca ilosc znak2rmv
{
    char * pbufor_source = bufor_adres;
    char * pbufor_wyn = bufor_adres;
    char znak;
    int meas_cntr = 0;

    for(int i = 0; i <= lngth; i++)
    {
        znak = *pbufor_source++;
        if(znak == 0)
        {
            *pbufor_wyn++ = znak;
            return (meas_cntr);
        }
        if(znak != znak2rmv)
        {
            *pbufor_wyn++ = znak;
        }
        else
        {
            meas_cntr++;
            if(znak2rmv == '\n')
            {
                if(meas_cntr >= http_pomiar2send) //obcinamy stringa do ilosci pomiarow
                {
                    *pbufor_wyn++ = 0;
                    return (meas_cntr);
                }
            }
        }
    }
    return (2555);
}

int trim_bufor_dupl(char *bufor_adres, char znak2rmv, int lngth) //usuwa duplikaty znak2rmv z bufor_adres zwraca ilosc znakow
{
    char * pbufor_source = bufor_adres;
    char * pbufor_wyn = bufor_adres;
    char znak, znak2 = znak2rmv, znak3 = znak2rmv;
    int znak_cntr = 0;

    http_wyslane_start = 0;

    for(int i = 0; i < lngth; i++)
    {
        znak = *pbufor_source++;
        if((znak != znak2rmv) || ((znak2 == znak)&&(znak3 != znak)))
        {
            *pbufor_wyn++ = znak;
            znak_cntr++;
            if(http_wyslane_start == 0)
            {
                http_wyslane_start = i;
            }
        }
        if(znak == 0)
        {
            return (znak_cntr);
        }
        znak3 = znak2;
        znak2 = znak;
    }
    return (2555);
}