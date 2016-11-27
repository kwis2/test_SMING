/* meas.h
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   meas.h
 * Author: pomiar
 *
 * Created on 7 listopada 2016, 09:58
 */

#ifndef MEAS_H
#define MEAS_H
//#include "ds18b20.h"

#define cfg_fname "cfg_net.dat"
#define data_file "wynik.dat"
#define data_file_name "wynik"
#define dane_dlugosc        10
#define http_pomiar2send    20  //dane_dlugosc * http_pomiar2send <= 200
#define max_file_size       1000

typedef enum
{
  CNCT_INIT = 0,
  CNCT_SMARTCONFIG,
  CNCT_FILE_EXIST,
  CNCT_TRY_CONNECT,
  CNCT_CONNECTED_INIT,
  CNCT_CONNECTED,
  CNCT_CONN_LOST,

} cnct_status;

extern cnct_status connectStatus;

uint8_t sprintf_flo(char *bfr, float liczba, uint8_t zaokr);

#define dbg_prnt Serial.println

int trim_bufor(char *bufor_adres, char znak2rmv, int lngth); //usuwa znaki znak2rmv z bufor_adres, zwraca ilosc znak2rmv
int trim_bufor_dupl(char *bufor_adres, char znak2rmv, int lngth); //usuwa duplikaty znak2rmv z bufor_adres zwraca ilosc znakow

#endif /* MEAS_H */

