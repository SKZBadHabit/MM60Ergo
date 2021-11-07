//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//                                                                              TODO

// TODO Code verkleinern
// TODO Code mit Klassen verschönern

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//                                                                              LIBRARY
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------

//#include <Keypad.h>
#include <Keypad_MC17.h> // I2C i/o library for Keypad
#include <Wire.h>        // I2C library for Keypad_MC17
#include <Keyboard.h>
#include <Mouse.h>
#include <EEPROMex.h> //INFO nur 100.000 Schreibvorgaenge pro Adresse!
//#include <Adafruit_MCP23017.h>
#include <avr/sleep.h>
#include "Trackpoint.h"

// Screen----------------------------------------------------------------------------------------------------

#include "SSD1306Ascii.h"
#include "SSD1306AsciiWire.h"

// 0X3C+SA0 - 0x3C or 0x3D
#define I2C_ADDRESS 0x3C
SSD1306AsciiWire oled;
// Screen----------------------------------------------------------------------------------------------------

#define I2CADDR 0x20 // address of MCP23017 chip on I2C bus

// Trackpoint:

Trackpoint trackpoint(9,   // CLK
                      8,   // DATA
                      12); // RESET

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//                                                                             VARIABLES
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------

// MCP Module:
// Adafruit_MCP23017 mcp;

// Allgemeines:
unsigned long previousMillis = 0;
byte setLayer = 1;
byte ModifierPressed = 0;

boolean PressActiveStatusL2 = false;
char AutoTypeKey = 3;
boolean AutoTypeStatus = false;
byte AutoTypeSpeedCounter = 0;
int AutoTypeSpeed = 400;

// Menu:
int RunOnlyOnce = 0;
unsigned long previousMillisMenuOled1 = 0;

String StringRead = "0";
byte RefreshMenu = 0;
byte StatusOled = 0;
byte RunOnlyOnceOLED = 0;
byte MenuOled = 1;
String MenuMaskOled = "";
byte beforeClear = 0;
byte removeToMuch = 0;
byte AutorefreshOledOnOff = 1;

// KeyCounter:
long KeyboardCounterAktuell = 0;
long KeyboardCounterTurnON = 0;
long KeyboardCounterGesamt = 0;

// Keyboard Modifiers:
byte Apostroph = 96;
byte MiniPipe = 39;
byte Strichpunkt = 59;
byte Space = 32;
byte Backslash = 92;
byte Wave = 96;
byte charFN = 30;
byte charFNr = 29;
byte LMT = 15;

byte OledMenuButton = 28;

// Keyboard Definitions:
unsigned long AutoTypeTimer = 0;
unsigned long previousMillisIdleTimer = 0;
byte IdleTimer = 0;
byte IdleTimer2 = 0;

// KEYS:=====================================================

/* Einrichtungsmatrix:
        {'1', '2', '3', '4'},
        {'5', '6', '7', '8'},
        {'9', '0', 'a', 'b'},
        {'c', 'd', 'e', 'f'},
        {'g', 'h', 'i', 'j'},
        {'k', 'l', 'm', 'n'},
        {'o', 'p', 'q', 'r'},
        {'s', 't', 'u', 'v'}};



*/

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------

// Layer1 mit Grundeinstellungen:

const byte ROWS = 8; // four rows
const byte COLS = 4; // three columns
char keysL1[ROWS][COLS] = {
    {KEY_DELETE, '7', '9', KEY_BACKSPACE},
    {']', 'u', 'o', Backslash},
    {'[', 'j', 'l', MiniPipe},
    {'=', 'm', '.', KEY_RIGHT_SHIFT},
    {'-', '6', '8', '0'},
    {Space, 'y', 'i', 'p'},
    {KEY_RETURN, 'h', 'k', ';'},
    {charFN, 'n', ',', '/'}};

byte rowPins[ROWS] = {0, 1, 2, 3, 4, 5, 6, 7}; // connect to the row pinouts of the kpd
byte colPins[COLS] = {8, 9, 10, 11};           // connect to the column pinouts of the kpd

// modify constructor for I2C i/o
Keypad_MC17 kpdL1(makeKeymap(keysL1), rowPins, colPins, ROWS, COLS, I2CADDR);

const byte ROWS1 = 8;
const byte COLS1 = 8;

char keysLayer1[ROWS1][COLS1] = {
    {KEY_ESC, KEY_LEFT_CTRL, '4', '2'},
    {KEY_TAB, KEY_RIGHT_GUI, 'r', 'w'},
    {OledMenuButton, Apostroph, 'f', 's'},
    {KEY_LEFT_SHIFT, KEY_RIGHT_ALT, 'v', 'x'},
    {'z', KEY_LEFT_ALT, 'b', 'c'},
    {'a', Space, 'g', 'd'},
    {'q', charFNr, 't', 'e'},
    {'1', KEY_RETURN, '5', '3'}};

byte rowPins1[ROWS1] = {10, 16, 14, 15, 18, 19, 20, 21}; // connect to the row pinouts of the keypadL1
byte colPins1[COLS1] = {0, 1, 4, 5};                     // connect to the column pinouts of the keypadL1

Keypad keypadL1 = Keypad(makeKeymap(keysLayer1), rowPins1, colPins1, ROWS1, COLS1);

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//                                                                              Classes
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------

class LED
{

public:
  void setPinsToOutput()
  {
    kpdL1.pin_mode(McpLED, OUTPUT);
    pinMode(UnderglowRight, OUTPUT);
    pinMode(UnterglowLeft, OUTPUT);

    pinMode(RXLED, OUTPUT); // Set RX LED as an output
  };

  void turnOffInternalLEDs()
  {
    digitalWrite(RXLED, HIGH); // set the RX LED OFF
    TXLED1;                    // TX LED is not tied to a normally controlled pin so a macro is needed, turn LED OFF
  }

  void LayerON()
  {
    if (McpLEDStatus == 0)
    {
      kpdL1.pin_write(McpLED, HIGH);
      McpLEDStatus = 1;
    }
  };

  void LayerOFF()
  {
    if (McpLEDStatus == 1)
    {
      kpdL1.pin_write(McpLED, LOW);
      McpLEDStatus = 0;
    }
  };

  void UnderglowON()
  {

    digitalWrite(UnderglowRight, HIGH);
    digitalWrite(UnterglowLeft, HIGH);
  };

  void UnderglowOFF()
  {

    digitalWrite(UnderglowRight, LOW);
    digitalWrite(UnterglowLeft, LOW);
  };

private:
  byte McpLED = 97;
  byte McpLEDStatus = 0;
  byte UnderglowRight = 6;
  byte UnterglowLeft = 98;

  byte RXLED = 17;
};

LED LEDClass;

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//                                                                              SETUP
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void setup()
{

  Wire.begin();
  Wire.setClock(400000);

  oled.begin(&Adafruit128x64, I2C_ADDRESS);

  Serial.begin(38400);
  kpdL1.begin(); // now does not starts wire library
  kpdL1.setDebounceTime(1);

  Mouse.begin();

  // Trackpoint
  Mouse.begin();
  trackpoint.reset();
  trackpoint.setRemoteMode();
  trackpoint.setSensitivityFactor(254); // more sensitive than by default

  LEDClass.setPinsToOutput();

  // EEProm Saved Values auslesen:
  KeyboardCounterGesamt = EEPROM.readLong(0);
}

//------------------------------------------------------------------------------

void loop()
{

  // Keypresscounter Start:
  kepressCounterFunktion();

  // Allgemein Keyboard:
  IdleTimerApp();
  LEDClass.turnOffInternalLEDs();
  RefreshOledAutomatical();
  AutoType();
  Trackpoint();

  if (setLayer == 1)
  {
    DoubleKeyboard();
    // LedSwitchLayerOFF();
    LEDClass.LayerOFF();
    RunOnlyOnceStartup(); // OLED:
    menuOled();
  }

  if (setLayer == 2)
  {
    DoubleKeyboard();
    // LedSwitchLayerON();
    LEDClass.LayerON();
    menuOled();
  }

  if (setLayer == 3)
  {
    DoubleKeyboard();
    // LedSwitchLayerON;
    LEDClass.LayerON();
    LEDClass.UnderglowOFF();
    menuOled();
  }
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//                                                                            Funktionen
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------

// LAYER--------------------------------------------------------------------------------

void DoubleKeyboard()
{

  if (keypadL1.getKeys() || kpdL1.getKeys())
  {

    for (int i = 0; i < LIST_MAX; i++) // Scan the whole key list.
    {
      if (keypadL1.key[i].stateChanged) // Only find keys that have changed state.
      {

        // Reset Idle Timer:
        IdleTimerReset();

        switch (keypadL1.key[i].kstate)
        { // Report active key state : IDLE, PRESSED, HOLD, or RELEASED
        case PRESSED:

          if (keypadL1.key[i].kchar == charFNr)
          {
            setLayer = 2;
            // Serial.print("Layer: ");Serial.println(setLayer);
          }

          if (keypadL1.key[i].kchar == OledMenuButton)
          {
            if (MenuOled >= 4)
            {
              MenuOled = 1;
            }
            else
            {
              MenuOled += 1;
            }

            ModifierPressed = 1;
          }

          if (setLayer == 2)
          {

            switch (keypadL1.key[i].kchar)
            {

            case 'c':
              macroCalc();
              ModifierPressed = 1;
              break;

            case 'r':
              ModifierPressed = 1;
              AutoTypeKey = 'r';
              AutoTypeStatus = true;
              break;

            case 'f':
              ModifierPressed = 1;
              AutoTypeKey = 'f';
              AutoTypeStatus = true;
              break;

            case 't':
              Mouse.press(MOUSE_MIDDLE);
              Mouse.release(MOUSE_MIDDLE);
              ModifierPressed = 1;
              break;

            case 'g':
              Mouse.press(MOUSE_RIGHT);
              Mouse.release(MOUSE_RIGHT);
              ModifierPressed = 1;
              break;

            case 'b':
              Mouse.press(MOUSE_LEFT);

              ModifierPressed = 1;
              break;

            case '1':
              keypadL1.key[i].kchar = KEY_F1;
              break;

            case '2':
              keypadL1.key[i].kchar = KEY_F2;
              break;

            case '3':
              keypadL1.key[i].kchar = KEY_F3;
              break;

            case '4':
              keypadL1.key[i].kchar = KEY_F4;
              break;

            case '5':
              keypadL1.key[i].kchar = KEY_F5;
              break;

            case 'a':
              keypadL1.key[i].kchar = KEY_F11;
              break;

            case 's':
              keypadL1.key[i].kchar = KEY_F12;
              break;

            default:
              // without case
              break;
            }
          }

          if (setLayer == 3)
          {

            switch (keypadL1.key[i].kchar)
            {

            case 'w':
              ModifierPressed = 1;
              Mouse.move(0, -5, 0);
              AutoTypeKey = 'w';
              AutoTypeStatus = true;
              break;

            case 'a':
              ModifierPressed = 1;
              Mouse.move(-5, 0, 0);
              AutoTypeKey = 'a';
              AutoTypeStatus = true;
              break;

            case 's':
              ModifierPressed = 1;
              Mouse.move(0, 5, 0);
              AutoTypeKey = 's';
              AutoTypeStatus = true;
              break;

            case 'd':
              ModifierPressed = 1;
              Mouse.move(5, 0, 0);
              AutoTypeKey = 'd';
              AutoTypeStatus = true;
              break;

            case 't':
              Mouse.press(MOUSE_MIDDLE);

              ModifierPressed = 1;
              break;

            case 'g':
              Mouse.press(MOUSE_RIGHT);
              Mouse.release(MOUSE_RIGHT);
              ModifierPressed = 1;
              break;

            case 'b':
              Mouse.press(MOUSE_LEFT);

              ModifierPressed = 1;
              break;

            default:
              // without case
              break;
            }
          }

          if (ModifierPressed == 0)
          {
            keypressHOLD(keypadL1.key[i].kchar);
          }
          if (ModifierPressed == 1)
          {
          }

          // Serial.println(keypadL1.key[i].kchar);

          break;
        case HOLD:

          if (ModifierPressed == 0)
          {
            keypressHOLD(keypadL1.key[i].kchar);
          }
          if (ModifierPressed == 1)
          {
          }
          break;
          // Serial.println("Hold layer1");

          break;
        case RELEASED:

          if (keypadL1.key[i].kchar == charFNr)
          {
            setLayer = 1;
          }

          if (ModifierPressed == 1)
          {
            if (keypadL1.key['b'].kchar)
            {
              Mouse.release(MOUSE_LEFT);
            }
            if (keypadL1.key['t'].kchar)
            {
              Mouse.release(MOUSE_MIDDLE);
            }
            ModifierPressed = 0;
          }

          keypressRELEASE(keypadL1.key[i].kchar);

          AutoTypeStatus = false;
          AutoTypeSpeed = 400;
          AutoTypeSpeedCounter = 0;

          break;
        case IDLE:
          break;
          // Serial.println("Idle layer1");
        }
      }

      if (kpdL1.key[i].stateChanged) // Only find keys that have changed state.
      {

        // Reset Idle Timer:
        IdleTimerReset();

        switch (kpdL1.key[i].kstate)
        { // Report active key state : IDLE, PRESSED, HOLD, or RELEASED
        case PRESSED:

          if (kpdL1.key[i].kchar == charFN)
          {
            switch (setLayer)
            {
            case 1:
              setLayer = 2;
              break;

            case 2:
              setLayer = 3;
              break;

            case 3:

              setLayer = 1;
              break;

            default:
              // without case
              break;
            }

            // Serial.print("Layer: ");Serial.println(setLayer);
          }

          if (setLayer == 2)
          {

            switch (kpdL1.key[i].kchar)
            {

            case 'i':
              kpdL1.key[i].kchar = KEY_UP_ARROW;
              break;

            case 'k':
              kpdL1.key[i].kchar = KEY_DOWN_ARROW;
              break;

            case 'j':
              kpdL1.key[i].kchar = KEY_LEFT_ARROW;
              break;

            case 'l':
              kpdL1.key[i].kchar = KEY_RIGHT_ARROW;
              break;

            case 'h':
              kpdL1.key[i].kchar = KEY_HOME;
              break;

            case 'n':
              kpdL1.key[i].kchar = KEY_END;
              break;

            case 'y':
              kpdL1.key[i].kchar = KEY_INSERT;
              break;

            case 'u':
              kpdL1.key[i].kchar = KEY_PAGE_UP;
              break;

            case 'o':
              kpdL1.key[i].kchar = KEY_PAGE_DOWN;
              break;

            case 'p':
              macroPrintScreen();
              ModifierPressed = 1;
              break;

            case '6':
              kpdL1.key[i].kchar = KEY_F6;
              break;
            case '7':
              kpdL1.key[i].kchar = KEY_F7;
              break;
            case '8':
              kpdL1.key[i].kchar = KEY_F8;
              break;
            case '9':
              kpdL1.key[i].kchar = KEY_F9;
              break;
            case '0':
              kpdL1.key[i].kchar = KEY_F10;
              break;

            case '[':
              Keyboard.press(KEY_LEFT_ALT);
              delay(50);
              Keyboard.press('\352');
              Keyboard.release('\352');
              delay(50);
              Keyboard.press('\352');
              Keyboard.release('\352');
              delay(50);
              Keyboard.press('\346');
              Keyboard.release('\346');
              delay(50);
              Keyboard.press('\352');
              Keyboard.release('\352');
              delay(50);
              Keyboard.release(KEY_LEFT_ALT);

              ModifierPressed = 1;

              break;

            case ']':

              Keyboard.press(KEY_LEFT_ALT);
              delay(50);
              Keyboard.press('\352');
              Keyboard.release('\352');
              delay(50);
              Keyboard.press('\352');
              Keyboard.release('\352');
              delay(50);
              Keyboard.press('\346');
              Keyboard.release('\346');
              delay(50);
              Keyboard.press('\342');
              Keyboard.release('\342');
              delay(50);
              Keyboard.release(KEY_LEFT_ALT);

              ModifierPressed = 1;

              break;

            case '=':

              Keyboard.press(KEY_LEFT_ALT);
              delay(50);
              Keyboard.press('\352');
              Keyboard.release('\352');
              delay(50);
              Keyboard.press('\341');
              Keyboard.release('\341');
              delay(50);
              Keyboard.press('\342');
              Keyboard.release('\342');
              delay(50);
              Keyboard.press('\344');
              Keyboard.release('\344');
              delay(50);
              Keyboard.release(KEY_LEFT_ALT);

              ModifierPressed = 1;

              break;

            default:
              // do something
              break;
            }

            if (ModifierPressed == 0)
            {

              keypressHOLD(keypadL1.key[i].kchar);
            }
            if (ModifierPressed == 1)
            {
            }
          }

          if (setLayer == 3)
          {

            switch (kpdL1.key[i].kchar)
            {
            case '6':
              kpdL1.key[i].kchar = '\334';
              break;

            case '7':
              kpdL1.key[i].kchar = '\335';
              break;

            case '8':
              kpdL1.key[i].kchar = '\337';
              break;

            case 'y':
              kpdL1.key[i].kchar = '\347';
              break;

            case 'u':
              kpdL1.key[i].kchar = '\350';
              break;

            case 'i':
              kpdL1.key[i].kchar = '\351';
              break;

            case 'h':
              kpdL1.key[i].kchar = '\344';
              break;

            case 'j':
              kpdL1.key[i].kchar = '\345';
              break;

            case 'k':
              kpdL1.key[i].kchar = '\346';
              break;

            case 'n':
              kpdL1.key[i].kchar = '\341';
              break;

            case 'm':
              kpdL1.key[i].kchar = '\342';
              break;

            case ',':
              kpdL1.key[i].kchar = '\343';
              break;

            case '=':
              kpdL1.key[i].kchar = '\352';
              break;

            default:
              // do something
              break;
            }
          }

          if (kpdL1.key[i].kchar == LMT)
          {
            Mouse.press(MOUSE_LEFT);
          }

          if (ModifierPressed == 0)
          {
            keypressHOLD(kpdL1.key[i].kchar);
          }
          if (ModifierPressed == 1)
          {
          }

          break;
        case HOLD:

          if (ModifierPressed == 0)
          {

            keypressHOLD(keypadL1.key[i].kchar);
          }
          if (ModifierPressed == 1)
          {
          }

          break;
        case RELEASED:

          if (kpdL1.key[i].kchar == LMT)
          {
            Mouse.release(MOUSE_LEFT);
          }

          keypressRELEASE(kpdL1.key[i].kchar);

          break;
        case IDLE:
          break;
        }
      }
    }
  }
}

//-------------------------------------------------------------------------------------

void keypressSingle(char inputKey)
{
  Keyboard.press(inputKey);
  Keyboard.release(inputKey);
}

void keypressHOLD(char inputKey)
{
  Keyboard.press(inputKey);
}

void keypressRELEASE(char inputKey)
{
  Keyboard.release(inputKey);
  KeyboardCounterAktuell++;
  KeyboardCounterTurnON++;
}

//-------------------------------------------------------------------------------------

void macroCalc()
{
  Keyboard.press(KEY_LEFT_GUI);
  delay(50);
  Keyboard.press('r');
  delay(50);

  Keyboard.release(KEY_LEFT_GUI);
  Keyboard.release('r');
  delay(100);
  Keyboard.print("calc.exe");
  delay(100);
  Keyboard.press(KEY_RETURN);
  Keyboard.release(KEY_RETURN);
}

//-------------------------------------------------------------------------------------

void macroPrintScreen()
{
  Keyboard.press(KEY_LEFT_GUI);
  delay(50);
  Keyboard.press(KEY_LEFT_SHIFT);
  delay(50);
  Keyboard.press('s');
  delay(50);

  Keyboard.release(KEY_LEFT_GUI);
  Keyboard.release(KEY_LEFT_SHIFT);
  Keyboard.release('s');
}

//-------------------------------------------------------------------------------------

void RunOnlyOnceStartup()
{
  if (RunOnlyOnce <= 255)
  {

    if (millis() - previousMillis >= 30)
    {
      previousMillis = millis();

      if (RunOnlyOnce >= 30 && RunOnlyOnceOLED == 0)
      {

        oled.clear();

        oled.setFont(CalLite24);
        oled.println(F("   Wait for..."));
        oled.println(F("      IT"));
        RunOnlyOnceOLED += 1;
      }

      if (RunOnlyOnce >= 100 && RunOnlyOnceOLED <= 1)
      {

        oled.clear();

        oled.setFont(CalLite24);
        oled.println(F(" MM64-Ergo"));
        oled.println(F("Fully Loaded"));
        RunOnlyOnceOLED += 1;
      }

      RunOnlyOnce += 1;
    }
  }

  if (RunOnlyOnce == 255)
  {

    LEDClass.UnderglowON();
    LEDClass.LayerON();

    RunOnlyOnce += 1;
  }
}

//-------------------------------------------------------------------------------------

void kepressCounterFunktion()
{
  if (KeyboardCounterAktuell == 1000)
  {

    KeyboardCounterGesamt += KeyboardCounterAktuell;

    EEPROM.updateLong(0, KeyboardCounterGesamt);

    KeyboardCounterAktuell = 0;
  }
}

//-------------------------------------------------------------------------------------

void menuOled()
{

  if (RunOnlyOnce == 256)
  {

    if (MenuOled == 1 && setLayer == 1 && StatusOled != 1)
    {

      StatusOled = 1;
      AutorefreshOledOnOff = 1;

      // Screen clear
      // oled.clear();

      oled.setFont(Callibri11);
      oled.setCursor(0, 0);

      oled.println(F("             MM-Ergo-64                     "));

      // Dynamic Screen String Build:
      MenuMaskOled = "  Keypress Now:";
      MenuMaskOled += "                                                 ";
      MenuMaskOled += KeyboardCounterAktuell;

      if (MenuMaskOled.length() > 25)
      {

        removeToMuch = MenuMaskOled.length() - 26;

        MenuMaskOled.remove(15, removeToMuch);
      }

      oled.print(MenuMaskOled);
      oled.println("   ");

      oled.println(F("  Layer                    Layer1 "));

      // Dynamic Screen String Build:
      MenuMaskOled = "  Runtime now:";
      MenuMaskOled += "                                                 ";
      MenuMaskOled += millis() / 60000;
      MenuMaskOled += " min";

      if (MenuMaskOled.length() > 26)
      {

        removeToMuch = MenuMaskOled.length() - 27;

        MenuMaskOled.remove(15, removeToMuch);
      }

      oled.print(MenuMaskOled);
      oled.println("   ");

      LEDClass.UnderglowON();
    }

    if (MenuOled == 1 && setLayer == 2 && StatusOled != 2)
    {

      StatusOled = 2;
      oled.setCursor(85, 4);
      oled.println(F("  Layer2 "));
    }

    if (MenuOled == 1 && setLayer == 3 && StatusOled != 3)
    {

      StatusOled = 3;
      oled.setCursor(85, 4);
      oled.println(F("  Layer3 "));
    }

    //---------------------------------------------------------------------------------------------

    if (MenuOled == 2 && StatusOled != 2)
    {

      StatusOled = 2;

      // Dynamic Screen String Build:
      MenuMaskOled = "";
      byte beforeClear = MenuMaskOled.length();
      MenuMaskOled += "                     ";
      MenuMaskOled += KeyboardCounterGesamt / 1000;
      MenuMaskOled += " k";

      if (MenuMaskOled.length() > 10)
      {

        removeToMuch = MenuMaskOled.length() - 10;

        MenuMaskOled.remove(beforeClear, removeToMuch);
      }

      // Screen clear
      oled.clear();
      oled.setFont(CalLite24);
      oled.print(F("Now: "));
      oled.println(KeyboardCounterTurnON);
      oled.print(F("All:"));
      oled.println(MenuMaskOled);
    }

    //---------------------------------------------------------------------------------------------

    if (MenuOled == 3 && StatusOled != 3)
    {

      StatusOled = 3;

      // Screen clear
      oled.clear();
      oled.setFont(CalLite24);
      oled.print(F(" NightMode  "));
      oled.println(KeyboardCounterTurnON);
      oled.print(F("  in 3 sec."));
      oled.println(MenuMaskOled);

      delay(3000);
      oled.clear();
      LEDClass.UnderglowOFF();
    }

    //---------------------------------------------------------------------------------------------

    if (MenuOled == 4 && StatusOled != 4)
    {

      StatusOled = 4;

      StatusOled += 1;
      MenuOled += 1;
    }

    //---------------------------------------------------------------------------------------------

    if (MenuOled == 99 && StatusOled != 99)
    {

      StatusOled = 99;
    }
  }
}

//-------------------------------------------------------------------------------------

void RefreshOledAutomatical()
{

  if (millis() - previousMillisMenuOled1 > 60000 && AutorefreshOledOnOff == 1 && MenuOled == 1)
  {
    previousMillisMenuOled1 = millis();

    MenuMaskOled = "              ";
    MenuMaskOled += millis() / 60000;
    MenuMaskOled += " min";

    if (MenuMaskOled.length() > 10)
    {

      removeToMuch = MenuMaskOled.length() - 11;

      MenuMaskOled.remove(0, removeToMuch);
    }

    oled.setCursor(78, 6);
    oled.print(MenuMaskOled);

    MenuMaskOled = "              ";
    MenuMaskOled += KeyboardCounterAktuell;

    if (MenuMaskOled.length() > 10)
    {

      removeToMuch = MenuMaskOled.length() - 11;

      MenuMaskOled.remove(0, removeToMuch);
    }

    oled.setCursor(78, 2);
    oled.print(MenuMaskOled);
  }
}

//-------------------------------------------------------------------------------------

void IdleTimerReset()
{

  IdleTimer = 0;
  previousMillisIdleTimer = millis();
}

//-------------------------------------------------------------------------------------

void UploadMode()
{

  oled.clear();
  LEDClass.UnderglowOFF();
}

//-------------------------------------------------------------------------------------

void IdleTimerApp()
{

  if (millis() - previousMillisIdleTimer >= 920000 && IdleTimer == 0)
  {

    IdleTimer = 1;
    IdleTimer2 = 1;
    AutorefreshOledOnOff = 0;

    oled.clear();
    LEDClass.UnderglowOFF();

    set_sleep_mode(SLEEP_MODE_IDLE);
    sleep_enable();
    sleep_mode();
  }

  if (IdleTimer == 0 && IdleTimer2 == 1)
  {

    sleep_disable();

    IdleTimer = 0;
    IdleTimer2 = 0;

    setLayer = 1;

    LEDClass.UnderglowON();

    oled.clear();
    MenuOled = 1;
    StatusOled = 0;

    previousMillisIdleTimer = millis();
  }
}

//-------------------------------------------------------------------------------------

void AutoType()
{

  if (AutoTypeStatus == true)
  {

    if (millis() - AutoTypeTimer >= AutoTypeSpeed)
    {

      AutoTypeTimer = millis();

      AutoTypeSpeedCounter += 1;

      switch (AutoTypeSpeedCounter)
      {
      case 3:
        AutoTypeSpeed = 100;
        break;
      case 10:
        AutoTypeSpeed = 50;
        break;
      case 20:
        AutoTypeSpeed = 20;
        break;
      case 60:
        AutoTypeSpeed = 1;
        break;
      default:
        break;
      }

      if (AutoTypeKey == 'r')
      {
        Mouse.move(0, 0, 1);
      }
      else if (AutoTypeKey == 'f')
      {
        Mouse.move(0, 0, -1);
      }

      if (AutoTypeKey == 'w')
      {
        Mouse.move(0, -5, 0);
      }
      else if (AutoTypeKey == 's')
      {
        Mouse.move(0, 5, 0);
      }
      if (AutoTypeKey == 'a')
      {
        Mouse.move(-5, 0, 0);
      }
      else if (AutoTypeKey == 'd')
      {
        Mouse.move(5, 0, 0);
      }
    }
  }
}

//-------------------------------------------------------------------------------------

void Trackpoint()
{

  Trackpoint::DataReport d = trackpoint.readData();

  Mouse.move(-d.x, d.y, 0);
}
