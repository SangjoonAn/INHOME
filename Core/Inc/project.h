#ifndef PROJECT_H
#define PROJECT_H


#include "common.h"

#define SYS_FREQ_900M       0
#define SYS_FREQ_18G        1


#define TX_900M_MAX_GAIN    114     // 57dB
#define RX_900M_MAX_GAIN    114

#define TX_18G_MAX_GAIN     120      //60dB
#define RX_18G_MAX_GAIN     120

extern u32 gCurTimerTick;



#pragma pack(push,1)
typedef struct
{
    // 000~009

    u8 RptMaker;
    u8 McuSwVer;
    u8 SysTemper;
    u8 Reserve003[2];
   
    union
    {
        u8 Data;
        struct
        {
            u8 OscAlarm         :1;
            u8 TxShutdownAlarm  :1;
            u8 RxShutdownAlarm  :1;
            u8 SleepAlarm       :1;
            u8 Reserved         :4;
        } Bit;

    } Alarm;

    u8 Reserve006;

    union
    {
        u8 Data;
        struct
        {
            u8 TxAlc         :1;
            u8 RxAlc         :1;
            u8 TxShutdown    :1;
            u8 RxShutdown    :1;
            u8 Reserved      :4;
        } Bit;

    } Flag1;

    union
    {
        u8 Data;
        struct
        {
            u8 TxPath         :1;
            u8 RxPath         :1;
            u8 IsoCheck       :1;
            u8 IsoReCheck     :1;
            u8 SleepMode      :1;
            u8 IsoLimitRun    :1;
            u8 Reserved       :2;
        } Bit;

    } Flag2;


    u8 Reserve009;

    // 010~019

    s16 TxInputPower;
    s16 TxOutputPower;

    u8 Reserve014;
    u8 Reserve015;

    s16 RxOutputPower;

    u8 TxGain;
    u8 RxGain;

    u8 Reserve020[7];
	
    // 027
    s8 TxShutdownLimit;
    s8 RxShutdownLimit;

    s8 TxAlcHighLevel;
    s8 RxAlcHighLevel;

    s8 TxAlcLowOffset;
    s8 RxAlcLowOffset;

    s8 SleepModeLimit;

    u8 Reserve034[11];

    // 045
    u8 TxGainAtt;
    u8 RxGainAtt;

    u8 TxLinkBalanceAtt;
    u8 RxLinkBalanceAtt;

    u8 IsoAtt;

    // 050~059

    u8 TxMaxGain;
    u8 RxMaxGain;

    u8 IsoMeasure;
    u8 IsoMsg;

    u8 SystemRunTime[5];

    u8 Stability;

    // 060 ~ 085
    u8 Reserve060[26]; // 060 ~ 085
    u8 PLLAlarm; // 086
    u8 OscOnOff; // 087
    u8 Reserve088; // 088
    s16 UlfbOffset; // 089 ~ 090

    // 090 ~ 099
    u8 Reserve091[2]; // 091 ~ 092
    u8 OscCheckCount; // 093
    u8 SdBySd; // 094
    u8 SdByIso; // 095
    u8 SdByUe; // 096
    u8 SignalDetect; // 097
    s16 SleepValue; // 098 ~ 099
	//u8 Reserve059[40];


    // 100~109
    u16 TxDetVoltage;
    u16 RxDetVoltage;
    u16 SleepModeVoltage;
    s16 SysTemperVoltage;            // 108~109

    // 110~119
    s8  TxPowerOffsetInput;            // 110
    s8  TxPowerOffsetOutput;           // 111
    s8  RxPowerOffsetInput;            // 112
    s8  RxPowerOffsetOutput;           // 113
    u8  TxAttGainOffset;               // 114
    s8  TxAttBalanceOffset;            // 115
    s8  RxAttGainOffset;               // 116
    s8  RxAttBalanceOffset;             // 117
    u8  RxAttIsoOffset;                // 118
    u8  TemperatureComp;                    // 119

    // 120~129
    u8  TemperatureOffset;                    // 120
    s8  TxAttGainStep;                 // 121
    s8  TxAttBalanceStep;              // 122
    s8  RxAttGainStep;                 // 123
    s8  RxAttBalanceStep;              // 124
    u8  RxAttIsoStep;                  // 125
    s8  TxAttGainTemp;                 // 126
    s8  TxAttBalanceTemp;              // 127
    s8  RxAttGainTemp;                 // 128
    s8  RxAttBalanceTemp;              // 129


    /* 130~139 --------------------------------------------------- */

    s8  RxAttIsoTemp;                  // 130
    s8  TxAttGainTotal;                // 131
    s8  TxAttBalanceTotal;              // 132
    s8  RxAttGainTotal;                 // 133
    s8  RxAttBalanceTotal;              // 134
    s8  RxAttIsoTotal;                  // 135
    u8  UlRfSw;                   // 136
    s8  IsoLimitLevel;              // 137
    s8  OscLevelLimit;                    // 138

    s16 TxSdTime; // 139 ~ 140
    s16 RxSdTime; // 141 ~ 142
    s16 UlFbDet; // 143 ~ 144
    s8 SleepReleaseOffset; // 145
    u8 TxAlcAtt; // 146
    u8 RxAlcAtt; // 147
    u8 SubVersion; // 148

    u8 TxSubAtt;
    u8 RxSubAtt;
    u8 Test_ModeAtt;
    u8 Test_TxAtt1;
    u8 Test_RxAtt1;
    u8 Test_RxAtt2;
    u8 SysFreq;
    s16 IsoThreshold;
    u8  Reserve149[2];                    // 139
} MY_STATE_t;
#pragma pack(pop)

#pragma pack(push,1)
typedef struct
{
    // 000 ~ 008

    union
    {
        u8 Data;
        struct
        {
            u8 TxAlc           :1;
            u8 RxAlc           :1;
            u8 TxShutdown      :1;
            u8 RxShutdown      :1;
            u8 SleepMode       :1;
            u8 IsoCheck        :1;
            u8 IsoReCheck      :1;
            u8 Reserved        :1;
        } Bit;
    } Flag1;
    u8 TxAlc;
    u8 RxAlc;
    u8 TxShutdown;
    u8 RxShutdown;
    u8 SleepMode;
    u8 IsoCheck;
    u8 IsoReCheck;
    u8 Reserved08;

    // 009 ~ 017
    union
    {
        u8 Data;
        struct
        {
            u8 TxPath          :1;
            u8 RxPath          :1;
            u8 SystemReset     :1;
            u8 IsoLimitRun     :1;
            u8 Reserved        :4;

        } Bit;

    } Flag2;

    u8 TxPath;
    u8 RxPath;
    u8 SystemReset;
    u8 IsoLimitRun;
    u8 Reserved17[4];

    // 018 ~ 026
    union
    {
        u8 Data;
        struct
        {
            u8 TxGainAtt           :1;
            u8 RxGainAtt           :1;
            u8 TxLinkBalanceAtt   :1;
            u8 RxLinkBalanceAtt   :1;
            u8 IsoAtt          :1;
            u8 Reserved        :3;

        } Bit;

    } Flag3;


    u8 TxGainAtt;
    u8 RxGainAtt;

    u8 TxLinkBalanceAtt;
    u8 RxLinkBalanceAtt;

    u8 IsoAtt;

    u8 Reserved26[3];



    // 027 ~ 035
    union
    {
        u8 Data;
        struct
        {
            u8 TxShutdownLimit :1;
            u8 RxShutdownLimit :1;
            u8 TxAlcHighLevel  :1;
            u8 RxAlcHighLevel  :1;
            u8 TxAlcLowOffset  :1;
            u8 RxAlcLowOffset  :1;
            u8 SleepModeLimit  :1;
            u8 Reserved        :1;
        } Bit;
    } Flag4;
    s8 TxShutdownLimit;
    s8 RxShutdownLimit;

    s8 TxAlcHighLevel;
    s8 RxAlcHighLevel;

    s8 TxAlcLowOffset;
    s8 RxAlcLowOffset;

    s8 SleepModeLimit;

    u8 Reserved35[4];

    // 036 ~ 053
    union 
    { 
        u8 Data; 
        struct 
        { 
            u8 TxSubAtt :1;         /* 0 : 변경 */ 
            u8 RxSubAtt :1;         /* 1 : 변경 */ 
            u8 Test_ModeAtt :1;     /* 2 : 변경 */ 
            u8 Test_TxAtt1 :1;      /* 3 : 변경 */ 
            u8 Test_RxAtt1 :1;      /* 4 : 변경 */ 
            u8 Test_RxAtt2 :1;      /* 5 : 변경 */ 
            u8 SysFreq     :1;     /* 6 : 변경 */ 
            u8 IsoThreshold :1;      /* 7 */ 
        } Bit; 
    } Flag8;
    u8 TxSubAtt;
    u8 RxSubAtt;
    u8 Test_ModeAtt;
    u8 Test_TxAtt1;
    u8 Test_RxAtt1;
    u8 Test_RxAtt2;

    u8 SysFreq;
    s16 IsoThreshold;
    u8 InitCheckNum;
    u8 Reserved49[49];

    union 
    { 
        u8 Data; 
        struct 
        { 
            u8 TxPowerOffsetInput :1; /* 0 : 변경 */ 
            u8 TxPowerOffsetOutput :1; /* 1 : 변경 */ 
            u8 RxPowerOffsetInput :1; /* 2 : 변경 */ 
            u8 RxPowerOffsetOutput :1; /* 3 : 변경 */ 
            u8 TemperatureComp :1; /* 4 : 변경 */ 
            u8 TemperatureOffset :1; /* 5 : 변경 */ 
            u8 TxAttGainOffset :1; /* 6 : 변경 */ 
            u8 Reserved :1; /* 7 */ 
        } Bit; 
    } Flag5;

    union 
    { 
        u8 Data; 
        struct 
        { 
            u8 Reserved0 :1; /* 0 : 변경 */ 
            u8 Reserved1 :1; /* 1 : 변경 */ 
            u8 RxAttIsoOffset :1; /* 2 : 변경 */ 
            u8 UlRfSw :1; /* 3 : 변경 */ 
            u8 IsoLimitLevel :1; /* 4 : 변경 */ 
            u8 OscLevelLimit :1; /* 5 : 변경 */ 
            u8 TxSdTime :1; /* 6 : 변경 */ 
            u8 RxSdTime :1; /* 7 */ 
        } Bit; 
    } Flag6;

    s8 TxPowerOffsetInput; /* 101 */ 
    s8 TxPowerOffsetOutput; /* 102 */ 
    s8 RxPowerOffsetInput; /* 103 */ 
    s8 RxPowerOffsetOutput; /* 104 */ 
    u8 TemperatureComp; /* 105 */ 
    s8 TemperatureOffset; /* 106 */ 
    u8 TxAttGainOffset; 

    u8 Reserved108[3];
    u8 RxAttIsoOffset;
    u8 UlRfSw;
    s8 IsoLimitLevel;
    s8 OscLevelLimit;
    u16 TxSdTime;
    u16 RxSdTime;

    union 
    { 
        u8 Data; 
        struct 
        { 
            u8 SleepReleaseOffset :1; /* 0 : 변경 */ 
            u8 TxAlcAtt :1; /* 1 : 변경 */ 
            u8 RxAlcAtt :1; /* 2 : 변경 */ 
            u8 OscOnOff :1; /* 3 : 변경 */ 
            u8 Reserved4 :1; /* 4 : 변경 */ 
            u8 UlFbOffset :1; /* 5 : 변경 */ 
            u8 Reserved6 :1; /* 6 : 변경 */ 
            u8 Reserved7 :1; /* 7 */ 
        } Bit; 
    } Flag7;
    s8 SleepReleaseOffset;
    u8 TxAlcAtt;
    u8 RxAlcAtt;
    u8 OscOnOff;
    u8 Reserved124[2];
    s16 UlFbOffset;
    u8 Reserved128[2];

} MY_CONTROL_t;
#pragma pack(pop)


extern MY_STATE_t      iMySts;
extern MY_CONTROL_t    iMyCtrl;


#endif