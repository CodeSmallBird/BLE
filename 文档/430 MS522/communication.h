
char SpiRxData,SpiRxData2;
char ReadRx;
char CardData[16],CardID[15],SAKData,TagType[2],TagType2[2];
char CardF[18];
char CarfATQB[16],CardCID;
char MFCrypto,ProtocolErr;

char CRCResult1[2],CRCResult2[2];
char ReceiveData1[64],ReceiveData2[64];
char NFCID3i[10];

#define SPEED_106 0x00
#define SPEED_212 0x10
#define SPEED_424 0x20
#define SPEED_848 0x30
#define SPEED_1696 0x40
#define SPEED_3392 0x50

#define ACTIVE  0x01
#define PASSIVE 0x01
