/*
  morse_modified_display_ATS-MINI.h

  compiled with   #include MORSE_MODIFICATED_DISPLAY
*/


#ifndef MORSE_OUTPUT_BUFFER_SIZE
  #define MORSE_OUTPUT_BUFFER_SIZE	64	// size of morse output buffer
#endif
extern char morse_output_buffer[MORSE_OUTPUT_BUFFER_SIZE];

spr.drawString((char*) morse_output_buffer, morse_text_X, morse_text_Y, 1);
spr.pushSprite(0, 0);
