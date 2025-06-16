/*
  morse_feedback_ATS-MINI_mod.h

  loaded by	#include MORSE_MODIFICATED_FEEDBACK
*/

int32_t morse_feedback_X=140;
int32_t morse_feedback_Y=5;
int32_t morse_feedback_R=10;

uint32_t morse_feedback_COLOR_OFF=	TFT_BLACK;
uint32_t morse_feedback_COLOR_DOT=	TFT_YELLOW;
uint32_t morse_feedback_COLOR_DASH=	TFT_GREEN;
uint32_t morse_feedback_COLOR_LOONG=	TFT_ORANGE;
uint32_t morse_feedback_COLOR_OVERLONG=	TFT_RED;

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
    // digitalWrite(HARDWARE.morse_output_pin, LOW);
    // spr.fillCircle(morse_feedback_X, morse_feedback_Y, morse_feedback_R, morse_feedback_COLOR_OFF);
    drawText("     ", morse_feedback_X, morse_feedback_Y, 1);
    break;

  case 'S':	// START stop a running task, START a new one and signal ON
    if(morse_input_feedback_handle != NULL) {	// STOP if still running
      vTaskDelete(morse_input_feedback_handle);
      morse_input_feedback_handle = NULL;
    }
    trigger_token_duration_feedback();		// START again
  case '+':	// just switch the signal on
  case '.':	// signal ON	// i.e. start a dot
    // digitalWrite(HARDWARE.morse_output_pin, HIGH);		// feedback: pin is TOUCHED, LED on
    drawText("ON", morse_feedback_X, morse_feedback_Y, 1);
    // spr.fillCircle(morse_feedback_X, morse_feedback_Y, morse_feedback_R, morse_feedback_COLOR_DOT);
    break;

  case '-':	// start a dash
    // noop on MORSE_OUTPUT_PIN
    // drawText("DASH", morse_feedback_X, morse_feedback_Y, 1);
    // spr.fillCircle(morse_feedback_X, morse_feedback_Y, morse_feedback_R, morse_feedback_COLOR_DASH);
    break;
  case '!':	// start a loong
    // noop on MORSE_OUTPUT_PIN
    drawText("LOONG", morse_feedback_X, morse_feedback_Y, 1);
    // spr.fillCircle(morse_feedback_X, morse_feedback_Y, morse_feedback_R, morse_feedback_COLOR_LOONG);
    break;
  case 'V':	// start a overloong
    // noop on MORSE_OUTPUT_PIN
    // drawText("_____", morse_feedback_X, morse_feedback_Y, 1);
    drawText("OVERLONG", morse_feedback_X, morse_feedback_Y, 1);
    // spr.fillCircle(morse_feedback_X, morse_feedback_Y, morse_feedback_R, morse_feedback_COLOR_OVERLONG);
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
    give status feedback while the morse key is pressed: when is DASH complete and LOONG has started?  etc
  */
  // signal_morse_in('S');	'S' means START new task and signal ON	// was called by touch_morse_ISR_v3()
  if(touchInterruptGetLastStatus(MORSE_TOUCH_INPUT_PIN)) {	// looks STILL TOUCHED
    //vTaskDelay((TickType_t) (dashTim * morse_TimeUnit / 1000 / portTICK_PERIOD_MS));	// ignore beginning of the DASH

    vTaskDelay((TickType_t) (limit_dash_loong * morse_TimeUnit / 1000 / portTICK_PERIOD_MS));	// react on LOONG now
    if(touchInterruptGetLastStatus(MORSE_TOUCH_INPUT_PIN)) {	// looks STILL TOUCHED
      signal_morse_in('!');	// '!' means LOONG has started

      vTaskDelay((TickType_t) (limit_loong_overlong * morse_TimeUnit / 1000 / portTICK_PERIOD_MS));	// react on LOONG now
      if(touchInterruptGetLastStatus(MORSE_TOUCH_INPUT_PIN)) {	// looks STILL TOUCHED
	signal_morse_in('V');	// 'V' means OVERLONG has started
      }
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

#else // *not*  CONFIG_IDF_TARGET_ESP32S3  ???
  #error 'NOT THE EXPECTED esp32s3 PROCESSOR, aborting'
#endif // processor

#endif // ESP32 && TOKEN_LENGTH_FEEDBACK_TASK	// experimental
/* **************************************************************************************** */
