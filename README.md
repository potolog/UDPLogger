# UDPLogger
UDPLogger plots data which comes from the UDP socket. The signals in a UDP dataframe were defined in a .xlsx and will imported into the logger.

Features:
- import and export Settings
- save incomming data to file
- change buffer sizes to improve performance
- adding dynamically new plots
- x Axis values can be choosen

Supported signal types:
- char / int8_t
- uint8_t
- bool
- int16_t
- uint16_t
- int / int32_t
- uint32_t
- float
- double

![alt text](https://github.com/Murmele/UDPLogger/blob/master/res/images/MainView.png)

TODO:
- qgrip , resizing plots
- different plot types

Dependencies:
- xlnt: https://github.com/tfussell/xlnt
- netcdf_c++4: https://www.unidata.ucar.edu/downloads/netcdf/index.jsp

For debugging, the libraries must be installed with debug information

## Flatpak
flatpak-builder --repo=UDPLoggerRepo --force-clean UDPLogger com.github.Murmele.UDPLogger.json
flatpak remote-add UDPLoggerRepo UDPLoggerRepo --no-gpg-verify
flatpak install UDPLoggerRepo com.github.Murmele.UDPLogger
