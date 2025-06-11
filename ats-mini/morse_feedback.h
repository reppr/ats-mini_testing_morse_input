/*
  morse_feedback.h
*/

#ifndef MORSE_FEEDBACK_H

int32_t morse_feedback_X=140;
int32_t morse_feedback_Y=5;
int32_t morse_feedback_R=10;

uint32_t morse_feedback_COLOR_OFF=	TFT_BLACK;
uint32_t morse_feedback_COLOR_DOT=	TFT_YELLOW;
uint32_t morse_feedback_COLOR_DASH=	TFT_GREEN;
uint32_t morse_feedback_COLOR_LOONG=	TFT_ORANGE;
uint32_t morse_feedback_COLOR_OVERLONG=	TFT_RED;

void show_morse_duration_feedback(uint8_t duration) {
  switch(duration) {
  case '0':	// OFF
    //    spr.fillCircle(morse_feedback_X, morse_feedback_Y, morse_feedback_R, morse_feedback_COLOR_OFF);
    drawText("     ", morse_feedback_X, morse_feedback_Y, 1);
    break;
  case '.':	// ON
    drawText("ON", morse_feedback_X, morse_feedback_Y, 1);
    //    spr.fillCircle(morse_feedback_X, morse_feedback_Y, morse_feedback_R, morse_feedback_COLOR_DOT);
    break;
  case '-':	// start dash
    drawText("DASH", morse_feedback_X, morse_feedback_Y, 1);
    //    spr.fillCircle(morse_feedback_X, morse_feedback_Y, morse_feedback_R, morse_feedback_COLOR_DASH);
    break;
  case '!':
    drawText("LOONG", morse_feedback_X, morse_feedback_Y, 1);
    //    spr.fillCircle(morse_feedback_X, morse_feedback_Y, morse_feedback_R, morse_feedback_COLOR_LOONG);
    break;
  case 'V':
    drawText("_____", morse_feedback_X, morse_feedback_Y, 1);
    //    spr.fillCircle(morse_feedback_X, morse_feedback_Y, morse_feedback_R, morse_feedback_COLOR_OVERLONG);
    break;

  default:
    DADA(F("invalid duration"));
    //  MENU.error_ln(F("invalid duration"));
  }
} // show_morse_duration_feedback()


#define MORSE_FEEDBACK_H
#endif
