# TEKHNĒ Wi-Fi HaLow Modules Standalone SDK Package

## Notice

> [!CAUTION]
> The software package released here is specifically designed for the TEKHNĒ NRC7394 Modules.
> If users plan to use this software package with other devices that incorporate the NRC7394 chip, they should utilize the board data file supplied by the device's vendor.

### Get the detailed user guide
Please refer to [UG-7394-004-Standalone SDK.pdf](https://github.com/tekhne-web/wifi-halow-standalone-sdk/blob/master/package/doc/UG-7394-004-Standalone%20SDK.pdf) in doc directory.

Inside the *package/make* directory you can find all the Makefile options available for TEKHNĒ NRC7394 Modules.

For example, to build the *sample_tcp_client* for the TEKHNĒ MYNA EU8 Module you can do it by executing: 

```
make select target=myna_eu8.sdk.release APP_NAME=sample_tcp_client
make
```
