# LTOEnc

LTOEnc – LTO Encryption Management is a small program to manage encryption on your LTO Tapes on Microsoft Windows. It supports LTO4 drives and higher. It can co-exist with LTFS drivers.

It has been tested with a HP Ultrium5 3000, IBM ULT3580-HH5 and IBM ULTRIUM 7 HH Tape Drives.

The program is based on [STENC](https://github.com/scsitape/stenc)

### Usage examples

```
LTOEnc /?
```
Displays help text



```
LTOEnc /k mykey.txt /g 256
```
Generates a new key in mykey.txt with the length of 256 Bits



```
LTOEnc /k mykey.txt /e mixed
```
Turn on tape encryption in mixed mode (reads encrypted and unencrypted data from tape, writes encrypted data)



```
LTOEnc /s /e off
```
Turn off tape encryption



```
LTOEnc /volumestatus
```
Displays status of drive and tape


### Prerequisites

* Any C++ compiler (I use Visual Studio 2017, but GCC on MingW should work too)

### Note

All these tools are provided as-is from [Vulpes](https://vulpes.lu).
If you've some questions, contact me [here](https://go.vulpes.lu/contact).
