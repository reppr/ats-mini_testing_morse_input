/*
  morse.h
  version 3 touch only

  (there is an alternative morse implementation in branch MORSE4)
*/

#ifndef MORSE_H

#include <string.h>
using std::string;

#define TOUCH_ISR_VERSION_3	// other code removed, see: morse_unused.txt

#define TOKEN_LENGTH_FEEDBACK_TASK	// using a rtos task for morse duration feedback, experimental
#define MORSE_DURATION_TASK_PRIORITY	2	// was: 0	// TODO: test&trim

#define COMPILE_MORSE_CHEAT_SHEETS	// ;)

#if ! defined MORSE_MONOCHROME_ROW
  #define MORSE_MONOCHROME_ROW	0	// row for monochrome morse display
#endif

//#define MORSE_DECODE_DEBUG	// tokens to letters, commands  REMOVED most of the code
#define MORSE_COMPILE_HELPERS		// compile some functions for info, debugging, *can be left out*

#include "driver/touch_pad.h"   // new ESP32-arduino version needs this for 'touch_pad_set_trigger_mode' declaration
#include "esp_attr.h"

#if defined ESP32
  extern const char* esp_err_to_name(esp_err_t);
#endif

#ifndef MORSE_TOUCH_INPUT_PIN		// TODO: be more flexible
  #define MORSE_TOUCH_INPUT_PIN	13	// use ESP32 touch sensor as morse input	(13 just a test)
#endif

bool morse_uppercase=true;	// lowercase very rarely used, changed many menu interfaces
// menu interface is case sensitive, so I need lowercase

portMUX_TYPE morse_MUX = portMUX_INITIALIZER_UNLOCKED;

// morse tick
unsigned long morse_TimeUnit=100000;

unsigned long morse_start_ON_time=0;
unsigned long morse_start_OFF_time=0;

// morse token timing
uint8_t dotTim=1;
uint8_t dashTim=3;
uint8_t loongTim=9;
uint8_t overlongTim=15;

uint8_t separeTokenTim=1;	// used only for statistics, seems buggy btw
uint8_t separeLetterTim=3;
uint8_t separeWordTim=7;


float limit_debounce;
float limit_dot_dash;
float limit_dash_loong;
float limit_loong_overlong;

#if defined MORSE_COMPILE_HELPERS
// morse_show_limits():	code left for debugging
void morse_show_limits() {
  MENU.out("limit_debounce\t\t");
  MENU.out(limit_debounce);
  MENU.tab();
  MENU.outln((unsigned int) (limit_debounce * morse_TimeUnit));

  MENU.out("limit_dot_dash\t\t");
  MENU.out(limit_dot_dash);
  MENU.tab();
  MENU.outln((unsigned int) (limit_dot_dash * morse_TimeUnit));

  MENU.out("limit_dash_loong\t");
  MENU.out(limit_dash_loong);
  MENU.tab();
  MENU.outln((unsigned int) (limit_dash_loong * morse_TimeUnit));

  MENU.out("limit_loong_overlong\t");
  MENU.out(limit_loong_overlong);
  MENU.tab();
  MENU.outln((unsigned int) (limit_loong_overlong * morse_TimeUnit));

  MENU.ln();
}
#endif // MORSE_COMPILE_HELPERS

#define MORSE_TOKEN_MAX		32		// new 32,  is uint8_t	TODO: test&trimm
// #define MORSE_TOKEN_MAX	128		// was: 128  a *lot* for debugging...	TODO:
volatile uint8_t morse_token_cnt=0;	// count up to MORSE_TOKEN_MAX . _ ! V tokens to form one letter or COMMAND
char morse_SEEN_TOKENS[MORSE_TOKEN_MAX];
float morse_token_duration[MORSE_TOKEN_MAX];	// debugging and statistics

/* I put these in pp #define macros
   as i am not sure about enum and IRAM_ATTR
*/
#define MORSE_TOKEN_nothing	'0'
#define MORSE_TOKEN_dot		'.'

#define MORSE_TOKEN_dash	'-'
//#define MORSE_TOKEN_dash	'_'	// on *your* terminal one *or* the other might look better...

#define MORSE_TOKEN_loong	'!'	// starts a COMMAND sequence
#define MORSE_TOKEN_overlong	'V'

#define MORSE_TOKEN_debounce		'°'
#define MORSE_TOKEN_separeToken		'\''
#define MORSE_TOKEN_separeLetter	' '
#define MORSE_TOKEN_separeWord		'|'

#define MORSE_TOKEN_separe_OTHER	'?'
// not used any more:
//  #define MORSE_TOKEN_OFF		'o'
//  #define MORSE_TOKEN_ON		'°'

#define MORSE_TOKEN_ILLEGAL		ILLEGAL8	// was: '§' multichar

bool is_real_token(char token) {	// check for real tokens like . - ! V and separation (word, letter)
  switch(token) {			//	reject all debugging and debounce stuff
  case MORSE_TOKEN_dot:
  case MORSE_TOKEN_dash:
  case MORSE_TOKEN_loong:
  case MORSE_TOKEN_overlong:
  case MORSE_TOKEN_separeLetter:
  case MORSE_TOKEN_separeWord:
    return true;
  }

  return false;
}

// forward declaration:
  void morse_received_token(char token, float token_duration);

#if defined MORSE_COMPILE_HELPERS	// *can* be left out...
void morse_show_tokens(bool show_all = false) {	// set show_all=true  for token debug output
  char token;
  bool last_was_separeLetter=false;	// normally we want to see *only the first space* of a sequence

  if(morse_token_cnt) {
    for (int i=0; i<morse_token_cnt; i++) {
      switch (token=morse_SEEN_TOKENS[i]) {
      case MORSE_TOKEN_dot:
      case MORSE_TOKEN_dash:
      case MORSE_TOKEN_loong:
      case MORSE_TOKEN_overlong:
      case MORSE_TOKEN_separeWord:
	MENU.out(token);	// known real tokens
	last_was_separeLetter=false;
	break;

      case MORSE_TOKEN_separeLetter:	// *only one* space separe letter
	if (!last_was_separeLetter)
	  MENU.out(token);	// space

	if(!show_all)
	  last_was_separeLetter=true;	// show_all *does show* sequences of spaces
	break;

      default:
	if(show_all) {
	  last_was_separeLetter=false;
	  MENU.out(token);	// show *all* tokens
	}
      }
    }
    // MENU.ln();
  }
}
#endif

uint8_t morse_expected_Tims(char token) {	// ATTENTION: returns 0 for unknown/unreal tokens
  uint8_t morse_Tims=0;

  switch(token) {				//	drop all debugging and debounce stuff
  case MORSE_TOKEN_dot:
    morse_Tims = dotTim;
    break;
  case MORSE_TOKEN_dash:
    morse_Tims = dashTim;
    break;
  case MORSE_TOKEN_loong:
    morse_Tims = loongTim;
    break;
  case MORSE_TOKEN_overlong:
    morse_Tims = overlongTim;
    break;
  case MORSE_TOKEN_separeWord:
    morse_Tims = separeWordTim;
    break;
  case MORSE_TOKEN_separeLetter:
    morse_Tims = separeLetterTim;
    break;
  }

  return morse_Tims;
}


void morse_debug_token_info() {		// DEBUG ONLY	// a *lot* of debug info...
  char token;

  if(morse_token_cnt) {
    for (int i=0; i<morse_token_cnt; i++) {
      token=morse_SEEN_TOKENS[i];
      MENU.out(token);	// show *all* tokens
      MENU.space();
      MENU.out(morse_token_duration[i], 4);
      if(is_real_token(token)) {
	  MENU.tab();
	  MENU.out(F("> "));
	  MENU.out(token);
	  MENU.tab();
	  MENU.out(morse_token_duration[i], 4);
	  if(morse_expected_Tims(token)) {
	    MENU.tab();
	    MENU.out(morse_token_duration[i] / morse_expected_Tims(token), 4);
	  }
      }
      MENU.ln();
    }
    MENU.outln(">>>done\n");
  }
} // morse_debug_token_info()


/* **************************************************************** */
// GPIO ISR code removed

/* **************************************************************** */
// touch ISR:	touch_morse_ISR()
void morse_stats_gather(char token, float duration);	// forwards declaration

#if defined MORSE_TOUCH_INPUT_PIN
#if defined TOUCH_ISR_VERSION_3	// currently used version, removed all older implementations

// data structure to save morse touch events from touch_morse_ISR_v3():
typedef struct morse_seen_events_t {
  int type=0;		// 0 is UNTOUCHED	1 is ON
  unsigned long time=0;
} morse_seen_events_t;


#if ! defined MORSE_EVENTS_MAX
  #define MORSE_EVENTS_MAX	256			// TODO: TEST&TRIMM:
#endif
volatile IRAM_ATTR morse_seen_events_t morse_events_cbuf[MORSE_EVENTS_MAX];			// new: IRAM_ATTR
volatile IRAM_ATTR unsigned int morse_events_write_i=0;	// morse_events_cbuf[i]  write index	// new: IRAM_ATTR
unsigned IRAM_ATTR int morse_events_read_i=0;		// morse_events_cbuf[i]  read index	// new: IRAM_ATTR

volatile bool too_many_events=false;			// error flag set by touch_morse_ISR_v3()

unsigned long morse_letter_separation_expected=0L;	// flag or time of automatic letter separation
							//		after input stays idle, untouched

/* ******************************************************************************** */
#if defined MORSE_MODIFICATED_FEEDBACK	// special morse input feedback, like on a fast screen, i.e. ats-mini
  #include MORSE_MODIFICATED_FEEDBACK

#else // compile inbuilt version based on MORSE_OUTPUT_PIN LED feedback

#if defined ESP32 && defined TOKEN_LENGTH_FEEDBACK_TASK	// experimental

#include <freertos/task.h>

TaskHandle_t morse_input_feedback_handle;
extern void trigger_token_duration_feedback();

void /*IRAM_ATTR*/ signal_morse_in(uint8_t token) {
  switch(token) {
  case 'X':	// STOP task and signal OFF
#if defined TOKEN_LENGTH_FEEDBACK_TASK	// experimental
    if(morse_input_feedback_handle != NULL) {
      vTaskDelete(morse_input_feedback_handle);
      morse_input_feedback_handle = NULL;
    }
#endif
  case '0':	// switch signal OFF
    digitalWrite(HARDWARE.morse_output_pin, LOW);
    //    spr.fillCircle(morse_feedback_X, morse_feedback_Y, morse_feedback_R, morse_feedback_COLOR_OFF);
    //    drawText("     ", morse_feedback_X, morse_feedback_Y, 1);
    break;

  case 'S':	// START stop a running task, START a new one and signal ON
#if defined TOKEN_LENGTH_FEEDBACK_TASK	// experimental
    if(morse_input_feedback_handle != NULL) {	// STOP if still running
      vTaskDelete(morse_input_feedback_handle);
      morse_input_feedback_handle = NULL;
    }
    trigger_token_duration_feedback();		// START again
#endif // TOKEN_LENGTH_FEEDBACK_TASK	// experimental
  case '+':	// just switch the signal on
  case '.':	// signal ON	// i.e. start a dot
    digitalWrite(HARDWARE.morse_output_pin, HIGH);		// feedback: pin is TOUCHED, LED on
    //    drawText("ON", morse_feedback_X, morse_feedback_Y, 1);
    //    spr.fillCircle(morse_feedback_X, morse_feedback_Y, morse_feedback_R, morse_feedback_COLOR_DOT);
    break;

//  other implementations might need that, so code left in here.  see: MORSE_MODIFICATED_FEEDBACK
//  case '-':	// start a dash
//    // noop on MORSE_OUTPUT_PIN
//    //    drawText("DASH", morse_feedback_X, morse_feedback_Y, 1);
//    //    spr.fillCircle(morse_feedback_X, morse_feedback_Y, morse_feedback_R, morse_feedback_COLOR_DASH);
//    break;
//  case '!':	// start a loong
//    // noop on MORSE_OUTPUT_PIN
//    //    drawText("LOONG", morse_feedback_X, morse_feedback_Y, 1);
//    //    spr.fillCircle(morse_feedback_X, morse_feedback_Y, morse_feedback_R, morse_feedback_COLOR_LOONG);
//    break;
//  case 'V':	// start a overloong
//    // noop on MORSE_OUTPUT_PIN
//    //    drawText("_____", morse_feedback_X, morse_feedback_Y, 1);
//    //    spr.fillCircle(morse_feedback_X, morse_feedback_Y, morse_feedback_R, morse_feedback_COLOR_OVERLONG);
//    break;

  default:
    DADA(F("invalid token"));
    //  MENU.error_ln(F("invalid duration"));
  }
} // signal_morse_in(token)


/* **************************************  classic ESP32  ****************************************** */
#if CONFIG_IDF_TARGET_ESP32	// version on classic ESP32 boards

// #define MORSE_INPUT_DURATION_FEEDBACK_SHOW_STACK_USE	// DEBUGGING ONLY
void morse_input_duration_feedback(void* dummy) {	// version: CONFIG_IDF_TARGET_ESP32 classic
  /*
    give status feedback while the morse key is pressed: when is DASH complete and LOONG has started?  etc
  */
  // signal_morse_in('S');  was called by touch_morse_ISR_v3()
  if(touchRead(HARDWARE.morse_touch_input_pin) < HARDWARE.touch_threshold) {	// looks STILL TOUCHED
    //vTaskDelay((TickType_t) (dashTim * morse_TimeUnit / 1000 / portTICK_PERIOD_MS));		// ignore beginning of the DASH

    vTaskDelay((TickType_t) (limit_dash_loong * morse_TimeUnit / 1000 / portTICK_PERIOD_MS));	// react on LOONG now
    if(touchRead(HARDWARE.morse_touch_input_pin) < HARDWARE.touch_threshold) {	// looks STILL TOUCHED
#if defined MORSE_OUTPUT_PIN				// blinkOFF as a sign OVERLONG has been reached
      // *ONLY* MAKES SENSE WITH A LED OUTPUT: MORSE_OUTPUT_PIN
      signal_morse_in('0');	// '0' means just switch signal OFF for a short blinkOFF
      vTaskDelay((TickType_t) 100 / portTICK_PERIOD_MS);				// blinkOFF when dash has ended
      if(touchRead(HARDWARE.morse_touch_input_pin) < HARDWARE.touch_threshold) {	// looks STILL TOUCHED, is LOONG and then OVERLONG
	signal_morse_in('+');	// '+' means just switch signal ON again
      }
      // else  signal_morse_in('X');	// 'X' means STOP task and signal OFF	will be called by touch_morse_ISR_v3()
#endif // MORSE_OUTPUT_PIN
    }
  }
  morse_input_feedback_handle = NULL;
 #if defined MORSE_INPUT_DURATION_FEEDBACK_SHOW_STACK_USE	// DEBUGGING ONLY
  MENU.out(F("morse_input_duration_feedback free STACK "));
  MENU.outln(uxTaskGetStackHighWaterMark(NULL));
 #endif
  vTaskDelete(NULL);
} // morse_input_duration_feedback()	// version: CONFIG_IDF_TARGET_ESP32 classic

void IRAM_ATTR touch_morse_ISR_v3() {	// ISR for CONFIG_IDF_TARGET_ESP32 classic  touch morse input
  unsigned long now = micros();

  portENTER_CRITICAL_ISR(&morse_MUX);
  unsigned long next_index = morse_events_write_i + 1;
  next_index %= MORSE_EVENTS_MAX;

  if(next_index == morse_events_read_i) {		// buffer full?
    too_many_events = true;				//   ERROR too_many_events
    goto morse_isr_exit;				//	   return
  }

  morse_events_cbuf[morse_events_write_i].time = now;	// save time

  // >>>>>>>>>>>>>>>> looks TOUCHED? <<<<<<<<<<<<<<<<
  if(touchRead(HARDWARE.morse_touch_input_pin) < HARDWARE.touch_threshold) {
    touch_pad_set_trigger_mode(TOUCH_TRIGGER_ABOVE);		// wait for touch release
    morse_events_cbuf[morse_events_write_i].type = 1 /*touched*/;
    signal_morse_in('S');		// 'S' means START a new token

    // >>>>>>>>>>>>>>>> looks RELEASED? <<<<<<<<<<<<<<<<
  } else {
    touch_pad_set_trigger_mode(TOUCH_TRIGGER_BELOW);		// wait for next touch
    morse_events_cbuf[morse_events_write_i].type = 0 /*released*/;
    signal_morse_in('X');	// 'X' means STOP task and signal OFF
  }

  morse_events_write_i++;
  morse_events_write_i %= MORSE_EVENTS_MAX;		// it's a ring buffer

 morse_isr_exit:
  portEXIT_CRITICAL_ISR(&morse_MUX);
} // touch_morse_ISR_v3()	 CONFIG_IDF_TARGET_ESP32 classic



/* **************************************    ESP32s3 version    ****************************************** */
#elif CONFIG_IDF_TARGET_ESP32S3		// s3 version on ESP32s3 boards

// #define MORSE_INPUT_DURATION_FEEDBACK_SHOW_STACK_USE	// DEBUGGING ONLY
void morse_input_duration_feedback(void* dummy) {	// s3 version: CONFIG_IDF_TARGET_ESP32S3
  /*
    give status feedback while the morse key is pressed: when is DASH complete and LOONG has started?  etc
  */
  // signal_morse_in('S');	'S' means START new task and signal ON	// was called by touch_morse_ISR_v3()
  if(touchInterruptGetLastStatus(MORSE_TOUCH_INPUT_PIN)) {	// looks STILL TOUCHED
    //vTaskDelay((TickType_t) (dashTim * morse_TimeUnit / 1000 / portTICK_PERIOD_MS));	// ignore beginning of the DASH

    vTaskDelay((TickType_t) (limit_dash_loong * morse_TimeUnit / 1000 / portTICK_PERIOD_MS));	// react on LOONG now
    if(touchInterruptGetLastStatus(MORSE_TOUCH_INPUT_PIN)) {	// looks STILL TOUCHED
#if defined MORSE_OUTPUT_PIN				// blinkOFF as a sign LOONG has been reached
      // *ONLY* MAKES SENSE WITH A LED OUTPUT: MORSE_OUTPUT_PIN
      signal_morse_in('0');	// '0' means just switch signal		// OFF for a short blinkOFF
      vTaskDelay((TickType_t) 100 / portTICK_PERIOD_MS);		// blinkOFF when dash has ended
      if(touchInterruptGetLastStatus(MORSE_TOUCH_INPUT_PIN)) {	// looks STILL TOUCHED, is in LOONG and then OVERLONG
	signal_morse_in('+');	// '+' means just switch signal ON again
      }
      // else  signal_morse_in('X');	// 'X' means STOP task and signal OFF	will be called by touch_morse_ISR_v3()
#endif // MORSE_OUTPUT_PIN
    }
  }
  morse_input_feedback_handle = NULL;
  #if defined MORSE_INPUT_DURATION_FEEDBACK_SHOW_STACK_USE	// DEBUGGING ONLY
    MENU.out(F("morse_input_duration_feedback free STACK "));
    MENU.outln(uxTaskGetStackHighWaterMark(NULL));
  #endif
  vTaskDelete(NULL);
} // morse_input_duration_feedback()	// version: CONFIG_IDF_TARGET_ESP32S3


void IRAM_ATTR touch_morse_ISR_v3() {	// s3 ISR for CONFIG_IDF_TARGET_ESP32S3 touch sensor as morse input	*NEW VERSION 3*	  ESP32s3 variant
  unsigned long now = micros();

  portENTER_CRITICAL_ISR(&morse_MUX);
  unsigned long next_index = morse_events_write_i + 1;
  next_index %= MORSE_EVENTS_MAX;

  if(next_index == morse_events_read_i) {		// buffer full?
    too_many_events = true;				//   ERROR too_many_events
    goto morse_isr_exit;				//	   return
  }

  morse_events_cbuf[morse_events_write_i].time = now;	// save time

  // >>>>>>>>>>>>>>>> TOUCHED? <<<<<<<<<<<<<<<<
  if (touchInterruptGetLastStatus(MORSE_TOUCH_INPUT_PIN)) {
    morse_events_cbuf[morse_events_write_i].type = 1 /*touched*/;
    signal_morse_in('S');		// 'S' means START a new token

  // >>>>>>>>>>>>>>>> RELEASED? <<<<<<<<<<<<<<<<
  } else {
    morse_events_cbuf[morse_events_write_i].type = 0 /*released*/;
    signal_morse_in('X');	// 'X' means STOP task and signal OFF
  }

  morse_events_write_i++;
  morse_events_write_i %= MORSE_EVENTS_MAX;		// it's a ring buffer

 morse_isr_exit:
  portEXIT_CRITICAL_ISR(&morse_MUX);
} // touch_morse_ISR_v3()	s3 CONFIG_IDF_TARGET_ESP32S3

#endif	// CONFIG_IDF_TARGET_ESP32S3  vs  CONFIG_IDF_TARGET_ESP32

#endif // ESP32 && TOKEN_LENGTH_FEEDBACK_TASK	// experimental
/* **************************************************************************************** */
#endif // inbuilt vs MORSE_MODIFICATED_FEEDBACK version


void trigger_token_duration_feedback() {
  BaseType_t err = xTaskCreatePinnedToCore(morse_input_duration_feedback,	// function
					   "morse_duration",			// name
					   1024,				// stack size	or: 4*1024   TODO: test! <<<<<<<<<<<<<<<<
					   NULL,				// task input parameter
					   MORSE_DURATION_TASK_PRIORITY,	// task priority
					   &morse_input_feedback_handle,	// task handle
					   0);					// core 0
  if(err != pdPASS) {
    MENU.out(err);
    MENU.space();
    ERROR_ln(F("morse input feedback"));
  }
} // trigger_token_duration_feedback()


bool check_and_treat_morse_events_v3() {	// polled from pulses.ino main loop()	*NEW VERSION 3*
  static int last_seen_type=ILLEGAL32;

  static unsigned long last_seen_touch_time=ILLEGAL32;
  static unsigned long last_seen_release_time=ILLEGAL32;

  float scaled_touch_duration = 0.0;
  float scaled_released_duration = 0.0;

  portENTER_CRITICAL(&morse_MUX);

  if(morse_events_read_i == morse_events_write_i) {
    bool retval=false;	// FIXME: unused?
    if(morse_letter_separation_expected) {
      extern bool morse_poll_letter_separation();
      retval = morse_poll_letter_separation();
    }
    portEXIT_CRITICAL(&morse_MUX);
    return retval;			// NO DATA, or no letter separation check needed
  }

  if(too_many_events) {			// ERROR buffer too small
    too_many_events = false;
    portEXIT_CRITICAL(&morse_MUX);

    ERROR_ln(F("morse event buffer too small"));
    return true;			// give up (for this round)

  } else {	// (! too_many_events)		OK,  *NO ISR ERROR*
     // check if already seen same type?
    if(morse_events_cbuf[morse_events_read_i].type == last_seen_type) { // repeated: already seen SAME TYPE?
      morse_events_read_i++;				// just ignore?
      morse_events_read_i %= MORSE_EVENTS_MAX;

      portEXIT_CRITICAL(&morse_MUX);
      return true;

    } else {		// NORMAL CASE:  >>>>>>>>>>>>>>>>  *NEW TYPE* was detected  <<<<<<<<<<<<<<<<

      last_seen_type = morse_events_cbuf[morse_events_read_i].type;

      morse_letter_separation_expected=0L;	// default, will be set if appropriate

      if(last_seen_type /* == new TOUCH */) {	//    >>>>>>>>>>>>>>>> seen TOUCHED now <<<<<<<<<<<<<<<<
	last_seen_touch_time = morse_events_cbuf[morse_events_read_i].time;
	scaled_released_duration = (float) (last_seen_touch_time - last_seen_release_time) / morse_TimeUnit;
	if(scaled_released_duration <= 0.0) {	// SAVETY NET, should not happen
	  MENU.out_Error_();
	  MENU.out(F("TODO:  negative scaled_released_duration "));
	  MENU.outln(scaled_released_duration);
	} // TODO: *does* this ever happen?

	if (scaled_released_duration >= separeWordTim) {		// word end?
	  morse_received_token(MORSE_TOKEN_separeLetter, scaled_released_duration);	// " |" is word end
	  morse_received_token(MORSE_TOKEN_separeWord, scaled_released_duration);

	} else if (scaled_released_duration >= separeLetterTim) {	// letter end?
	  morse_received_token(MORSE_TOKEN_separeLetter, scaled_released_duration);

	} else if (scaled_released_duration <= limit_debounce) {	// ignore debounce
	  ;
	} else  {							// token end
	  morse_stats_gather(MORSE_TOKEN_separeToken, scaled_released_duration);
	}

      } else { /* new RELEASE */	//    >>>>>>>>>>>>>>>> seen RELEASED now <<<<<<<<<<<<<<<<
	last_seen_release_time = morse_events_cbuf[morse_events_read_i].time;
	scaled_touch_duration = (float) (last_seen_release_time - last_seen_touch_time) / morse_TimeUnit;
	if(scaled_touch_duration <= 0.0) {		// SAVETY NET, should not happen
	  MENU.out_Error_();
	  MENU.out(F("TODO:  negative touch duration "));
	  MENU.outln(scaled_touch_duration);
	} // TODO: *does* this ever happen?

	if (scaled_touch_duration > limit_debounce) {		// *REAL INPUT* no debounce
	  if(scaled_touch_duration > limit_loong_overlong) {
	    morse_received_token(MORSE_TOKEN_overlong, scaled_touch_duration);

	  } else if (scaled_touch_duration > limit_dash_loong) {
	    morse_received_token(MORSE_TOKEN_loong, scaled_touch_duration);

	  } else if (scaled_touch_duration > limit_dot_dash) {
	    morse_received_token(MORSE_TOKEN_dash, scaled_touch_duration);

	  } else {
	    morse_received_token(MORSE_TOKEN_dot, scaled_touch_duration);
	  }
	} // real input?
	else {						// DEBOUNCE, ignore
	  ;
	} // debounce
      } // touched | RELEASED ?

      morse_events_read_i++;
      morse_events_read_i %= MORSE_EVENTS_MAX;

      if(morse_events_read_i == morse_events_write_i) {	// waiting untouched, if no input follows it would miss separeLetter
	morse_letter_separation_expected = last_seen_release_time + (separeLetterTim * morse_TimeUnit)*3; // wait 3* as long...
											// TODO: TEST&TRIMM: wait 3* as long...
      } // set morse_letter_separation_expected ?
    } // NEW or repeated type?
  } // ! (too_many_events) ISR error?

  portEXIT_CRITICAL(&morse_MUX);
  return true;
} // check_and_treat_morse_events_v3()

void show_morse_event_buffer() {	// not used, DEBUGGING ONLY
  MENU.outln(F("\nmorse events circular buffer"));
  for(int i=0; i<MORSE_EVENTS_MAX; i++) {
    MENU.out(i);
    MENU.space();
    if(i<10)
      MENU.space();

    if(i==morse_events_write_i)
      MENU.out(F("<WP>"));
    else
      MENU.space(4);

    if(i==morse_events_read_i)
      MENU.out(F("<RP>"));
    else
      MENU.space(4);

    MENU.tab();
    switch(morse_events_cbuf[i].type) {
    case 0:
      MENU.out(F("released"));
      break;
    case 1:
      MENU.out(F("TOUCHED "));
      break;

    default:
      MENU.out(morse_events_cbuf[i].type);
      MENU.out(F("\t?"));
    }

    MENU.tab();
    MENU.outln(morse_events_cbuf[i].time);
  }
  MENU.ln();
} // show_morse_event_buffer()

bool morse_poll_letter_separation() {
  if(morse_letter_separation_expected == 0L)		// we *might* miss one at a 0, i don't mind ;)
    return false;	// nothing to do, return
  // else

  unsigned long now = micros();
  signed long diff = now - morse_letter_separation_expected;
  if(diff < 0)		// not time yet
    return false;	//   do *not* block while polling
  else {		// it *is* time
    // morse_MUX is respected by caller
    morse_received_token(MORSE_TOKEN_separeLetter, separeLetterTim /*that's a fake!*/);
    morse_letter_separation_expected = 0L;
    return true;
  }
} // morse_poll_letter_separation()  TOUCH_ISR_VERSION_3
#endif // TOUCH_ISR_VERSION_3

#endif // MORSE_TOUCH_INPUT_PIN
/* **************************************************************** */

// removed obsolete code:
//   char morse_OUTPUT_buffer[MORSE_OUTPUT_MAX]={'0'};
//   void morse_2output_buffer(char letter);
//   void show_morse_output_buffer()
//   void morse_store_as_letter(char token);
//   char morse_2ACTION();


/* **************************************************************** */
// morse token duration statistics
uint16_t morse_stats_dot_cnt=0;			// sample count dot
float morse_stats_dot_duration_=0.0;		// summing up dot durations
uint16_t morse_stats_dash_cnt=0;		// sample count dash
float morse_stats_dash_duration_=0.0;		// summing up dash durations
uint16_t morse_stats_loong_cnt=0;		// sample count loong
float morse_stats_loong_duration_=0.0;		// summing up loong durations
uint16_t morse_stats_separeToken_cnt=0;		// sample count separeToken
float morse_stats_separeToken_duration_=0.0;	// summing up separeToken durations
uint16_t morse_stats_separeLetter_cnt=0;	// sample count separeLetter
float morse_stats_separeLetter_duration_=0.0;	// summing up separeLetter durations
// uint16_t morse_stats_separeWord_cnt=0;	// sample count separeWord
// float morse_stats_separeWord_duration_=0.0;	// summing up separeWord durations

// auto adapt
float morse_stats_mean_dash_factor=1.0;		// last letters mean dash duration factor
						//	(also used as flag for display)

void morse_stats_init() {
  morse_stats_dot_cnt = 0;
  morse_stats_dot_duration_ = 0.0;
  morse_stats_dash_cnt = 0;
  morse_stats_dash_duration_ = 0.0;
  morse_stats_loong_cnt = 0;
  morse_stats_loong_duration_= 0.0;
  morse_stats_separeToken_cnt = 0;
  morse_stats_separeToken_duration_ = 0.0;
  morse_stats_separeLetter_cnt = 0;
  morse_stats_separeLetter_duration_ = 0.0;
  // morse_stats_separeWord_cnt = 0;
  // morse_stats_separeWord_duration_ = 0.0;
}


// TODO: comment ################
void morse_stats_gather(char token, float duration) {	// only real tokens please
  switch (token) {
  case MORSE_TOKEN_dot:
    morse_stats_dot_duration_ += duration;
    morse_stats_dot_cnt++;
    break;
  case MORSE_TOKEN_dash:
    morse_stats_dash_duration_ += duration;
    morse_stats_dash_cnt++;
    break;
  case MORSE_TOKEN_loong:
    morse_stats_loong_duration_ += duration;
    morse_stats_loong_cnt++;
    break;
  case MORSE_TOKEN_separeToken:
    morse_stats_separeToken_duration_ += duration;
    morse_stats_separeToken_cnt++;
    break;
  case MORSE_TOKEN_separeLetter:
    morse_stats_separeLetter_duration_ += duration;
    morse_stats_separeLetter_cnt++;
    break;
  }
} // morse_stats_gather()


float morse_saved_stats_dot=0.0;
float morse_saved_stats_dash=0.0;
float morse_saved_stats_loong=0.0;
float morse_saved_stats_separeToken=0.0;
float morse_saved_stats_separeLetter=0.0;
// float morse_saved_stats_separeWord=0.0;


unsigned short morse_stat_ID=0;		// flag (counter)	// flag (counter) for display control
unsigned short morse_stat_seen_ID=0;	// flag (counter)	// flag (counter) for display control

void static IRAM_ATTR morse_stats_save() {	// call immediately after morse_stats_do() to save statistics
  morse_stat_ID++;	// display flag (counter)

  if(morse_stats_dot_cnt)
    morse_saved_stats_dot = morse_stats_dot_duration_;

  if(morse_stats_dash_cnt)
    morse_saved_stats_dash = morse_stats_dash_duration_;

  if(morse_stats_loong_cnt)
    morse_saved_stats_loong = morse_stats_loong_duration_;

  if(morse_stats_separeToken_cnt)
    morse_saved_stats_separeToken = morse_stats_separeToken_duration_;

  if(morse_stats_separeLetter_cnt)
    morse_saved_stats_separeLetter = morse_stats_separeLetter_duration_;

//  if(morse_stats_separeWord_cnt)
//    morse_saved_stats_separeWord = morse_stats_separeWord_duration_;
} // morse_stats_save()

void morse_show_saved_stats() {
  morse_stat_seen_ID = morse_stat_ID;	// flag (counter) for display control

  MENU.outln(F("morse duration stats"));

  if(morse_saved_stats_dot) {
    MENU.out(F("dot\t\t"));
    MENU.outln(morse_saved_stats_dot);
  }
  if(morse_saved_stats_dash) {
    MENU.out(F("dash\t\t"));
    MENU.outln(morse_saved_stats_dash / dashTim);
  }
  if(morse_saved_stats_loong) {
    MENU.out(F("loong\t\t"));
    MENU.outln(morse_saved_stats_loong / loongTim);
  }
  if(morse_saved_stats_separeToken) {
    MENU.out(F("separeToken\t"));
    MENU.outln(morse_saved_stats_separeToken / separeTokenTim);
  }
  if(morse_saved_stats_separeLetter) {
    MENU.out(F("separeLetter\t\t"));
    MENU.outln(morse_saved_stats_separeLetter / separeLetterTim);
  }
//  if(morse_saved_stats_separeWord) {
//    MENU.out(F("separeWord\t\t"));
//    MENU.outln(morse_saved_stats_separeWord / separeWordTim);
//  }

  MENU.out(F("mean dash factor "));
  MENU.out(morse_stats_mean_dash_factor);
  MENU.tab();
  MENU.out(F("morse_TimeUnit "));
  MENU.outln(morse_TimeUnit);
} // morse_show_saved_stats()

void static IRAM_ATTR morse_stats_do() {
  if(morse_stats_dot_cnt) {
    morse_stats_dot_duration_ /= morse_stats_dot_cnt;
  }

  if(morse_stats_dash_cnt) {
    morse_stats_dash_duration_ /= morse_stats_dash_cnt;

    // auto adapt, 1st simple try: only look at dashs
    morse_stats_mean_dash_factor = morse_stats_dash_duration_ / dashTim; // save for auto speed adaption
  }

  if(morse_stats_loong_cnt) {
    morse_stats_loong_duration_ /= morse_stats_loong_cnt;
  }

  if(morse_stats_separeToken_cnt) {
    morse_stats_separeToken_duration_ /= morse_stats_separeToken_cnt;
  }

  if(morse_stats_separeLetter_cnt) {
    morse_stats_separeLetter_duration_ /= morse_stats_separeLetter_cnt;
  }

  morse_stats_save();		// called from morse_stats_do()
} // morse_stats_do()

/* **************************************************************** */
void static morse_token_decode();	// pre declaration

void morse_received_token(char token, float duration) {
  // portENTER_CRITICAL(&morse_MUX);	// respected by caller

  if(morse_token_cnt < MORSE_TOKEN_MAX) {	// buffer not full?
    extern short morse_out_buffer_cnt;
    if(morse_out_buffer_cnt==0 && token==MORSE_TOKEN_overlong) {	// 'V' overlong cannot be starting token
      // MENU.outln(F("skip 'V'"));					//     but sometimes it *is* (i.e. when starting morse input), silently ignore
      return;
    }
    morse_token_duration[morse_token_cnt] = duration;	// SAVE TOKEN and duration
    morse_SEEN_TOKENS[morse_token_cnt++] = token;

    if(is_real_token(token)) {	// react on REAL morse tokens
      switch (token) {		// save letters?, do statistics?
      case MORSE_TOKEN_separeLetter:
	if(morse_token_cnt == 1) {	// ignore separation tokens on startup
	  --morse_token_cnt;		//   remove 1st from buffer
	} else {			// MORSE_TOKEN_separeLetter  ok
	  morse_stats_do();	// do statistics on received letter
	  morse_stats_init();	// prepare stats for next run

	  morse_token_decode();
	}
	break;

      case MORSE_TOKEN_separeWord:
	if(morse_token_cnt == 1) {	// ignore separation tokens on startup
	  --morse_token_cnt;		//   remove from buffer
	} else {
	  //	morse_save_symbol(token);
	}
	break;

      default:
	morse_stats_gather(token, duration);
      }
    }
    // else unreal token
  } else {
    ERROR_ln(F("MORSE_TOKEN_MAX	buffer cleared"));
    morse_token_cnt=0;	// TODO: maybe still use data or use a ring buffer?
  }
} // morse_received_token()


#include "morse_definitions.h"


uint8_t morse_def_token_cnt=0;
char morse_def_TYPE=ILLEGAL8;
string * morse_def_TOKENS;	// TODO: fix or remove ################
string * morse_def_UPPER_Letter;
string * morse_def_LOWER_Letter;
string * morse_def_Letter_in_CASE;
string * morse_def_COMMAND;

#define morse_def_TOKENS_LEN	16	// up to 15 tokens for morse pattern strings
#define morse_def_LETTER_LEN	8	// up to 7  chars for compound morse "letter" strings
#define morse_def_COMMAND_LEN	12	// up to 11 chars for morse command names


// show the words of a space separated string
int space_separated_string_WORDS(string * source) {	/* for debugging only */
  int cnt=0, size=0, pos=0;
  string word;
  while (size >= 0) {
    size = source->find(' ', pos);
    word = source->substr(pos, size - pos);
    MENU.outln(word.c_str());
    pos = size;
    pos++;
    cnt++;
  }

  MENU.outln(cnt);
  return cnt;
}

// TODO: comments ################################################################
string morse_DEFINITION;
string morse_DEFINITION_TOKENS;
char morse_PRESENT_UPPER_Letter;
char morse_PRESENT_LOWER_Letter;
char morse_PRESENT_in_case_Letter;
char morse_PRESENT_TYPE='\0';
string morse_PRESENT_COMMAND;

uint8_t morse_seen_TOKENS=0;
uint8_t morse_SYMBOLS_cnt=0;

void morse_show_definition() {
  if(morse_DEFINITION.length()) {
    MENU.out(morse_PRESENT_TYPE);
    MENU.space();
    MENU.out(morse_DEFINITION_TOKENS.c_str());
    MENU.space();
    switch (morse_PRESENT_TYPE) {
    case '*':
      MENU.out(morse_PRESENT_UPPER_Letter);
      MENU.space();
      MENU.out(morse_PRESENT_LOWER_Letter);
      MENU.space();
      if(morse_PRESENT_in_case_Letter)
	MENU.out(morse_PRESENT_in_case_Letter);
      break;

    case 'C':
      MENU.outln(morse_PRESENT_COMMAND.c_str());	// TODO: do I still need .c_str() ??? ################################
      break;
    }
  }
}


void morse_reset_definition(string source) {
  morse_DEFINITION = source;

  morse_DEFINITION_TOKENS="";
  morse_PRESENT_UPPER_Letter='\0';
  morse_PRESENT_LOWER_Letter='\0';
  morse_PRESENT_in_case_Letter='\0';
  morse_PRESENT_TYPE='\0';
  morse_PRESENT_COMMAND="";

  morse_seen_TOKENS=0;
  morse_SYMBOLS_cnt=0;
}

void morse_read_definition(int index) {	// keep index in range, please
  int cnt=0, size=0, pos=0;
  string dataset, word;
  dataset = morse_definitions_tab[index];
  morse_reset_definition(dataset);

  while (size >= 0) {
    size = dataset.find(' ', pos);
    word = dataset.substr(pos, size - pos);
    pos = size;
    pos++;

    switch (cnt) {
    case 0:
      morse_SYMBOLS_cnt = word[0] - '0';
      break;
    case 1:
      morse_PRESENT_TYPE = word[0];
      break;
    case 2:
      morse_DEFINITION_TOKENS = word;
      break;
    case 3:
      if(morse_PRESENT_TYPE=='*') {		// item 3 LETTER mode	= UPPER
	morse_PRESENT_UPPER_Letter = word[0];
      } else if(morse_PRESENT_TYPE=='C') {	// item 3 COMMAND mode	= COMMAND  (last command item)
	morse_PRESENT_COMMAND = word;
	size = -1;	// command read, done
      }
      break;
    case 4:
      if(morse_PRESENT_TYPE=='*') {		// item 4 LETTER mode	= LOWER  (last letter item)
	morse_PRESENT_LOWER_Letter = word[0];
	size = -1;	// letter read, done
      }
      // else  (no check here)
      break;
    }

    if(morse_PRESENT_TYPE=='*') {		// upper | lowercase letters
      if(morse_uppercase)
	morse_PRESENT_in_case_Letter = morse_PRESENT_UPPER_Letter;
      else
	morse_PRESENT_in_case_Letter = morse_PRESENT_LOWER_Letter;
    }

    cnt++;
  }
}

void morse_show_space_delimited_string(const char *p) {		// probably only used for DEBUGGING
  if(p==NULL)
    return;

  char c;
  for(int i=0; (c = p[i]) ; i++) {
    MENU.out(c);
    if(c == ' ')	// *does* show delimiting space
      break;
  }
} // morse_show_space_delimited_string()


void morse_definition_set_show(bool uppercase) {	// probably only used for DEBUGGING
  MENU.ln();
  MENU.out(morse_def_token_cnt);
  MENU.space();
  MENU.out(morse_def_TYPE);
  MENU.space();

  // morse_show_space_delimited_string(morse_def_TOKENS);
  MENU.out(morse_def_TOKENS->c_str());
  MENU.space();

  switch(morse_def_TYPE) {
  case '*':	// letters
    //    morse_show_space_delimited_string(morse_def_Letter_in_CASE);
    MENU.out(morse_def_Letter_in_CASE->c_str());
    break;	// TODO: inserted break, is it ok?
  case 'C':	// COMMANDs
    morse_show_space_delimited_string(morse_def_COMMAND->c_str());
    break;
  }

  MENU.ln();
} // morse_definition_set_show()


int morse_find_definition(const char* pattern) {  // returns index in morse_definitions_tab[i] or ILLEGAL32
  for(int i=0; i<MORSE_DEFINITIONS ; i++) {
    morse_read_definition(i);
    if((string) pattern == morse_DEFINITION_TOKENS) {
#if defined MORSE_DECODE_DEBUG	// left that in here
      MENU.out("== ");
      MENU.outln(pattern);
      morse_show_definition();
#endif
      return i;
    }
  }

  morse_reset_definition("");
  return ILLEGAL32;	// not found
} // morse_find_definition()


/* **************************************************************** */

#ifndef MORSE_OUTPUT_BUFFER_SIZE
  #define MORSE_OUTPUT_BUFFER_SIZE	64	// size of morse output buffer
#endif
char  morse_output_buffer[MORSE_OUTPUT_BUFFER_SIZE]={'0'};	// buffer
short morse_out_buffer_cnt=0;

volatile bool morse_trigger_KB_macro = false;		// triggers morse_PLAY_input_KB_macro()

extern bool musicbox_is_idle();

#if defined HAS_OLED
extern bool monochrome_can_be_used();	// TODO: do i want that *here*?
extern void MC_display_message(const char * message);
// avoid sound glitches when using OLED:
extern bool oled_feedback_while_playing;
extern void MC_printlnBIG(uint8_t row, const char* str);		// 2x2 lines
extern void MC_setInverseFont();
extern void MC_clearInverseFont();
extern void monochrome_setPowerSave(uint8_t value);
extern void MC_setCursor(uint8_t col, uint8_t row);
extern uint8_t monochrome_getCols();
extern void MC_print(const char* str);
extern void monochrome_print_f(float f, int chiffres);
extern void MC_printBIG_at(uint8_t col, uint8_t row, const char* str);	// for short 2x2 strings
extern void monochrome_clear();
#endif // HAS_OLED

void morse_PLAY_input_KB_macro() {	// triggered by morse_clear_display__prepare_action() by setting  morse_trigger_KB_macro = true;
  morse_trigger_KB_macro = false;	// reset trigger

  MENU.out(F("morse "));
  MENU.play_KB_macro((char*) morse_output_buffer);	// *does* the menu feedack
}

#if defined MULTICORE_DISPLAY
void morse_clear_display__prepare_action() {	// can set trigger  morse_trigger_KB_macro = true;	// MULTICORE_DISPLAY version
  morse_output_buffer[morse_out_buffer_cnt]='\0';	// append '\0'
  if(morse_out_buffer_cnt) {
#if defined HAS_ePaper
    if(ePaper_printing_available()) {
      xSemaphoreTake(ePaper_generic_MUX, portMAX_DELAY);
      ePaper_generic_print_job=1376;
      set_used_font(big_mono_font_p);								 // HAS_ePaper
      ePaper.fillRect(0, 0, morse_out_buffer_cnt*used_font_x, used_font_yAdvance, GxEPD_WHITE);	 // HAS_ePaper
      ePaper.display(true);									 // HAS_ePaper
      xSemaphoreGive(ePaper_generic_MUX);
    }

#elif defined HAS_OLED
    MC_printlnBIG(MORSE_MONOCHROME_ROW, "        ");
#endif // HAS_ePaper vs HAS_OLED

    morse_out_buffer_cnt = 0;
    morse_trigger_KB_macro = true;	// *triggers* morse_PLAY_input_KB_macro()
  }
  morse_uppercase = true;	// reset to uppercase
} // morse_clear_display__prepare_action()  MULTICORE

#else // *NO* MULTICORE_DISPLAY

// simple *NO* visible output version, *no* MULTICORE
void morse_clear_display__prepare_action() {	// can set trigger  morse_trigger_KB_macro = true;	    morse_output_buffer[morse_out_buffer_cnt]='\0';	// append '\0'
  if(morse_out_buffer_cnt) {
    morse_out_buffer_cnt = 0;
    morse_trigger_KB_macro = true;	// *triggers* morse_PLAY_input_KB_macro()
  }
  morse_uppercase = true;	// reset to uppercase
} // morse_clear_display__prepare_action()  MULTICORE

#endif // MULTICORE versions  vs  *NO* OUTPUT

char morse_output_char = '\0';	// '\0' means *no* output

#if defined HAS_DISPLAY
void monochrome_out_morse_char() {
 #if defined HAS_OLED
  if(! monochrome_can_be_used())
    return;
 #endif

  if(morse_output_char && morse_out_buffer_cnt) {
    // 2x2 version
    char s[]="  ";
    s[0] = morse_output_char;
 #if defined HAS_ePaper
    if(ePaper_printing_available()) {
      MC_printBIG_at((morse_out_buffer_cnt - 1), MORSE_MONOCHROME_ROW, s, /*offset_y= */ -6);	// HAS_ePaper
      morse_output_char = '\0';	// trigger off
    } else {
      return;	// do that later
    }
#else	// HAS_OLED
    MC_printBIG_at(2*(morse_out_buffer_cnt - 1), MORSE_MONOCHROME_ROW, s, /*offset_y= */ -6);	// HAS_OLED
#endif	// HAS_ePaper vs HAS_OLED
  }

  morse_output_char = '\0';	// trigger off
} // monochrome_out_morse_char()
#endif // HAS_DISPLAY


bool echo_morse_input=true;
bool morse_store_and_show_received_letter(char letter) {	// returns error
  if(echo_morse_input)
    MENU.out(letter);
#if defined HAS_DISPLAY
  morse_output_char = letter;	// char and switch
#endif
  if(morse_out_buffer_cnt < MORSE_OUTPUT_BUFFER_SIZE) {
    morse_output_buffer[morse_out_buffer_cnt++] = letter;
    morse_output_buffer[morse_out_buffer_cnt]='\0';	// append '\0' just in case ;)
    return 0;	// ok
  }

  return true;	// ERROR
}


extern bool accGyro_is_active;
extern uint8_t monochrome_power_save;
void static morse_token_decode() {	// decode received token sequence
/*
  decode received token sequence
  SAVE LETTERS,

  EXECUTE COMMANDS,
  return of commands is currently unused, always 0
*/
  char pattern[10] = { '\0' };	// 10 (up to 9 real tokens + '\0'
  char token;

  pattern[0] = '\0';
  short pattern_length=0;
  if(morse_token_cnt) {
    for (int i=0; i<morse_token_cnt; i++) {
      token = morse_SEEN_TOKENS[i];
      if(is_real_token(token)) {
	if(pattern_length > 8) {	  // ERROR
	  ERROR_ln(F("morse pattern_length"));
	  morse_token_cnt=0;
	  return;
	}
	switch(token) {
	case MORSE_TOKEN_dot:
	case MORSE_TOKEN_dash:
	  pattern[pattern_length++] = token;
	  break;

	case MORSE_TOKEN_separeLetter:		// time to store letter
	  if(morse_find_definition(pattern) != ILLEGAL32) {
	    switch(morse_PRESENT_TYPE) {
	    case '*':	// letter
	      morse_store_and_show_received_letter(morse_PRESENT_in_case_Letter);
	      break;

	    case 'C':	// Command
	      if(morse_PRESENT_COMMAND == "NEXT") {
		if(morse_out_buffer_cnt) {	// if something is buffered, send it,
#if defined MULTICORE_DISPLAY
		  do_on_other_core(&morse_clear_display__prepare_action);	// might set morse_trigger_KB_macro=true
#else
		  morse_clear_display__prepare_action();	// *NO* output version
#endif
		}	// NEXT (*with* stored inputs)
		else {	// bare NEXT

#if defined PULSES_SYSTEM	// bare NEXT in PULSES_SYSTEM 'P'=START/STOP		TODO: for preset mode, TEST: for others
		  morse_store_and_show_received_letter(morse_PRESENT_in_case_Letter = 'P');
#else	// bare NEXT outside PULSES_SYSTEM
		  // just ignoring it gives strange problems...  so:
		  morse_store_and_show_received_letter(morse_PRESENT_in_case_Letter = '?');	// '?' shows menu
#endif	// PULSES_SYSTEM or not

#if defined MULTICORE_DISPLAY
		  do_on_other_core(&morse_clear_display__prepare_action);	// will set trigger  morse_trigger_KB_macro=true
  #else // *NO* output version
		  morse_clear_display__prepare_action();
  #endif // MULTICORE_DISPLAY  vs *NO* output
		}

	      } else if(morse_PRESENT_COMMAND == "LOWER")
		morse_uppercase = false;

	      else if(morse_PRESENT_COMMAND == "UPPER")
		morse_uppercase = true;

	      else if(morse_PRESENT_COMMAND == "DELLAST") {
		if(morse_out_buffer_cnt) {
		  morse_out_buffer_cnt--;
#if defined HAS_ePaper
		  MC_printBIG_at(morse_out_buffer_cnt, MORSE_MONOCHROME_ROW, " ");	// DELLAST	// HAS_ePaper
#elif defined HAS_OLED
		  MC_printBIG_at(2*morse_out_buffer_cnt, MORSE_MONOCHROME_ROW, " ");	// DELLAST	// HAS_OLED
#endif
		}

	      } else if(morse_PRESENT_COMMAND == "OLED") {	// ---.-  "OA"
#if defined HAS_OLED			// (NOOP if no HAS_OLED)
		if(oled_feedback_while_playing ^= 1) {		// got switched on
		  monochrome_power_save = 0;
		  monochrome_setPowerSave(monochrome_power_save);

		  MENU.out(F("OLED"));
		  MENU.out_ON_off(oled_feedback_while_playing);
		  MENU.ln();
		  MC_display_message(F(" OLED "));
		} else {					// got switched off
		  MC_display_message(F(" off "));
		} // oled switched on/off
#else
		;
#endif

	      } else if(morse_PRESENT_COMMAND == "CANCEL") {	// CANCEL
		morse_token_cnt = 0;
		morse_out_buffer_cnt = 0;
#if defined HAS_DISPLAY
		MC_printBIG_at(0, MORSE_MONOCHROME_ROW, "__");	// CANCEL shows "__"	 HAS_ePaper || HAS_OLED
#endif
	      } else if(morse_PRESENT_COMMAND == "ANY1") {	// '----'
		MENU.outln(F("\"ANY1\" currently unused"));

	      } else if(morse_PRESENT_COMMAND == "MACRO_NOW") {	// ...-.  SN
		if(morse_out_buffer_cnt) {
#if defined USE_ESP_NOW
		  extern void esp_now_send_maybe_do_macro(uint8_t* mac_addr, char * macro);
		  morse_output_buffer[morse_out_buffer_cnt]='\0';	// append '\0'
		  extern uint8_t* esp_now_send2_mac_p;
		  esp_now_send_maybe_do_macro(esp_now_send2_mac_p, morse_output_buffer); // send to *esp_now_send2_mac_p
#else
		  MENU.outln(F("*no* USE_ESP_NOW"));
		  MENU.outln(F("TODO: reset morse output buffer?"));	// TODO: TEST:
#endif
		} else {
		  MENU.outln(F("no data to send now"));
		}

#if defined USE_MPU6050_at_ADDR
	      } else if(morse_PRESENT_COMMAND == "UIswM") {	// '..-..'  UI	switch Motion UI on/off
		  MENU.out(F("motion UI "));
		  MENU.out_ON_off(accGyro_is_active ^= 1);	// toggle and show
		  MENU.ln();
  #if defined HAS_OLED
		  if(monochrome_can_be_used()) {
		    MC_setCursor(monochrome_getCols() - 7 ,0);	// position of OLED UI display
		    if(accGyro_is_active) {
		      MC_setInverseFont();
		      MC_print(F("      "));	// TODO: real string
		      MC_clearInverseFont();
		    } else
		      MC_print(F("      "));
		  }
  #endif // HAS_OLED
#endif // USE_MPU6050_at_ADDR

	      } else	// unknown morse command
		{
		  DADA("unknown");		MENU.out("\nCOMMAND:\t"); MENU.outln(morse_PRESENT_COMMAND.c_str());
		}
		break;

	    default:
	      ERROR_ln(F("morse_decode type"));
	      morse_reset_definition("");
	      morse_token_cnt=0;
	      return;
	    }
	  } else {	// no definition found, error
#if defined HAS_DISPLAY
	    if(morse_out_buffer_cnt) {
  #if defined HAS_ePaper
	      MC_printBIG_at(morse_out_buffer_cnt, MORSE_MONOCHROME_ROW, "'");	// TODO: TEST:	 HAS_ePaper
  #elif defined HAS_OLED
	      if(monochrome_can_be_used()) {
		MC_setInverseFont();							// HAS_OLED
		MC_printBIG_at(2*morse_out_buffer_cnt, MORSE_MONOCHROME_ROW, "'");	// HAS_OLED // TODO: TEST:
		MC_clearInverseFont();							// HAS_OLED
	      }
  #endif	// ePaper | OLED
	    }
#endif // HAS_DISPLAY
	    MENU.out(pattern);
	    MENU.space(2);
	    ERROR_ln(F("morse  no definition"));
	    morse_reset_definition("");
	    morse_token_cnt=0;
	    return;
	  }
	  break;

	case MORSE_TOKEN_separeWord:
	  break;

	case MORSE_TOKEN_loong:
	  pattern[pattern_length++] = token;
	  break;

	case MORSE_TOKEN_overlong:
	  pattern[pattern_length++] = token;
	  break;

	default:
	  ERROR_ln(F("morse real token unknown"));
	  morse_reset_definition("");
	  morse_token_cnt=0;
	  return;
	}
      } // real token
    }

    morse_token_cnt=0;
  } // if(morse_token_cnt)
} // morse_token_decode()


#if defined COMPILE_MORSE_CHEAT_SHEETS && defined HAS_DISPLAY
bool /*ok*/ morse_tokens_of_letter(char* result, uint8_t len, char c) {	// uppercase only
  result[0] = '\0';
  char* format = F("%c %s");
  for(int i=0; i < MORSE_DEFINITIONS; i++) {
    morse_read_definition(i);
    if(morse_PRESENT_UPPER_Letter == c) {	// uppercase only
      snprintf(result, len, format, morse_PRESENT_UPPER_Letter, morse_DEFINITION_TOKENS.c_str());
      return true;	// OK
    }
  }
  return false;	// ERROR, not found
} // morse_tokens_of_letter()

#if defined HAS_OLED
  #define CHEAT_BUFLEN	4
#elif defined HAS_ePaper
  #define CHEAT_BUFLEN	8
#endif
char cheat_buffer[CHEAT_BUFLEN] = {0};

void show_cheat_sheet() {
  char c;
  uint8_t maxlen=17;

#if defined HAS_DISPLAY
  monochrome_clear();	// subito, MC_clear_display() does it too late!
#endif
  char result[maxlen];
#if defined HAS_OLED
  uint8_t rows=4;
#elif defined HAS_ePaper
  uint8_t rows=5;
#endif
  for(int i=0; i<rows; i++) {
    if((c = cheat_buffer[i])) {
      if(morse_tokens_of_letter(result, maxlen, c)) {	// uppercase only
#if defined HAS_OLED
	extern uint8_t /*next_row*/ monochrome_big_or_multiline(int row, const char* str);
	monochrome_big_or_multiline(2*i, result);
#elif defined HAS_ePaper
	ePaper_BIG_or_multiline(i, result);	 // HAS_ePaper
#endif
	MENU.outln(result);
      } else {	// morse code not found
	MENU.out(F("unknown symbol "));
	MENU.out(c);
	MENU.tab();
	MENU.outln((int) c);
      }
    } else break;
  }
} // show_cheat_sheet()

void make_morse_cheat_sheet(char* symbols) {
  for(int i=0; i<CHEAT_BUFLEN; i++)
    cheat_buffer[i] = symbols[i];
  do_on_other_core(show_cheat_sheet);
}

void morse_cheat_sheets_UI() {
  static uint8_t cheat_sheet_i = 0;

  bool next_sheet=true;
  int input_value = MENU.numeric_input(cheat_sheet_i);	// use false input as UI to *keep* the same sheet ;)

  #define CHEAT_SHEETS_MAX	8	//	>>>>>>>> *DO* *UPDATE* number of CHEAT_SHEETS_MAX <<<<<<<<<<<<<<
  if(input_value <= CHEAT_SHEETS_MAX  &&  input_value >= 0)
    cheat_sheet_i = (uint8_t) input_value;
  else				// use false input as UI to *keep* the same sheet ;)
    next_sheet = false;

  MENU.out(F("morse cheat sheet "));
  MENU.outln(cheat_sheet_i);

  switch(cheat_sheet_i) {
  case 0:
    make_morse_cheat_sheet(F("BCDG"));
    break;
  case 1:
    make_morse_cheat_sheet(F("FLPQ"));
    break;
  case 2:
    make_morse_cheat_sheet(F("JUVW"));
    break;
  case 3:
    make_morse_cheat_sheet(F("XYZ'"));
    break;
  case 4:
    make_morse_cheat_sheet(F("*/+-"));
    break;
  case 5:
    make_morse_cheat_sheet(F("=!:?"));
    break;
  case 6:
    make_morse_cheat_sheet(F(".,;:"));
    break;
  case 7:
    make_morse_cheat_sheet(F("&$'\""));
    break;
  case 8:
    make_morse_cheat_sheet(F("@"));
    break;
    /*  unkown: |~%<>{} <space>
	case :
	make_morse_cheat_sheet(F("|~% "));
	break;
	case :
	make_morse_cheat_sheet(F("<>{}"));
	break;
    */
  }
  // *DO* update #define CHEAT_SHEETS_MAX

  if(next_sheet) {
    cheat_sheet_i++;
    if(cheat_sheet_i > CHEAT_SHEETS_MAX)
      cheat_sheet_i=0;
  }
} // morse_cheat_sheets_UI()
#endif // COMPILE_MORSE_CHEAT_SHEETS

  /* **************************************************************** */

void morse_init() {
  // MENU.outln(F("MORSE morse_init()"));

  //limit_debounce		= 0.5;
  //limit_debounce		= 0.25;	// *LOWERED* as a test		// TODO: test&trim
  limit_debounce		= 0.35;	// hat some stray 'E' so test 0.35	// TODO: test&trim
  limit_dot_dash		= (float) (dotTim + dashTim + 1)/2;
  limit_dot_dash = 2.0;	// maybe better?	TODO: TEST: ################
  limit_dash_loong	= (float) (dashTim + loongTim + 1 )/2;
  limit_loong_overlong	= (float) (loongTim + overlongTim + 1)/2;

  /*	float mean(int a, int b) is (a+b)/2	*/

#ifdef MORSE_GPIO_INPUT_PIN	// use GPIO with pulldown as morse input	// TODO: update, very old version, untested
  MENU.out(F("MORSE GPIO input pin "));
  MENU.outln(MORSE_GPIO_INPUT_PIN);

  pinMode(MORSE_GPIO_INPUT_PIN, INPUT);
  attachInterrupt(digitalPinToInterrupt(MORSE_GPIO_INPUT_PIN), morse_GPIO_ISR_rising, RISING);
#endif

#ifdef MORSE_TOUCH_INPUT_PIN	// use ESP32 touch sensor as morse input	// in use
  MENU.out(F("MORSE touch pin "));
  MENU.out(HARDWARE.morse_touch_input_pin);
  MENU.out(F("\tinterrupt level "));
  MENU.outln(HARDWARE.touch_threshold);
  // touchAttachInterrupt(HARDWARE.morse_touch_input_pin, touch_morse_ISR, HARDWARE.touch_threshold);	 // do that last, later...
#endif

#ifdef MORSE_OUTPUT_PIN
  MENU.out(F("MORSE output pin "));
  MENU.outln(HARDWARE.morse_output_pin);

  pinMode(HARDWARE.morse_output_pin, OUTPUT);
  digitalWrite(HARDWARE.morse_output_pin, LOW);
#endif

#if defined(MORSE_GPIO_INPUT_PIN) || defined(MORSE_TOUCH_INPUT_PIN)
  // morse_show_limits();	// code left for debugging
#endif

#ifdef MORSE_TOUCH_INPUT_PIN	// use ESP32 touch sensor as morse input
  // do that last, now.
  touchAttachInterrupt(HARDWARE.morse_touch_input_pin, touch_morse_ISR_v3, HARDWARE.touch_threshold);
#endif // MORSE_TOUCH_INPUT_PIN
} // morse_init()


#define MORSE_H
#endif
