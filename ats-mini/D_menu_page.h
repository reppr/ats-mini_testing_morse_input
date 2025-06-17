/*
  D_menu_page.h

  temporary Debugging and Developing menu
*/


extern int32_t morse_feedback_X;
extern int32_t morse_feedback_Y;
extern int32_t morse_feedback_R;

extern uint32_t morse_feedback_COLOR_OFF;
extern uint32_t morse_feedback_COLOR_DOT;
extern uint32_t morse_feedback_COLOR_DASH;
extern uint32_t morse_feedback_COLOR_LOONG;
extern uint32_t morse_feedback_COLOR_OVERLONG;

extern void signal_morse_in(uint8_t token);

extern unsigned long morse_TimeUnit;

extern uint8_t morse_sep_algo;

void D_menu_display() {
  #if defined RUNNING_ON_ATS_MINI
    MENU.outln(F("\tESP32-SI4732 Receiver"));
  #else
    DADA(F("unknown gadget"));
  #endif

  MENU.outln(F("\tmorse feedback:\tX<nnn>\tY<nnn>\tR<nnn>"));
  MENU.out(F("\t\t\t"));
  MENU.out(morse_feedback_X);
  MENU.tab();
  MENU.out(morse_feedback_Y);
  MENU.tab();
  MENU.outln(morse_feedback_R);
  MENU.outln(F("\tmorse feedback: 0\t.\t-\t!\tV"));

  MENU.outln(F("\n\tfilled Circle:\t'C'"));
  MENU.outln(F("\t'x'\t'y'\t'r'"));
  MENU.tab();
  MENU.out(morse_feedback_X);
  MENU.tab();
  MENU.out(morse_feedback_Y);
  MENU.tab();
  MENU.outln(morse_feedback_R);
  MENU.ln();

  MENU.out(F("\tmorse Time unit: 'T'\t: "));
  MENU.outln(morse_TimeUnit);
  MENU.ln();

  MENU.out(F("\talgorithm '1'=tims '2'=limits '3'=diff '4'=limit diffs\t: "));
  MENU.outln(morse_sep_algo);
} // D_menu_display()


bool D_menu_reaction(char token) {
  long newValue;

  switch (token) {
  case '1':
    morse_sep_algo=1;
    break;
  case '2':
    morse_sep_algo=2;
    break;
  case '3':
    morse_sep_algo=3;
    break;
  case '4':
    morse_sep_algo=4;
    break;

  case 'X':	// morse feedback X
  case 'x':
    morse_feedback_X = (int32_t) MENU.calculate_input(morse_feedback_X);
    spr.fillCircle(morse_feedback_X, morse_feedback_Y, morse_feedback_R, morse_feedback_COLOR_DOT);
    spr.pushSprite(0, 0);
    MENU.outln(morse_feedback_X);
    break;

  case 'Y':	// morse feedback Y
  case 'y':
    morse_feedback_Y = (int32_t) MENU.calculate_input(morse_feedback_Y);
    spr.fillCircle(morse_feedback_X, morse_feedback_Y, morse_feedback_R, morse_feedback_COLOR_DOT);
    spr.pushSprite(0, 0);
    MENU.outln(morse_feedback_Y);
    break;

  case 'R':	// morse feedback R
  case 'r':
    morse_feedback_R = (int32_t) MENU.calculate_input(morse_feedback_R);
    spr.fillCircle(morse_feedback_X, morse_feedback_Y, morse_feedback_R, morse_feedback_COLOR_DOT);
    spr.pushSprite(0, 0);
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

  case 'C':
    spr.fillCircle(morse_feedback_X, morse_feedback_Y, morse_feedback_R, morse_feedback_COLOR_DOT);
    spr.pushSprite(0, 0);
    delay(200);

    spr.fillCircle(morse_feedback_X, morse_feedback_Y, morse_feedback_R, morse_feedback_COLOR_LOONG);
    spr.pushSprite(0, 0);
    delay(200);
    break;

  case 'T':	// change morse time scaling for tests
    morse_TimeUnit = (unsigned long) MENU.calculate_input(morse_TimeUnit);
    MENU.outln(morse_TimeUnit);
    break;

  default:
    return 0;		// token not found in this menu
  }

  return 1;		// token found in this menu
} // D_menu_reaction()
