/* smartConfig.cpp
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
#include <user_config.h>
#include <SmingCore/SmingCore.h>

#include "meas.h"

void smartConfigCallback(sc_status status, void *pdata)
{

    switch(status)
    {
        case SC_STATUS_WAIT:
            debugf("SC_STATUS_WAIT\n");
            break;
        case SC_STATUS_FIND_CHANNEL:
            debugf("SC_STATUS_FIND_CHANNEL\n");
            break;
        case SC_STATUS_GETTING_SSID_PSWD:
            debugf("SC_STATUS_GETTING_SSID_PSWD\n");
            break;
        case SC_STATUS_LINK:
        {
            char bufor[30];
            debugf("SC_STATUS_LINK\n");
            station_config *sta_conf = (station_config *) pdata;
            char *ssid = (char*) sta_conf->ssid;
            char *password = (char*) sta_conf->password;

            sprintf(bufor, "%s %s", ssid, password);

            fileSetContent(cfg_fname, bufor);
            WifiStation.config(ssid, password);
            connectStatus=CNCT_TRY_CONNECT;
        }
            break;
        case SC_STATUS_LINK_OVER:
            debugf("SC_STATUS_LINK_OVER\n");
            WifiStation.smartConfigStop();
            break;
    }
}


