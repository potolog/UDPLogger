UDPLogger plots data which comes from the UDP socket. The signals in a UDP dataframe were defined in a .xlsx and will imported into the logger.

Features:
- import and export Settings
- save incomming data to file
- change buffer sizes to improve performance
- adding dynamically new plots
- x Axis values can be choosen

![alt text](https://github.com/Murmele/UDPLogger/blob/master/res/images/MainView.png)

TODO:
- qgrip , resizing plots
- different plot types
- export plots

Dependencies:
- xlnt: https://github.com/tfussell/xlnt
- netcdf_c++4: https://www.unidata.ucar.edu/downloads/netcdf/index.jsp

For debugging, the libraries must be installed with debug information
