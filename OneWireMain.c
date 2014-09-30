/*
 * File:   OneWireMain.c
 * Author: NevenSperanda
 *
 * Created on 2013. studeni 29, 16:01
 */


#include <xc.h>

#pragma config OSC = INTIO67   //internal ocsillator
#pragma config WDT = OFF            //turn off WDT
#pragma config LVP = OFF
#pragma config PBADEN = 0


#ifndef _XTAL_FREQ
#define _XTAL_FREQ 8000000 //8Mhz FRC internal osc
#define __delay_us(x) _delay((unsigned long)((x)*(_XTAL_FREQ/4000000.0)))
#define __delay_ms(x) _delay((unsigned long)((x)*(_XTAL_FREQ/4000.0)))
#endif
#define OneWireWritePin LATB0
#define OneWireReadPin RB0
#define OneWirePinDirection TRISB0
#define true 1
#define false 0
#define Input 1
#define Output 0
#define ReadROM 0x33
#define MatchROM 0x55
#define SkipROM 0xCC
#define SearchROM 0xF0
#define AlarmSearch 0xEC
#define ConvertT 0x44
#define ReadScratchpad 0xBE
#define WriteScratchpad 0x4E
#define CopyScratchpad 0x48
#define RecallEE 0xB8
#define ReadPowerSupply 0xB4
#define DevicesNumber 2


static unsigned char DeviceAddressMap[DevicesNumber][8];//mapa svih rom adresa uređaja



void driveOneWireLow(void) {
    OneWirePinDirection = Output;
    OneWireWritePin = false;
}

void driveOneWireHigh(void) {
    OneWirePinDirection = Output;
    OneWireWritePin = true;
}

unsigned char sampleOneWirePin(void) {
    unsigned char result = 0;
    OneWirePinDirection = Input;
    result = OneWireReadPin;
    return result;
}

unsigned char readOneWireBit(void) {
    unsigned char result = 0x00;

    driveOneWireLow();
    __delay_us(6);
    driveOneWireHigh();
    __delay_us(9);
    result = sampleOneWirePin();
    __delay_us(55);
    return result;
}

void writeOneWireBit(unsigned char WriteArg){
    if (WriteArg==1){
        driveOneWireLow();
        __delay_us(6);
        driveOneWireHigh();
        __delay_us(64);
    }else{
        driveOneWireLow();
        __delay_us(60);
        driveOneWireHigh();
        __delay_us(10);
    }
}

unsigned char resetOneWirePin(void) {
    unsigned char presence;
    driveOneWireLow();
    __delay_us(240);
    __delay_us(240);
    driveOneWireHigh();
    __delay_us(70);
    presence = sampleOneWirePin();
    __delay_us(205);
    __delay_us(205);
    driveOneWireHigh();
    return presence; //zero if present; one if not present
}

void writeOneWireByte(unsigned char data){
    for(int i=0;i<8;i++){
        writeOneWireBit(data & 0x01);
        data>>=1;
    }
}

unsigned char readOneWireByte(void){
    unsigned char data=0;
    for (int i=0;i<8;i++){
        data>>=1;
        if(readOneWireBit()){
        data|=0x80;
        }


    }
    return data;
}

unsigned char addressOneWireDevice(unsigned char CurrentAdressedOneWireDevice[]){
   if( resetOneWirePin()==0){
       writeOneWireByte(MatchROM);
       for(int i=0;i<8;i++){
           writeOneWireByte(CurrentAdressedOneWireDevice[i]);
       }
       return 1; //sucessful
   }else {
       return 0;//non sucesfull
   }
}

unsigned char searchOneWireDevicesROM(void){
    unsigned char SearchedROMDevice[8];
    unsigned char LastConflictBitNum=0;
    for(int i=0;i<DevicesNumber;i++){
         unsigned char mask=0x01;
         for(int k=0;k<8;k++){
         SearchedROMDevice[k]=0x00;
         }
         if (resetOneWirePin()){
             return 0;
         }
         writeOneWireByte(SearchROM);
         int bitnum=1;
         unsigned char CurrentConflictBitPos=0;
         while(bitnum<65){
             unsigned char SelectedBit=0;
             unsigned char InputBitStatus=0x00;
             if (readOneWireBit()==1){
                 InputBitStatus|=2;
             }
             if(readOneWireBit()==1){
                 InputBitStatus|=1;
             }
        /* description for values of InputBitStatus: */
        /* 00    There are devices connected to the bus which have conflicting */
        /*       bits in the current ROM code bit position. */
        /* 01    All devices connected to the bus have a 0 in this bit position. */
        /* 10    All devices connected to the bus have a 1 in this bit position. */
        /* 11    There are no devices connected to the 1-wire bus. */
             if (InputBitStatus==3){
                 return 0;
             }
             if(InputBitStatus>0){
                 SelectedBit=InputBitStatus>>1;
             }else{
                 if(bitnum==LastConflictBitNum){
                     SelectedBit=1;
                 }
                 else{
                     SelectedBit=0;
                     CurrentConflictBitPos=bitnum;
                 }

             }
             writeOneWireBit(SelectedBit);
             mask=0x01;
             mask<<=(bitnum-1)%8;
             if (SelectedBit==1){
             SearchedROMDevice[(bitnum-1)/8]|=mask;
                  }
                 bitnum++;
             }
         LastConflictBitNum=CurrentConflictBitPos;
         for(int j=0;j<8;j++){
         DeviceAddressMap[i][j]=SearchedROMDevice[j];
         }
    }
    return 1;
}






void delay100ms(void) {
    __delay_ms(10);
    __delay_ms(10);
    __delay_ms(10);
    __delay_ms(10);
    __delay_ms(10);
    __delay_ms(10);
    __delay_ms(10);
    __delay_ms(10);
    __delay_ms(10);
    __delay_ms(10);
}

void delay50ms(void) {
__delay_ms(10);
    __delay_ms(10);
    __delay_ms(10);
    __delay_ms(10);
    __delay_ms(10);

}
void delay750ms(void) {
    delay100ms();
    delay100ms();
    delay100ms();
    delay100ms();
    delay100ms();
    delay100ms();
    delay100ms();
    delay50ms();
}
void main(void) {
    OSCCON = 0x73;
    unsigned char SearchROMDevicesSucess;
SearchROMDevicesSucess=searchOneWireDevicesROM();

    while (1) {





        unsigned char prisutnost = 0;
        unsigned char templ1,temph1,templ2,temph2;
        int temp1,temp2;
        prisutnost = resetOneWirePin();

        TRISB1 = 0;
        TRISB2=0;
        if (prisutnost == 0) { //prisutan device
            
           
            addressOneWireDevice(DeviceAddressMap[0]);
           
            writeOneWireByte(ConvertT);
            delay750ms();
            resetOneWirePin();
            addressOneWireDevice(DeviceAddressMap[0]);
            writeOneWireByte(ReadScratchpad);
            templ1= readOneWireByte();
            temph1=readOneWireByte();
            temp1=temph1;
            temp1<<=8;
            temp1|=templ1;
            temp1=temp1;
            if (temp1>0x16D){
            LATB1 = 1;}
            else{
                LATB1 = 0;
            }
            if(temp2>0x16D){
                LATB2=1;
            }else{
                LATB2=0;
            }

            addressOneWireDevice(DeviceAddressMap[1]);

            writeOneWireByte(ConvertT);
            delay750ms();
            resetOneWirePin();
            addressOneWireDevice(DeviceAddressMap[1]);
            writeOneWireByte(ReadScratchpad);
            templ2= readOneWireByte();
            temph2=readOneWireByte();
            temp2=temph2;
            temp2<<=8;
            temp2|=templ2;
            temp2=temp2;
            
        } else {
           
        }
        //delay100ms();




    }
}
