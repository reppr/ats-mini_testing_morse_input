/*
  DADA_debugging.h

  debugging helper ;)

  say:
  DADA("some text");
  to display something like
  >>>> DEBUG /home/dada/Arduino/S3/s3_some_tests/s3_some_tests.ino L108	setup()	"finally arrived!"	<<<<<<<<<<<<
*/


#define DADA(txt)	MENU.out(">>>> DEBUG "); MENU.out(__FILE__); MENU.space(); MENU.out('L'); MENU.out(__LINE__); MENU.tab(); MENU.out(__func__); MENU.out("()"); MENU.tab(); MENU.out('"'); MENU.out(txt);  MENU.out('"'); MENU.outln("\t<<<<<<<<<<<<");
