# ModbusRAW

Output example:

```
/../cmake-build-debug/Modbus_RAW
-------------
Request RAW: 00 01 00 00 00 06 01 01 00 01 00 0F 

Transaction ID: 00 01
Protocol ID   : 00 00
Length        : 00 06
Unit ID       : 01

Function code : 01
Start address : 00 01
Data count    : 00 0F

-------------
Response RAW: 00 01 00 00 00 05 01 01 02 FF 7F 

Transaction ID: 00 01
Protocol ID   : 00 00
Length        : 00 05
Unit ID       : 01
Function code : 01
Data count    : 02 bytes


Process finished with exit code 0
```