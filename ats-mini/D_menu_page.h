/*
  D_menu_page.h

  temporary Debugging and Developing menu
*/


extern int32_t morse_feedback_X;
extern int32_t morse_feedback_Y;
extern int32_t morse_feedback_H;
extern int32_t morse_feedback_R;

extern uint32_t morse_feedback_COLOR_OFF;
extern uint32_t morse_feedback_COLOR_DOT;
extern uint32_t morse_feedback_COLOR_DASH;
extern uint32_t morse_feedback_COLOR_LOONG;
extern uint32_t morse_feedback_COLOR_OVERLONG;

extern void signal_morse_in(uint8_t token);

void D_menu_display() {
  #if defined RUNNING_ON_ATS_MINI
    MENU.outln(F("\tESP32-SI4732 Receiver"));
  #else
    DADA(F("unknown gadget"));
  #endif

  MENU.outln(F("\tmorse feedback: X<nnn>\tY<nnn>\tR<nnn>"));
  MENU.outln(F("\tmorse feedback: 0\t.\t-\t!\tV"));
}

bool D_menu_reaction(char token) {
  long newValue;

  switch (token) {
  case 'X':	// morse feedback X
    morse_feedback_X = (int32_t) MENU.calculate_input(morse_feedback_X);
    //    spr.fillCircle(morse_feedback_X, morse_feedback_Y, morse_feedback_R, morse_feedback_COLOR_DOT);
    drawText("ON", morse_feedback_X, morse_feedback_Y, 1);
    MENU.outln(morse_feedback_X);
    break;

  case 'Y':	// morse feedback Y
    morse_feedback_Y = (int32_t) MENU.calculate_input(morse_feedback_Y);
    //    spr.fillCircle(morse_feedback_X, morse_feedback_Y, morse_feedback_R, morse_feedback_COLOR_DOT);
    drawText("DASH", morse_feedback_X, morse_feedback_Y, 1);
    MENU.outln(morse_feedback_Y);
    break;

  case 'R':	// morse feedback R
    morse_feedback_R = (int32_t) MENU.calculate_input(morse_feedback_R);
    spr.fillCircle(morse_feedback_X, morse_feedback_Y, morse_feedback_R, morse_feedback_COLOR_DOT);
    MENU.outln(morse_feedback_R);
    break;

  case '0':
    signal_morse_in('0');
    break;
    
  case '.':
    signal_morse_in('.');
    break;
    
  case '-':
    signal_morse_in('-');
    break;
    
  case '!':
    signal_morse_in('!');
    break;
    
  case 'V':
    signal_morse_in('V');
    break;
    
  default:
    return 0;		// token not found in this menu
  }

  //  delay(1000);	// TODO: REMOVE ################################################################
  return 1;		// token found in this menu 
}
