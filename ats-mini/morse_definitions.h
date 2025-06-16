/*
  morse_definitions.h

  2nd implementation: human readable look up table
*/


// morse code see i.e.
// https://www.electronics-notes.com/articles/ham_radio/morse_code/characters-table-chart.php


// the table *is sorted* for quick lookup

// common fixed place entries:
// *		number of tokens 1-9 A-F
//  *		delimiter ' '
//   *		type: '*' is LETTER
//                    'C' is COMMAND
//                    '0' undefined
//    *		delimiter ' '
//     *	first token
//      *	next token until space as delimiter)

// LETTERS:	if type=='*' it continues like this:
//       *	first UPPERCASE letter or compound string, *can* be ' '
//        ****	' ' delimiter or more chars from UPPERCASE COMPOUND string
//            *	first LOWERCASE letter or compound string, *can* be ' '
//             ****	' ' delimiter or more chars from LOWERCASE COMPOUND string

//                 *	delimiter or string end
//                  *<NAME>" comment or extensions

// COMMAND:
// *		number of tokens 1-9 A-F
//  *		delimiter
//  'C'		type: 'C' is COMMAND
//     *...	tokens
//	   *	delimiter
//          *<COMMAND>"
//		      "	STRING END,
//                    " delimiter ' ' is *not* enough here


#ifndef MORSE_DEFINITIONS_H

const char * morse_definitions_tab[] = {
  "1 * . E e",
  "1 * - T t",

  "1 C ! NEXT",		// SEND if morse input,  else  START/STOP playing
  "1 C V CANCEL",

  "2 * .. I i",
  "2 * .- A a",
  "2 * -. N n",
  "2 * -- M m",

  "2 C !. LOWER",
  "2 C !- UPPER",
  "2 C !! DELLAST",

  "3 * ... S s",
  "3 * ..- U u",
  "3 * .-. R r",
  "3 * .-- W w",
  "3 * -.. D d",
  "3 * -.- K k",
  "3 * --. G g",
  "3 * --- O o",

  "4 * .... H h",
  "4 * ...- V v",
  "4 * ..-. F f",
  "4 * ..-- Ü ü",
  "4 * .-.. L l",
  "4 * .-.- Ä ä",
  "4 * .--. P p",
  "4 * .--- J j",
  "4 * -... B b",
  "4 * -..- X x",
  "4 * -.-. C c",
  "4 * -.-- Y y",
  "4 * --.. Z z",
  "4 * --.- Q q",
  "4 * ---. Ö ö",
  "4 C ---- ANY1",	// was: |CH| |ch|

  "5 * ..... 5 5",
  "5 * ....- 4 4",
  "5 C ...-. MACRO_NOW",	// SN
  "5 * ...-- 3 3",
//"5 * ..-.. É É",	// was: É	FIX: lowercase
  "5 C ..-.. UIswM",	// switch Motion UI activity
//"5 0 ..-.- 5 5",	// ..K  IK  UA  FT EÄ
  "5 * ..--. - -",	// ???
  "5 * ..--- 2 2",
  "5 * .-... & &",
  "5 * .-..- È È",	// È	FIX: lowercase
  "5 * .-.-. + +",
//"5 0 .-.-- 5 5",
//"5 0 .--.. 5 5",
  "5 * .--.- Å Å",	// Å	FIX: lowercase
  "5 * .---. - -",	// ???
  "5 * .---- 1 1",
  "5 * -.... 6 6",
  "5 * -...- = =",
  "5 * -..-. / /",
  "5 C -..-- UPPER",	// NW	_..__	UPPERCASE
  "5 C -.-.. LOWER",	// ND	-.-..	LOWERCASE
//"5 C -.-.- SEND",	// NK	-.-.-	SEND	obsolete, replaced by 'NEXT'
  "5 * -.--. ( (",
  "5 C -.--- DELLAST",	// NO -.---	DELETE LAST LETTER
  "5 * --... 7 7",
//"5 0 --..- 5 5",	// FREE M	MU	__.._ 3 M- COMMAND triggers	free :)  rhythm sample? tap  *HOW TO GET OUT?*
//"5 0 --.-. 5 5",	// FREE M	MR	--.-. 3 M- COMMAND triggers	free :)  rhythm replay
//"5 * --.-- Ñ Ñ",	// Ñ
  "5 * --.-- * *",	// my private code for '*'	// was: Ñ
  "5 0 ---.- OLED",	// OA	---.-	OLED	toggle oled display
  "5 * ---.. 8 8",
  "5 * ----. 9 9",
  "5 C ---.- DELWORD",	// MK	---.-	DELETE WORD
  "5 * ----- 0 0",


//"6 0 ...... TODO",	// FREE
//"6 0 ....._ TODO",	// FREE
//"6 0 ...._. TODO",	// FREE
//"6 0 ....__ TODO",	// FREE
//"6 0 ...-.. TODO",	// FREE		(maybe morse speed calibration?)
//"6 0 ...-.- TODO",	// FREE		(maybe morse speed calibration?)
//"6 0 ...__. TODO",	// FREE
//"6 C ...--- (was: OLED)",	// FREE	  was: OLED toggle oled display while playing
//"6 0 ...... TODO",	// FREE
//"6 0 ...... TODO",	// FREE

  "6 * ..--.. ? ?",
  "6 * .-..-. \" \"",
  "6 * .----. ' '",
  "6 * .-.-.- . .",
  "6 * .--.-. @ @",
  "6 * -....- _ _",	// check - _
  "6 * -.-.-. ; ;",
  "6 * -.-.-- ! !",
  "6 * --..-- , ,",
  "6 * ---... : :",

  "7 * ...-..- $ $",
//"7 0 ...--.. X X TODO:",		// beta ss	TODO: FIX:

  "8 C ........ MISTAKE",		// MISTAKE
};

#define MORSE_DEFINITIONS	(sizeof (morse_definitions_tab) / sizeof(const char *))


#define MORSE_DEFINITIONS_H
#endif
