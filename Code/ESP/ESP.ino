#include "ConfigESP.h"
#include "DS3231.h"
#include <Wire.h>
#include <ESP8266WiFi.h>
#include <FirebaseArduino.h>
#include <OneWire.h>
#include <Adafruit_NeoPixel.h>

#define FIREBASE_HOST "hashirama-project-7638a.firebaseio.com"
#define FIREBASE_AUTH "a28hwe1MUsGk2Oi0MFn3EEB3GjVo1c9hN0WAhweF"

#define WIFI_SSID "SR Hub Old"
#define WIFI_PASSWORD "longprolam"

#define ReceivedBufferLength 15
char cmdReceivedBuffer[ReceivedBufferLength + 1];
byte cmdReceivedBufferIndex;
unsigned int controlMode = 0;

int motorSpeed;
double nowEC, nowTDS, nowPH, nowTemp;
float nowMois;
unsigned long int nowLux;
double desireEC, desireTDS, desirePH;

boolean autoState = false, oldAutoState = false;
boolean sentFlag = false, doneFlag = false, ledOn = false, decoLedOn = false;

String stringEC = "EC:";
String stringPH = "PH:";
String stringToSend;

bool h12;
bool PM;
bool Century = false;

unsigned long int ecRec,phRec,tempRec,moisRec,luxRec;

// rtc module
DS3231 rtc;
// rtc module variables
int second, minute, hour, date, month, year;
float temperature;

Adafruit_NeoPixel ledStrip = Adafruit_NeoPixel(NUM_LEDS, LEDPIN, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel ledRing = Adafruit_NeoPixel(NUM_DECOLEDS, DECOLEDPIN, NEO_GRB + NEO_KHZ800);

void setup(){
    Wire.begin();
    Serial.begin(9600);
    //pinMode(15, OUTPUT);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.print("connecting");
    while (WiFi.status() != WL_CONNECTED){
        Serial.print(".");
        delay(500);
    }
    Serial.println();
    Serial.print("connected: ");
    Serial.println(WiFi.localIP());
    Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
    delay(500);
    Serial.swap();
    //Rtc automatically obtains the computer system time to initialize the RTC module
    rtc.setup();
    stringToSend = String();
    Firebase.setBool("isEspOnline", true);
    Firebase.setBool("onlineQuerry",false);
    if(Firebase.getBool("AutoState")){
        command(10);
    }
    Firebase.setInt("ControlMode", 0);
    Firebase.setBool("RelayState/DecoLED", false);
    Firebase.setBool("RelayState/DissolvePump", false);
    Firebase.setBool("RelayState/LED", false);
    Firebase.setBool("RelayState/PumpA", false);
    Firebase.setBool("RelayState/PumpB", false);
    Firebase.setBool("RelayState/PumpPH", false);
    Firebase.setBool("RelayState/PumpWater", false);
    Firebase.setBool("RelayState/Solenoid1", false);
    Firebase.setBool("RelayState/Solenoid2", false);
    Firebase.setBool("RelayState/Solenoid3", false);
    motorSpeed = Firebase.getInt("CurrentState/MotorSpeed");

    ecRec = Firebase.getInt("ECHistory/ecRecord");
    phRec = Firebase.getInt("pHHistory/phRecord");
    tempRec = Firebase.getInt("TempHistory/tempRecord");

    ledStrip.setBrightness(BRIGHTNESS_LED);
    ledRing.setBrightness(BRIGHTNESS_DECOLED);
    ledStrip.begin();
    ledRing.begin();
    ledStrip.show();
    ledRing.show();
}

void loop(){
    if(Firebase.getBool("onlineQuerry")){
        Firebase.setBool("isEspOnline", true);
        Firebase.setBool("onlineQuerry", false);
    }

    controlMode = Firebase.getInt("ControlMode");
    if (controlMode != 0){
        if(autoState){
            Serial.println("I'm in auto mode and can't do manual stuff!");
            if(controlMode == 21){
                command(controlMode);
            } else if (controlMode == 10){
                Serial.println("System is alreaday in auto mode");
            }
        }else{
            command(controlMode);
        }
        Firebase.setInt("ControlMode", 0);
    }

    if(autoState){
        if (cmdSerialDataAvailable() > 0){
            commandAlt(cmdParse());
        }
    }

    if (cmdSerialDataAvailable() > 0){
        //Serial.println(cmdReceivedBuffer);
        commandAlt(cmdParse());
    }

    if (decoLedOn){
        openDecoLed();
    }
}

boolean cmdSerialDataAvailable(){
    char cmdReceivedChar;
    static unsigned long cmdReceivedTimeOut = millis();
    while (Serial.available() > 0){
        if (millis() - cmdReceivedTimeOut > 500U){
            cmdReceivedBufferIndex = 0;
            memset(cmdReceivedBuffer, 0, (ReceivedBufferLength + 1));
        }
        cmdReceivedTimeOut = millis();
        cmdReceivedChar = Serial.read();
        if (cmdReceivedChar == '\n' || cmdReceivedBufferIndex == ReceivedBufferLength){
            cmdReceivedBufferIndex = 0;
            strupr(cmdReceivedBuffer);
            return true;
        }
        else{
            cmdReceivedBuffer[cmdReceivedBufferIndex] = cmdReceivedChar;
            cmdReceivedBufferIndex++;
        }
    }
    return false;
}

void command(byte mode){
    switch (mode){
    case 1:
        Serial.println("SENALL");
        sentFlag = false;
        while(1){
            if (cmdSerialDataAvailable() > 0){
                commandSensor(cmdSensor());
                Serial.println("DONE");
            }
            if(sentFlag){
                break;
            }
        }

        break;
    case 2:
        Serial.println("SENEC");
        while(1){
            if (cmdSerialDataAvailable() > 0){
                commandSensor(cmdSensor());
                Serial.println("DONE");
                break;
            }
        }
        break;
    case 3:
        Serial.println("SENPH");
        while (1){
            if (cmdSerialDataAvailable() > 0){
                commandSensor(cmdSensor());
                Serial.println("DONE");
                break;
            }
        }
        break;
    case 4:
        Serial.println("SENTEMP");
        while (1){
            if (cmdSerialDataAvailable() > 0){
                commandSensor(cmdSensor());
                Serial.println("DONE");
                break;
            }
        }
        break;
    case 5:
        Serial.println("SENMOIS");
        while (1){
            if (cmdSerialDataAvailable() > 0){
                commandSensor(cmdSensor());
                Serial.println("DONE");
                break;
            }
        }
        break;
    case 6:
        Serial.println("SENLUX");
        while (1){
            if (cmdSerialDataAvailable() > 0){
                commandSensor(cmdSensor());
                Serial.println("DONE");
                break;
            }
        }
        break;
    case 7:
        break;
    case 8:
        break;
    case 9:
        motorSpeed = Firebase.getInt("SetState/MotorSpeed");
        switch (motorSpeed){
            case 0:
                Serial.println("STOPMOTOR");
                break;
            case 1:
                Serial.println("OPENMOTORCW1");
                break;
            case 2:
                Serial.println("OPENMOTORCW2");
                break;
            case 3:
                Serial.println("OPENMOTORCW3");
                break;
            case 4:
                Serial.println("OPENMOTORCW4");
                break;
            case 5:
                Serial.println("OPENMOTORCW5");
                break;
            case 6:
                Serial.println("OPENMOTORCW6");
                break;
            case -1:
                Serial.println("OPENMOTORCCW1");
                break;
            case -2:
                Serial.println("OPENMOTORCCW2");
                break;
            case -3:
                Serial.println("OPENMOTORCCW3");
                break;
            case -4:
                Serial.println("OPENMOTORCCW4");
                break;
            case -5:
                Serial.println("OPENMOTORCCW5");
                break;
            case -6:
                Serial.println("OPENMOTORCCW6");
                break;
        }
        while(1){
            if (cmdSerialDataAvailable() > 0){
                if (strstr(cmdReceivedBuffer, "DONE") != NULL){
                    Firebase.setInt("CurrentState/MotorSpeed", motorSpeed);
                    break;
                }
            }
        }
        break;
    case 10:
        autoState = true;
        break;
    case 11:
        Serial.println("OPENPA");
        while (1){
            if (cmdSerialDataAvailable() > 0){
                if (strstr(cmdReceivedBuffer, "DONE") != NULL){
                    Firebase.setBool("RelayState/PumpA", true);
                    break;
                }
            }
        }
        break;
    case 12:
        Serial.println("OPENPB");
        while (1){
            if (cmdSerialDataAvailable() > 0){
                if (strstr(cmdReceivedBuffer, "DONE") != NULL){
                    Firebase.setBool("RelayState/PumpB", true);
                    break;
                }
            }
        }
        break;
    case 13:
        Serial.println("OPENPPH");
        while (1){
            if (cmdSerialDataAvailable() > 0){
                if (strstr(cmdReceivedBuffer, "DONE") != NULL){
                    Firebase.setBool("RelayState/PumpPH", true);
                    break;
                }
            }
        }
        break;
    case 14:
        Serial.println("OPENPW");
        while (1){
            if (cmdSerialDataAvailable() > 0){
                if (strstr(cmdReceivedBuffer, "DONE") != NULL){
                    Firebase.setBool("RelayState/PumpWater", true);
                    break;
                }
            }
        }
        break;
    case 15:
        Serial.println("OPENPD");
        while (1){
            if (cmdSerialDataAvailable() > 0){
                if (strstr(cmdReceivedBuffer, "DONE") != NULL){
                    Firebase.setBool("RelayState/DissolvePump", true);
                    break;
                }
            }
        }
        break;
    case 16:
        Serial.println("OPENS1");
        while (1){
            if (cmdSerialDataAvailable() > 0){
                if (strstr(cmdReceivedBuffer, "DONE") != NULL){
                    Firebase.setBool("RelayState/Solenoid1", true);
                    break;
                }
            }
        }
        break;
    case 17:
        Serial.println("OPENS2");
        while (1){
            if (cmdSerialDataAvailable() > 0){
                if (strstr(cmdReceivedBuffer, "DONE") != NULL){
                    Firebase.setBool("RelayState/Solenoid2", true);
                    break;
                }
            }
        }
        break;
    case 18:
        Serial.println("OPENS3");
        while (1){
            if (cmdSerialDataAvailable() > 0){
                if (strstr(cmdReceivedBuffer, "DONE") != NULL){
                    Firebase.setBool("RelayState/Solenoid3", true);
                    break;
                }
            }
        }
        break;
    case 19:
        ledOn = true;
        openLedWhite();
        Firebase.setBool("RelayState/LED", true);
        break;
    case 20:
        decoLedOn = true;
        openDecoLed();
        Firebase.setBool("RelayState/DecoLED", true);
        break;
    case 21:
        autoState = false;
        break;
    case 22:
        Serial.println("CLOSEPA");
        while (1){
            if (cmdSerialDataAvailable() > 0){
                if (strstr(cmdReceivedBuffer, "DONE") != NULL){
                    Firebase.setBool("RelayState/PumpA", false);
                    break;
                }
            }
        }
        break;
    case 23:
        Serial.println("CLOSEPB");
        while (1){
            if (cmdSerialDataAvailable() > 0){
                if (strstr(cmdReceivedBuffer, "DONE") != NULL){
                    Firebase.setBool("RelayState/PumpB", false);
                    break;
                }
            }
        }
        break;
    case 24:
        Serial.println("CLOSEPPH");
        while (1){
            if (cmdSerialDataAvailable() > 0){
                if (strstr(cmdReceivedBuffer, "DONE") != NULL){
                    Firebase.setBool("RelayState/PumpPH", false);
                    break;
                }
            }
        }
        break;
    case 25:
        Serial.println("CLOSEPW");
        while (1){
            if (cmdSerialDataAvailable() > 0){
                if (strstr(cmdReceivedBuffer, "DONE") != NULL){
                    Firebase.setBool("RelayState/PumpWater", false);
                    break;
                }
            }
        }
        break;
    case 26:
        Serial.println("CLOSEPD");
        while (1){
            if (cmdSerialDataAvailable() > 0){
                if (strstr(cmdReceivedBuffer, "DONE") != NULL){
                    Firebase.setBool("RelayState/DissolvePump", false);
                    break;
                }
            }
        }
        break;
    case 27:
        Serial.println("CLOSES1");
        while (1){
            if (cmdSerialDataAvailable() > 0){
                if (strstr(cmdReceivedBuffer, "DONE") != NULL){
                    Firebase.setBool("RelayState/Solenoid1", false);
                    break;
                }
            }
        }
        break;
    case 28:
        Serial.println("CLOSES2");
        while (1){
            if (cmdSerialDataAvailable() > 0){
                if (strstr(cmdReceivedBuffer, "DONE") != NULL){
                    Firebase.setBool("RelayState/Solenoid2", false);
                    break;
                }
            }
        }
        break;
    case 29:
        Serial.println("CLOSES3");
        while (1){
            if (cmdSerialDataAvailable() > 0){
                if (strstr(cmdReceivedBuffer, "DONE") != NULL){
                    Firebase.setBool("RelayState/Solenoid3", false);
                    break;
                }
            }
        }
        break;
    case 30:
        ledOn = false;
        closeLedWhite();
        Firebase.setBool("RelayState/LED", false);
        break;
    case 31:
        decoLedOn = false;
        closeDecoLed();
        Firebase.setBool("RelayState/DecoLED", false);
        break;
    } 
}

void commandAlt(byte mode){
    switch(mode){
    case 22:
        Firebase.setBool("RelayState/PumpA", false);
        break;
    case 23:
        Firebase.setBool("RelayState/PumpB", false);
        break;
    case 24:
        Firebase.setBool("RelayState/PumpPH", false);
        break;
    case 25:
        Firebase.setBool("RelayState/PumpWater", false);
        break;
    case 27:
        Firebase.setBool("RelayState/Solenoid1", false);
        break;
    case 28:
        Firebase.setBool("RelayState/Solenoid2", false);
        break;

    case 2:
        while (1){
            if (cmdSerialDataAvailable() > 0){
                commandSensor(cmdSensor());
                Serial.println("DONE");
                break;
            }
        }
        break;
    case 3:
        while (1){
            if (cmdSerialDataAvailable() > 0){
                commandSensor(cmdSensor());
                Serial.println("DONE");
                break;
            }
        }
        break;

    case 7:
        desireEC = Firebase.getFloat("SetState/EC");
        //Serial.print("Desire EC is: "); Serial.println(desireEC);
        stringToSend = stringEC + desireEC;
        Serial.println(stringToSend);
        while (1){
            if (cmdSerialDataAvailable() > 0){
                if (strstr(cmdReceivedBuffer, "DONE") != NULL){
                    break;
                }
            }
        }
        //Serial.println("DONE");
        break;
    case 8:
        desirePH = Firebase.getFloat("SetState/pH");
        //Serial.print("Desire pH is: "); Serial.println(desirePH);
        stringToSend = stringPH + desirePH;
        Serial.println(stringToSend);
        while (1){
            if (cmdSerialDataAvailable() > 0){
                if (strstr(cmdReceivedBuffer, "DONE") != NULL){
                    break;
                }
            }
        }
        break;
    }
}

byte cmdParse(){
    byte modeIndex = 0;
    if (strstr(cmdReceivedBuffer,"CLOSEPA") != NULL)
        modeIndex = 22;
    else if (strstr(cmdReceivedBuffer, "CLOSEPB") != NULL)
        modeIndex = 23;
    else if (strstr(cmdReceivedBuffer, "CLOSEPPH") != NULL)
        modeIndex = 24;
    else if (strstr(cmdReceivedBuffer, "CLOSEPW") != NULL)
        modeIndex = 25;
    else if (strstr(cmdReceivedBuffer, "CLOSES1") != NULL)
        modeIndex = 27;
    else if (strstr(cmdReceivedBuffer, "CLOSES2") != NULL)
        modeIndex = 28;

    // This one below use for autoMode only
    else if (strstr(cmdReceivedBuffer, "SENEC") != NULL)
        modeIndex = 2;
    else if (strstr(cmdReceivedBuffer, "SENPH") != NULL)
        modeIndex = 3;
    else if (strstr(cmdReceivedBuffer, "GETECDESIRE") != NULL)
        modeIndex = 7;
    else if (strstr(cmdReceivedBuffer, "GETPHDESIRE") != NULL)
        modeIndex = 8;
    return modeIndex;
}

byte cmdSensor(){
    byte modeIndex = 0;
    if (strstr(cmdReceivedBuffer, "EC:") != NULL)
        modeIndex = 1;
    else if (strstr(cmdReceivedBuffer, "PH:") != NULL)
        modeIndex = 2;
    else if (strstr(cmdReceivedBuffer, "TEMP:") != NULL)
        modeIndex = 3;
    else if (strstr(cmdReceivedBuffer, "MOIS:") != NULL)
        modeIndex = 4;
    else if (strstr(cmdReceivedBuffer, "LUX:") != NULL)
        modeIndex = 5;
    else if (strstr(cmdReceivedBuffer, "DONESENTALL") != NULL)
        modeIndex = 6;
    return modeIndex;
}

void commandSensor(byte mode){
    char *cmdReceivedBufferPtr;
    switch(mode){
        case 1:
            cmdReceivedBufferPtr = strstr(cmdReceivedBuffer, "EC:");
            cmdReceivedBufferPtr += strlen("EC:");
            nowEC = strtod(cmdReceivedBufferPtr, NULL);
            Firebase.setFloat("CurrentState/EC/Value", nowEC);
            readRTC();
            Firebase.setInt("CurrentState/EC/LastUpdated/second", second);
            Firebase.setInt("CurrentState/EC/LastUpdated/minute", minute);
            Firebase.setInt("CurrentState/EC/LastUpdated/hour", hour);
            Firebase.setInt("CurrentState/EC/LastUpdated/date", date);
            Firebase.setInt("CurrentState/EC/LastUpdated/month", month);
            Firebase.setInt("CurrentState/EC/LastUpdated/year", year);

            if(autoState){
                ecRec += 1;
                String record = "ECHistory/" + ecRec ;
                Firebase.setFloat(record + "/Value", nowEC);

                Firebase.setInt(record + "/Timestamp/second", second);
                Firebase.setInt(record + "/Timestamp/minute", minute);
                Firebase.setInt(record + "/Timestamphour", hour);
                Firebase.setInt(record + "/Timestampdate", date);
                Firebase.setInt(record + "/Timestamp/month", month);
                Firebase.setInt(record + "/Timestamp/year", year);
            }
            
            break;
        case 2:
            cmdReceivedBufferPtr = strstr(cmdReceivedBuffer, "PH:");
            cmdReceivedBufferPtr += strlen("PH:");
            nowPH = strtod(cmdReceivedBufferPtr, NULL);
            readRTC();
            Firebase.setFloat("CurrentState/pH/Value", nowPH);
            Firebase.setInt("CurrentState/pH/LastUpdated/second", second);
            Firebase.setInt("CurrentState/pH/LastUpdated/minute", minute);
            Firebase.setInt("CurrentState/pH/LastUpdated/hour", hour);
            Firebase.setInt("CurrentState/pH/LastUpdated/date", date);
            Firebase.setInt("CurrentState/pH/LastUpdated/month", month);
            Firebase.setInt("CurrentState/pH/LastUpdated/year", year);
            if(autoState){
                phRec += 1;
                String record = "pHHistory/" + phRec;
                Firebase.setFloat(record + "/Value", nowPH);
                Firebase.setInt(record + "/Timestamp/second", second);
                Firebase.setInt(record + "/Timestamp/minute", minute);
                Firebase.setInt(record + "/Timestamphour", hour);
                Firebase.setInt(record + "/Timestampdate", date);
                Firebase.setInt(record + "/Timestamp/month", month);
                Firebase.setInt(record + "/Timestamp/year", year);
            }
            break;
        case 3:
            cmdReceivedBufferPtr = strstr(cmdReceivedBuffer, "TEMP:");
            cmdReceivedBufferPtr += strlen("TEMP:");
            nowTemp = strtod(cmdReceivedBufferPtr, NULL);
            readRTC();
            Firebase.setFloat("CurrentState/Temp/Value", nowTemp);
            Firebase.setInt("CurrentState/Temp/LastUpdated/second", second);
            Firebase.setInt("CurrentState/Temp/LastUpdated/minute", minute);
            Firebase.setInt("CurrentState/Temp/LastUpdated/hour", hour);
            Firebase.setInt("CurrentState/Temp/LastUpdated/date", date);
            Firebase.setInt("CurrentState/Temp/LastUpdated/month", month);
            Firebase.setInt("CurrentState/Temp/LastUpdated/year", year);
            if(autoState){
                tempRec += 1;
                String record = "TempHistory/" + tempRec ;
                Firebase.setFloat(record + "/Value", nowTemp);
                Firebase.setInt(record + "/Timestamp/second", second);
                Firebase.setInt(record + "/Timestamp/minute", minute);
                Firebase.setInt(record + "/Timestamphour", hour);
                Firebase.setInt(record + "/Timestampdate", date);
                Firebase.setInt(record + "/Timestamp/month", month);
                Firebase.setInt(record + "/Timestamp/year", year);
            }
            
            break;
        case 4:
            cmdReceivedBufferPtr = strstr(cmdReceivedBuffer, "MOIS:");
            cmdReceivedBufferPtr += strlen("MOIS:");
            nowMois = strtof(cmdReceivedBufferPtr, NULL);
            readRTC();
            Firebase.setFloat("CurrentState/Moisture/Value", nowMois);
            Firebase.setInt("CurrentState/Moisture/LastUpdated/second", second);
            Firebase.setInt("CurrentState/Moisture/LastUpdated/minute", minute);
            Firebase.setInt("CurrentState/Moisture/LastUpdated/hour", hour);
            Firebase.setInt("CurrentState/Moisture/LastUpdated/date", date);
            Firebase.setInt("CurrentState/Moisture/LastUpdated/month", month);
            Firebase.setInt("CurrentState/Moisture/LastUpdated/year", year);

            break;
        case 5:
            cmdReceivedBufferPtr = strstr(cmdReceivedBuffer, "LUX:");
            cmdReceivedBufferPtr += strlen("LUX:");
            nowLux = strtol(cmdReceivedBufferPtr, NULL, 10);
            readRTC();
            Firebase.setInt("CurrentState/Lux/Value", nowLux);
            Firebase.setInt("CurrentState/Lux/LastUpdated/second", second);
            Firebase.setInt("CurrentState/Lux/LastUpdated/minute", minute);
            Firebase.setInt("CurrentState/Lux/LastUpdated/hour", hour);
            Firebase.setInt("CurrentState/Lux/LastUpdated/date", date);
            Firebase.setInt("CurrentState/Lux/LastUpdated/month", month);
            Firebase.setInt("CurrentState/Lux/LastUpdated/year", year);
            break;
        case 6:
            sentFlag = true;
            break;
    }
}

void readRTC(){
    // **************************** Time *********************************
    second = rtc.getSecond();
    minute = rtc.getMinute();
    hour = rtc.getHour(h12, PM);
    date = rtc.getDate();
    month = rtc.getMonth(Century);
    year = rtc.getYear() + 2000;
    temperature = rtc.getTemperature();
    /* Serial.print("   Year = "); //year
    Serial.print(year);
    Serial.print("   Month = "); //month
    Serial.print(month);
    Serial.print("   Day = "); //day
    Serial.print(date);
    Serial.print("   Hour = "); //hour
    Serial.print(hour);
    Serial.print("   Minute = "); //minute
    Serial.print(minute);
    Serial.print("   Second = "); //second
    Serial.println(second); */
}

void openLedWhite()
{
    for (uint8_t i = 0; i < NUM_LEDS; i++)
    {
        ledStrip.setPixelColor(i, ledStrip.Color(255, 255, 255));
    }
    ledStrip.show();
}

void closeLedWhite()
{
    for (uint8_t i = 0; i < NUM_LEDS; i++)
    {
        //ledStrip.clear();
        ledStrip.setPixelColor(i, ledStrip.Color(0, 0, 0));
    }
    ledStrip.show();
}

void openDecoLed()
{
    if (motorSpeed == 0)
    {
        rainbowCycle(10);
    }
    else
    {
        meteorRain(10, 64, true, 60);
    }
}

void closeDecoLed()
{
    for (uint8_t i = 0; i < NUM_DECOLEDS; i++)
    {
        ledRing.clear();
        //ledRing.setPixelColor(i, ledRing.Color(0, 0, 0, 0));
    }
    ledRing.show();
}

//LED function
void rainbowCycle(uint8_t wait)
{
    uint16_t i, j;

    for (j = 0; j < 256; j++)
    { // 1 cycles of all colors on wheel
        for (i = 0; i < NUM_DECOLEDS; i++)
        {
            ledRing.setPixelColor(i, Wheel(((i * 256 / NUM_DECOLEDS) + j) & 255));
        }
        ledRing.show();
        delay(wait);
    }
}

uint32_t Wheel(byte WheelPos)
{
    WheelPos = 255 - WheelPos;
    if (WheelPos < 85)
    {
        return ledRing.Color(255 - WheelPos * 3, 0, WheelPos * 3);
    }
    if (WheelPos < 170)
    {
        WheelPos -= 85;
        return ledRing.Color(0, WheelPos * 3, 255 - WheelPos * 3);
    }
    WheelPos -= 170;
    return ledRing.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}

void setPixel(int Pixel, byte red, byte green, byte blue)
{
    ledRing.setPixelColor(Pixel, ledRing.Color(red, green, blue));
}

void setAll(byte red, byte green, byte blue)
{
    for (int i = 0; i < NUM_DECOLEDS; i++)
    {
        setPixel(i, red, green, blue);
    }
    ledRing.show();
}

void meteorRain(byte meteorSize, byte meteorTrailDecay, boolean meteorRandomDecay, int wait)
{
    //byte red, byte green, byte blue;
    setAll(0, 0, 0);

    for (int i = 0; i < NUM_DECOLEDS * 2; i++)
    {
        // fade brightness all LEDs one step
        for (int j = 0; j < NUM_DECOLEDS; j++)
        {
            if ((!meteorRandomDecay) || (random(10) > 5))
            {
                fadeToBlack(j, meteorTrailDecay);
            }
        }

        // draw meteor
        for (int j = 0; j < meteorSize; j++)
        {
            if ((i - j < NUM_DECOLEDS) && (i - j >= 0))
            {
                //setPixel(i - j, red, green, blue);
                ledRing.setPixelColor(i - j, Wheel(((i * 256 / NUM_DECOLEDS) + j) & 255));
            }
        }
        ledRing.show();
        delay(wait);
    }
}

void fadeToBlack(int ledNo, byte fadeValue)
{
    // NeoPixel
    uint32_t oldColor;
    uint8_t r, g, b;
    int value;

    oldColor = ledRing.getPixelColor(ledNo);
    r = (oldColor & 0x00ff0000UL) >> 16;
    g = (oldColor & 0x0000ff00UL) >> 8;
    b = (oldColor & 0x000000ffUL);

    r = (r <= 10) ? 0 : (int)r - (r * fadeValue / 256);
    g = (g <= 10) ? 0 : (int)g - (g * fadeValue / 256);
    b = (b <= 10) ? 0 : (int)b - (b * fadeValue / 256);

    ledRing.setPixelColor(ledNo, r, g, b);
}

void autoProcess()
{
    //TODO: AUTO mode
}