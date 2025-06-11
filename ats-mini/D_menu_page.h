/*
  D_menu_page.h

  temporary Debugging and Developing menu
*/

#include "morse_feedback.h"


void D_menu_display() {
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
    show_morse_duration_feedback('0');
    break;
    
  case '.':
    show_morse_duration_feedback('.');
    break;
    
  case '-':
    show_morse_duration_feedback('-');
    break;
    
  case '!':
    show_morse_duration_feedback('!');
    break;
    
  case 'V':
    show_morse_duration_feedback('V');
    break;
    
  default:
    return 0;		// token not found in this menu
  }

  //  delay(1000);	// TODO: REMOVE ################################################################
  return 1;		// token found in this menu 
}
