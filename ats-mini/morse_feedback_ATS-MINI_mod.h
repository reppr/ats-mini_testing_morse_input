/*
  morse_feedback_ATS-MINI_mod.h

  special ADAPTED morse duration feedback for ATS-MINI ESP32-SI4732 Receiver
  loaded by	#include MORSE_MODIFICATED_FEEDBACK
*/

#if ! defined RUNNING_ON_ATS_MINI
  #warning "UNKNOWN GADGET...  Compiling special adapted morse duration feedback for ESP32-SI4732 Receiver"));
#endif

int32_t morse_feedback_X=100;
int32_t morse_feedback_Y=25;
int32_t morse_feedback_R=8;

uint32_t morse_feedback_COLOR_OFF=	TFT_BLACK;
uint32_t morse_feedback_COLOR_DOT=	TFT_YELLOW;
uint32_t morse_feedback_COLOR_DASH=	TFT_GREEN;
uint32_t morse_feedback_COLOR_LOONG=	TFT_MAGENTA;
uint32_t morse_feedback_COLOR_OVERLONG=	TFT_RED;

uint8_t morse_sep_algo=4;	// DEVELOPMENT	TODO: REMOVE

#if defined ESP32 && defined TOKEN_LENGTH_FEEDBACK_TASK	// experimental

#include <freertos/task.h>

TaskHandle_t morse_input_feedback_handle;
extern void trigger_token_duration_feedback();

void /*IRAM_ATTR*/ signal_morse_in(uint8_t token) {
  switch(token) {
  case 'X':	// STOP task and signal OFF
    if(morse_input_feedback_handle != NULL) {
      vTaskDelete(morse_input_feedback_handle);
      morse_input_feedback_handle = NULL;
    }
  case '0':	// switch signal OFF
    spr.fillCircle(morse_feedback_X, morse_feedback_Y, morse_feedback_R, morse_feedback_COLOR_OFF);
    spr.pushSprite(0, 0);
    break;

  case 'S':	// START stop a running task, START a new one and signal ON
    if(morse_input_feedback_handle != NULL) {	// STOP if still running
      vTaskDelete(morse_input_feedback_handle);
      morse_input_feedback_handle = NULL;
    }
    trigger_token_duration_feedback();		// START again
  case '+':	// just switch the signal on
  case '.':	// signal ON	// i.e. start a DOT
    spr.fillCircle(morse_feedback_X, morse_feedback_Y, morse_feedback_R, morse_feedback_COLOR_DOT);
    spr.pushSprite(0, 0);
    break;

  case '-':	// start a DASH
    spr.fillCircle(morse_feedback_X, morse_feedback_Y, morse_feedback_R, morse_feedback_COLOR_DASH);
    spr.pushSprite(0, 0);
    break;

  case '!':	// start a LOONG
    spr.fillCircle(morse_feedback_X, morse_feedback_Y, morse_feedback_R, morse_feedback_COLOR_LOONG);
    spr.pushSprite(0, 0);
    break;

  case 'V':	// start a OVERLOONG
    spr.fillCircle(morse_feedback_X, morse_feedback_Y, morse_feedback_R, morse_feedback_COLOR_OVERLONG);
    spr.pushSprite(0, 0);
    break;

  default:
    DADA(F("invalid token"));
    // MENU.error_ln(F("invalid duration"));
  }
} // signal_morse_in(token)


/* **************************************    ESP32s3 version    ****************************************** */
#if CONFIG_IDF_TARGET_ESP32S3		// s3 version on ESP32s3 boards

// #define MORSE_INPUT_DURATION_FEEDBACK_SHOW_STACK_USE	// DEBUGGING ONLY
void morse_input_duration_feedback(void* dummy) {	// s3 version: CONFIG_IDF_TARGET_ESP32S3
  /*
    give status feedback WHILE THE MORSE KEY IS PRESSED: show when is DASH complete and LOONG has started?  etc
  */
  // signal_morse_in('S');	'S' means START new task and signal ON	// was called by touch_morse_ISR_v3()

  // test different algorithms for morse key pressed duration feedback	// TODO: REMOVE	DEVELOPMENT ONLY
  if(morse_sep_algo==1) { // Tims
    vTaskDelay((TickType_t) (dotTim * morse_TimeUnit / 1000 / portTICK_PERIOD_MS));
    if(touchInterruptGetLastStatus(HARDWARE.morse_touch_input_pin)) {	// looks STILL TOUCHED
      signal_morse_in('-');
      vTaskDelay((TickType_t) (dashTim * morse_TimeUnit / 1000 / portTICK_PERIOD_MS));	// react on LOONG now

      if(touchInterruptGetLastStatus(HARDWARE.morse_touch_input_pin)) {	// looks STILL TOUCHED
	signal_morse_in('!');	// '!' means LOONG has started
	vTaskDelay((TickType_t) (loongTim * morse_TimeUnit / 1000 / portTICK_PERIOD_MS));	// react on OVERLONG now
	if(touchInterruptGetLastStatus(HARDWARE.morse_touch_input_pin)) {	// looks STILL TOUCHED
	  signal_morse_in('V');	// 'V' means OVERLONG has started
	}
      }
    }
  } else
    if(morse_sep_algo==2) {	// algorhythm "limits"
      vTaskDelay((TickType_t) (limit_dot_dash * morse_TimeUnit / 1000 / portTICK_PERIOD_MS));
      if(touchInterruptGetLastStatus(HARDWARE.morse_touch_input_pin)) {	// looks STILL TOUCHED
	signal_morse_in('-');
	vTaskDelay((TickType_t) (limit_dash_loong * morse_TimeUnit / 1000 / portTICK_PERIOD_MS));	// react on LOONG now

	if(touchInterruptGetLastStatus(HARDWARE.morse_touch_input_pin)) {	// looks STILL TOUCHED
	  signal_morse_in('!');	// '!' means LOONG has started
	  vTaskDelay((TickType_t) (limit_loong_overlong * morse_TimeUnit / 1000 / portTICK_PERIOD_MS));	// react on OVERLONG now
	  if(touchInterruptGetLastStatus(HARDWARE.morse_touch_input_pin)) {	// looks STILL TOUCHED
	    signal_morse_in('V');	// 'V' means OVERLONG has started
	  }
	}
      }
    } else if(morse_sep_algo==3) { // algorhythm "diff"
      vTaskDelay((TickType_t) (dotTim * morse_TimeUnit / 1000 / portTICK_PERIOD_MS));
      if(touchInterruptGetLastStatus(HARDWARE.morse_touch_input_pin)) {		// looks STILL TOUCHED
	signal_morse_in('-');
	vTaskDelay((TickType_t) ((dashTim - dotTim) * morse_TimeUnit / 1000 / portTICK_PERIOD_MS));	// react on LOONG now

	if(touchInterruptGetLastStatus(HARDWARE.morse_touch_input_pin)) {	// looks STILL TOUCHED
	  signal_morse_in('!');	// '!' means LOONG has started
	  vTaskDelay((TickType_t) ((loongTim - dashTim - dotTim) * morse_TimeUnit / 1000 / portTICK_PERIOD_MS));	// react on OVERLONG now
	  if(touchInterruptGetLastStatus(HARDWARE.morse_touch_input_pin)) {	// looks STILL TOUCHED
	    signal_morse_in('V');	// 'V' means OVERLONG has started
	  }
	}
      }
  } else if(morse_sep_algo==4) { // algorhythm "diff with limits"	>>>>>>>>>>>>>>>> LOOKS LIKE THE BEST, TEST <<<<<<<<<<<<<<<<
      vTaskDelay((TickType_t) (limit_dot_dash * morse_TimeUnit / 1000 / portTICK_PERIOD_MS));
      if(touchInterruptGetLastStatus(HARDWARE.morse_touch_input_pin)) {	// looks STILL TOUCHED
	signal_morse_in('-');
	vTaskDelay((TickType_t) ((limit_dash_loong - limit_dot_dash) * morse_TimeUnit / 1000 / portTICK_PERIOD_MS));	// react on LOONG now

	if(touchInterruptGetLastStatus(HARDWARE.morse_touch_input_pin)) {	// looks STILL TOUCHED
	  signal_morse_in('!');	// '!' means LOONG has started
	  vTaskDelay((TickType_t) ((limit_loong_overlong - limit_dash_loong) * morse_TimeUnit / 1000 / portTICK_PERIOD_MS));	// react on OVERLONG now
	  if(touchInterruptGetLastStatus(HARDWARE.morse_touch_input_pin)) {	// looks STILL TOUCHED
	    signal_morse_in('V');	// 'V' means OVERLONG has started
	  }
	}
      }
    }

#if defined MORSE_INPUT_DURATION_FEEDBACK_SHOW_STACK_USE	// DEBUGGING ONLY
  MENU.out(F("morse_input_duration_feedback free STACK "));
  MENU.outln(uxTaskGetStackHighWaterMark(NULL));
#endif
  morse_input_feedback_handle = NULL;
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

  // >>>>>>>>>>>>>>>> TOUCHED? <<<<<<<<<<<<<<<<<
  if (touchInterruptGetLastStatus(HARDWARE.morse_touch_input_pin)) {
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

#else // *not*  CONFIG_IDF_TARGET_ESP32S3  ???
  #error 'NOT THE EXPECTED esp32s3 PROCESSOR, aborting'
#endif // processor

#endif // ESP32 && TOKEN_LENGTH_FEEDBACK_TASK	// experimental
/* **************************************************************************************** */
