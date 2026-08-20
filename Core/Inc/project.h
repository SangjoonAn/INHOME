#ifndef PROJECT_H
#define PROJECT_H


#include "common.h"


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

    } Control1;

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

    } Control2;
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

    u8 IsoLevel;
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
    s16 SleepModeLimitLevel;         
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
    u8  Reserve119;                    // 119

    // 120~129
    u8  Reserve120;                    // 120
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
    u8  UlRfSwOnOff;                   // 136
    s8  IsolationSetting;              // 137
    s8  OscSetting;                    // 138

    s16 TxSdTTime; // 139 ~ 140
    s16 RxSdTTime; // 141 ~ 142
    s16 UlFbDet; // 143 ~ 144
    s8 SleepReleaseOffset; // 145
    u8 TxAlcAtt; // 146
    u8 RxAlcAtt; // 147
    u8 SubVersion; // 148

    u8  Reserve149[11];                    // 139
} MY_STATE_t;
#pragma pack(pop)

#pragma pack(push,1)
typedef struct
{
    // 000 ~ 008
    struct
    {
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
    } Control;

    // 018 ~ 026
    struct
    {
        union
        {
            u8 Data;
            struct
            {
                u8 TxAtt           :1;
                u8 RxAtt           :1;
                u8 TxLinkBalance   :1;
                u8 RxLinkBalance   :1;
                u8 IsoAtt          :1;
                u8 Reserved        :3;

            } Bit;

        } Flag;


        u8 TxAtt;
        u8 RxAtt;

        u8 TxLinkBalanceAtt;
        u8 RxLinkBalanceAtt;

        u8 IsoAtt;

        u8 Reserved26[3];
    } System;


    // 027 ~ 035
    struct
    {
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
        } Flag;
        s8 TxShutdownLimit;
        s8 RxShutdownLimit;

        s8 TxAlcHighLevel;
        s8 RxAlcHighLevel;

        s8 TxAlcLowOffset;
        s8 RxAlcLowOffset;

        s8 SleepModeLimit;

        u8 Reserved35;

    } Level;
    // 036 ~ 053
    u8 InitCheckNum;
    u16 PreAlarmSts;
    u8 Reserved[15];
    u8 Reserved54[75];
} MY_CONTROL_t;
#pragma pack(pop)


extern MY_STATE_t      iMySts;
extern MY_CONTROL_t    iMyCtrl;


#endif