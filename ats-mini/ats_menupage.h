/*
  ats_menupage.h
*/

#include "Menu.h"	// from ats-mini Menu

//
// JUST A TEST
//
void drawText(const char* text, int32_t X,  int32_t Y, uint8_t font)
{
  spr.drawString(text, X, Y, font);
  spr.pushSprite(0, 0);
}


void ats_menu_display() {
  MENU.outln(F("ATS-MINI RADIO remote actions (TEST)\n"));

  MENU.outln("'B'=Band up\t'b'=Band down");
  MENU.outln("'M'=Mode up\t'm'=Mode down");
  MENU.outln("'O'=sleep on\t'o'=sleep off");
  MENU.outln("'S'=Step Up\t's'=Step down");
  MENU.outln("'W'=bandwith +\t'w'=bandwith -");
  MENU.outln("'A'=AGC +\t'a'=AGC -");
  MENU.outln("'V'=Volume +\t'v'=Volume -");
  MENU.outln("'L'=Light +\t'l'=Light -");
} // ats_menu_display()


bool ats_menu_reaction(char token) {
  switch(token)
    {
    case 'B': // Band up
      doBand(1);
      break;

    case 'b': // Band down
      doBand(-1);
      break;

    case 'M': // Mode up
      doMode(1);
      break;

    case 'm': // Mode down
      doMode(-1);
      break;

    case 'O': // sleep on
      sleepOn(true);
      break;

    case 'o': // sleep off
      sleepOn(false);
      break;

    case 'S': // Step Up
      doStep(1);
      break;

    case 's': // Step Down
      doStep(-1);
      break;

    case 'W': // Bandwidth Up
      doBandwidth(1);
       break;

    case 'w': // Bandwidth Down
      doBandwidth(-1);
      break;

    case 'A': // AGC/ATTN Up
      doAgc(1);
      break;

    case 'a': // AGC/ATTN Down
      doAgc(-1);
      break;

    case 'V': // Volume Up
      doVolume(1);
      break;

    case 'v': // Volume Down
      doVolume(-1);
      break;

    case 'L': // Backlight Up
      doBrt(1);
      break;

    case 'l': // Backlight Down
      doBrt(-1);
      break;

//    case 'T':
//      drawText("MAMMA", 120, 5, 1);
//      delay(1000);
//      drawText("MIA", 200, 5, 1);
//      delay(2000);
//      break;

    default:
      return 0;		// token not found in this menu
    }
  return 1;		// token found in this menu
} // ats_menu_reaction()


/*
switch(key)
  {
    case 'R': // Rotate Encoder Clockwise
      event |= 1 << REMOTE_DIRECTION;
      event |= REMOTE_EEPROM;
      break;
    case 'r': // Rotate Encoder Counterclockwise
      event |= -1 << REMOTE_DIRECTION;
      event |= REMOTE_EEPROM;
      break;
    case 'e': // Encoder Push Button
      event |= REMOTE_CLICK;
      break;
    case 'B': // Band Up
      doBand(1);
      event |= REMOTE_EEPROM;
      break;
    case 'b': // Band Down
      doBand(-1);
      event |= REMOTE_EEPROM;
      break;
    case 'M': // Mode Up
      doMode(1);
      event |= REMOTE_EEPROM;
      break;
    case 'm': // Mode Down
      doMode(-1);
      event |= REMOTE_EEPROM;
      break;
    case 'S': // Step Up
      doStep(1);
      event |= REMOTE_EEPROM;
      break;
    case 's': // Step Down
      doStep(-1);
      event |= REMOTE_EEPROM;
      break;
    case 'W': // Bandwidth Up
      doBandwidth(1);
      event |= REMOTE_EEPROM;
      break;
    case 'w': // Bandwidth Down
      doBandwidth(-1);
      event |= REMOTE_EEPROM;
      break;
    case 'A': // AGC/ATTN Up
      doAgc(1);
      event |= REMOTE_EEPROM;
      break;
    case 'a': // AGC/ATTN Down
      doAgc(-1);
      event |= REMOTE_EEPROM;
      break;
    case 'V': // Volume Up
      doVolume(1);
      event |= REMOTE_EEPROM;
      break;
    case 'v': // Volume Down
      doVolume(-1);
      event |= REMOTE_EEPROM;
      break;
    case 'L': // Backlight Up
      doBrt(1);
      event |= REMOTE_EEPROM;
      break;
    case 'l': // Backlight Down
      doBrt(-1);
      event |= REMOTE_EEPROM;
      break;
    case 'O':
      sleepOn(true);
      break;
    case 'o':
      sleepOn(false);
      break;
    case 'I':
      doCal(1);
      event |= REMOTE_EEPROM;
      break;
    case 'i':
      doCal(-1);
      event |= REMOTE_EEPROM;
      break;
    case 'C':
      remoteLogOn = false;
      remoteCaptureScreen();
      break;
    case 't':
      remoteLogOn = !remoteLogOn;
      break;

    case '$':
      remoteGetMemories();
      break;
    case '#':
      if (remoteSetMemory())
        event |= REMOTE_EEPROM;
      break;

    case 'T':
      Serial.println(switchThemeEditor(!switchThemeEditor()) ? "Theme editor enabled" : "Theme editor disabled");
      break;
    case '!':
      if(switchThemeEditor()) setColorTheme();
      break;
    case '@':
      if(switchThemeEditor()) getColorTheme();
      break;

    default:
      // Command not recognized
      return(event);
  }

  // Command recognized
  return(event | REMOTE_CHANGED);
}
*/
