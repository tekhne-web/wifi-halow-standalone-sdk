/*
 * MIT License
 *
 * Copyright (c) 2022 Newracom, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 */

#include "nrc_sdk.h"
#include "wifi_config_setup.h"
#include "wifi_connect_common.h"


#define MAX_RETRY 10

/******************************************************************************
 * FunctionName : run_sample_wifi_state
 * Description  : sample test for wifi connect & disconnect
 * Parameters   : WIFI_CONFIG
 * Returns      : 0 or -1 (0: success, -1: fail)
 *******************************************************************************/
nrc_err_t run_sample_wifi_state(WIFI_CONFIG *param)
{
	SCAN_RESULTS results;
	int count = 0;
	int interval = 0;
	int retry_count = 0;
	int ssid_found =false;
	int i = 0;
	AP_INFO *ap_info = NULL;

	nrc_usr_print("[%s] Sample App for Wi-Fi State \n",__func__);
	nrc_usr_print(" - ssid : %s\n", param->ssid);
	nrc_usr_print(" - security : ");
	if (param->security_mode == WIFI_SEC_OPEN) {
		nrc_usr_print("open\n");
	} else if (param->security_mode == WIFI_SEC_WPA2) {
		nrc_usr_print("wpa2\n");
	} else if (param->security_mode == WIFI_SEC_WPA3_OWE) {
		nrc_usr_print("wpa3-owe\n");
	} else if (param->security_mode == WIFI_SEC_WPA3_SAE) {
		nrc_usr_print("wpa3-sae\n");
	} else {
		nrc_usr_print("unknown\n");
	}
	nrc_usr_print(" - password : %s\n",
				  (param->security_mode == WIFI_SEC_WPA2) || (param->security_mode == WIFI_SEC_WPA3_SAE) ? (char *)param->password : "");

	count = 10;
	interval =3000;

	/* set initial wifi configuration */
	while(1){
		if (wifi_init(param)== WIFI_SUCCESS) {
			nrc_usr_print ("[%s] wifi_init Success !! \n", __func__);
			break;
		} else {
			nrc_usr_print ("[%s] wifi_init Failed !! \n", __func__);
			_delay_ms(1000);
		}
	}

	/* find AP */
	while(1){
		if (nrc_wifi_scan(0) == WIFI_SUCCESS){
			if (nrc_wifi_scan_results(0, &results)== WIFI_SUCCESS) {
				/* Find the ssid in scan results */
				for(i=0; i<results.n_result ; i++){
					if((strcmp((char*)param->ssid, (char*)results.result[i].ssid)== 0)
					   && (results.result[i].security == param->security_mode)){
						ssid_found = true;
						break;
					}
				}

				if(ssid_found){
					nrc_usr_print ("[%s] %s is found \n", __func__, param->ssid);
					break;
				}
			}
		} else {
			nrc_usr_print ("[%s] Scan fail !! \n", __func__);
			_delay_ms(1000);
		}
	}

	/* connect to AP */
	while(1) {
		if (wifi_connect(param)== WIFI_SUCCESS) {
			nrc_usr_print ("[%s] connect to %s successfully !! \n", __func__, param->ssid);
			uint8_t	txpower = 0;
			nrc_wifi_get_tx_power(0, &txpower);
			nrc_usr_print("[%s] TX Power (%d dBm)\n", __func__, txpower);
			break;
		} else{
			nrc_usr_print ("[%s] Fail for connection %s\n", __func__, param->ssid);
			_delay_ms(1000);
		}
	}

	/* check if IP is ready */
	while(1){
		if (nrc_addr_get_state(0) == NET_ADDR_SET) {
			nrc_usr_print("[%s] IP ...\n",__func__);
			break;
		} else {
			nrc_usr_print("[%s] IP Address setting State : %d != NET_ADDR_SET(%d) yet...\n",
						  __func__, nrc_addr_get_state(0), NET_ADDR_SET);
		}
		_delay_ms(1000);
	}

	ap_info = nrc_mem_malloc(sizeof(AP_INFO));
	if (ap_info) {
		if (nrc_wifi_get_ap_info(0, ap_info) == WIFI_SUCCESS) {
			nrc_usr_print("[%s] AP ("MACSTR" %s (len:%d) %c%c bw:%d ch:%d freq:%d security:%d)\n",
				__func__, MAC2STR(ap_info->bssid), ap_info->ssid, ap_info->ssid_len,
				ap_info->cc[0],ap_info->cc[1], ap_info->bw, ap_info->ch, ap_info->freq,
				ap_info->security);
		} else {
			nrc_usr_print("[%s] Fail to get AP INFO\n",__func__);
			nrc_mem_free(ap_info);
			return NRC_FAIL;
		}
	} else {
		nrc_usr_print("[%s] Fail to alloc mem\n", __func__);
		return NRC_FAIL;
	}

	if (ap_info) {
		nrc_mem_free(ap_info);
	}

	for(i = 0 ; i < count ; i++) {
		/* Repete connection and disconnection every 3 seconds */
		_delay_ms(interval);
		if (nrc_wifi_get_state(0) == WIFI_STATE_CONNECTED) {
			nrc_usr_print("[%s] Trying to DISCONNECT...\n",__func__);
			if (nrc_wifi_disconnect(0, 5000) != WIFI_SUCCESS) {
				nrc_usr_print ("[%s] Fail for Wi-Fi disconnection\n", __func__);
				return -1;
			}
			_delay_ms(2000);
		} else if (nrc_wifi_get_state(0) == WIFI_STATE_DISCONNECTED) {
			nrc_usr_print("[%s] Trying to CONNECT... (cnt:%d)\n",__func__, i);
			for(retry_count=0; retry_count<MAX_RETRY; retry_count++){
				if (wifi_connect(param)!= WIFI_SUCCESS) {
					nrc_usr_print ("[%s] Fail for Wi-Fi connection, retry:%d\n", __func__, retry_count);
					_delay_ms(1000);
				}else{
					nrc_usr_print ("[%s] Success for Wi-Fi connection\n", __func__);
					retry_count = 0;
					break;
				}
			}
			if(retry_count == MAX_RETRY)
				return -1;

			/* check if IP is ready */
			while(1){
				if (nrc_addr_get_state(0) == NET_ADDR_SET) {
					nrc_usr_print("[%s] IP ...\n",__func__);
					break;
				} else {
					nrc_usr_print("[%s] IP Address setting State : %d != NET_ADDR_SET(%d) yet...\n",
								  __func__, nrc_addr_get_state(0), NET_ADDR_SET);
				}
				_delay_ms(1000);
			}
		}
	}

	if (nrc_wifi_get_state(0) == WIFI_STATE_CONNECTED) {
		nrc_usr_print("[%s] Trying to DISCONNECT... for exit\n",__func__);
		if (nrc_wifi_disconnect(0, 5000) != WIFI_SUCCESS) {
			nrc_usr_print ("[%s] Fail for Wi-Fi disconnection\n", __func__);
			return -1;
		}
	}

	nrc_usr_print("[%s] End of run_sample_wifi_state!! \n",__func__);

	return 0;
}

/******************************************************************************
 * FunctionName : user_init
 * Description  : Start Code for User Application, Initialize User function
 * Parameters   : none
 * Returns      : none
 *******************************************************************************/
WIFI_CONFIG wifi_config;
WIFI_CONFIG* param = &wifi_config;

void user_init(void)
{
	memset(param, 0x0, WIFI_CONFIG_SIZE);
	nrc_usr_print("[%s] \n", __func__);

	nrc_wifi_set_config(param);
	run_sample_wifi_state(param);
}
