////////////////////////////////////////////////////////////////////////////////
// file       : smallfont.c
// project    : mini8
// author     : Christian Unhold (christian.unhold@gmx.at)
// compiler   : WinAVR GCC
// hardware   : 5*5 dotmatrix (or bigger)
// description: character fonts for a 5*5 dotmatrix display, stored in Flash
//            : fonts have variable width, making them suitabe for scrolltext
////////////////////////////////////////////////////////////////////////////////

#include "smallfont.h"

#define LINE(_x_) 0b100##_x_
#define ZERO 0

static smallfont_at unknown = {
	LINE(11100),
	LINE(10100),
	LINE(11100),
	ZERO};

static smallfont_at space = {
	LINE(00000),
	LINE(00000),
	ZERO};

static smallfont_at exclamation_mark = {
	LINE(10111),
	ZERO};

static smallfont_at quote = {
	LINE(00011),
	ZERO};

static smallfont_at number_sign = {
	LINE(01010),
	LINE(11111),
	LINE(01010),
	LINE(11111),
	LINE(01010),
	ZERO};

static smallfont_at dollar_sign = {
	LINE(00110),
	LINE(01001),
	LINE(10001),
	LINE(10010),
	LINE(10001),
	LINE(01001),
	LINE(00110),
	ZERO};

static smallfont_at percent = {
	LINE(10010),
	LINE(01000),
	LINE(00100),
	LINE(10010),
	ZERO};

static smallfont_at plus = {
	LINE(00100),
	LINE(01110),
	LINE(00100),
	ZERO};

static smallfont_at left_parenthesis = {
	LINE(01110),
	LINE(10001),
	ZERO};

static smallfont_at right_parenthesis = {
	LINE(10001),
	LINE(01110),
	ZERO};

static smallfont_at asterisk = {
	LINE(01010),
	LINE(00100),
	LINE(01010),
	ZERO};

static smallfont_at comma = {
	LINE(11000),
	ZERO};

static smallfont_at minus = {
	LINE(00100),
	LINE(00100),
	LINE(00100),
	ZERO};

static smallfont_at dot = {
	LINE(10000),
	ZERO};

static smallfont_at slash = {
	LINE(10000),
	LINE(01000),
	LINE(00100),
	LINE(00010),
	ZERO};

static smallfont_at zero = {
	LINE(01110),
	LINE(10001),
	LINE(01110),
	ZERO};

static smallfont_at one = {
	LINE(00010),
	LINE(11111),
	ZERO};

static smallfont_at two = {
	LINE(10010),
	LINE(11001),
	LINE(10101),
	LINE(10010),
	ZERO};

static smallfont_at three = {
	LINE(01010),
	LINE(10001),
	LINE(10101),
	LINE(01010),
	ZERO};

static smallfont_at four = {
	LINE(00100),
	LINE(00110),
	LINE(11101),
	ZERO};

static smallfont_at five = {
	LINE(10111),
	LINE(10101),
	LINE(01001),
	ZERO};

static smallfont_at six = {
	LINE(01110),
	LINE(10101),
	LINE(10101),
	LINE(01001),
	ZERO};

static smallfont_at seven = {
	LINE(00001),
	LINE(11101),
	LINE(00011),
	LINE(00001),
	ZERO};

static smallfont_at eight = {
	LINE(01010),
	LINE(10101),
	LINE(10101),
	LINE(01010),
	ZERO};

static smallfont_at nine = {
	LINE(10010),
	LINE(10101),
	LINE(10101),
	LINE(01110),
	ZERO};

static smallfont_at colon = {
	LINE(01010),
	ZERO};

static smallfont_at semi_colon = {
	LINE(11010),
	ZERO};

static smallfont_at less_than = {
	LINE(00100),
	LINE(01010),
	LINE(10001),
	ZERO};

static smallfont_at equal_sign = {
	LINE(01010),
	LINE(01010),
	LINE(01010),
	ZERO};

static smallfont_at greater_sign = {
	LINE(10001),
	LINE(01010),
	LINE(00100),
	ZERO};

static smallfont_at question_mark = {
	LINE(00001),
	LINE(10101),
	LINE(00010),
	ZERO};

static smallfont_at a = {
	LINE(11110),
	LINE(01001),
	LINE(01001),
	LINE(11110),
	ZERO};

static smallfont_at b = {
	LINE(11111),
	LINE(10101),
	LINE(10101),
	LINE(01010),
	ZERO};

static smallfont_at c = {
	LINE(01110),
	LINE(10001),
	LINE(10001),
	ZERO};

static smallfont_at d = {
	LINE(11111),
	LINE(10001),
	LINE(10001),
	LINE(01110),
	ZERO};

static smallfont_at e = {
	LINE(11111),
	LINE(10101),
	LINE(10101),
	ZERO};

static smallfont_at f = {
	LINE(11111),
	LINE(00101),
	LINE(00101),
	ZERO};

static smallfont_at g = {
	LINE(01110),
	LINE(10001),
	LINE(10101),
	LINE(01101),
	ZERO};

static smallfont_at h = {
	LINE(11111),
	LINE(00100),
	LINE(00100),
	LINE(11111),
	ZERO};

static smallfont_at i = {
	LINE(11111),
	ZERO};

static smallfont_at j = {
	LINE(01001),
	LINE(10001),
	LINE(10001),
	LINE(01111),
	ZERO};

static smallfont_at k = {
	LINE(11111),
	LINE(00100),
	LINE(01010),
	LINE(10001),
	ZERO};
	
static smallfont_at l = {
	LINE(11111),
	LINE(10000),
	LINE(10000),
	ZERO};
	
static smallfont_at m = {
	LINE(11111),
	LINE(00010),
	LINE(00100),
	LINE(00010),
	LINE(11111),
	ZERO};
	
static smallfont_at n = {
	LINE(11111),
	LINE(00010),
	LINE(00100),
	LINE(11111),
	ZERO};
	
static smallfont_at o = {
	LINE(01110),
	LINE(10001),
	LINE(10001),
	LINE(01110),
	ZERO};
	
static smallfont_at p = {
	LINE(11111),
	LINE(00101),
	LINE(00101),
	LINE(00010),
	ZERO};
	
static smallfont_at q = {
	LINE(01110),
	LINE(10001),
	LINE(10101),
	LINE(01110),
	LINE(10000),
	ZERO};
	
static smallfont_at r = {
	LINE(11111),
	LINE(00101),
	LINE(01101),
	LINE(10010),
	ZERO};
	
static smallfont_at s = {
	LINE(10010),
	LINE(10101),
	LINE(10101),
	LINE(01001),
	ZERO};
	
static smallfont_at t = {
	LINE(00001),
	LINE(11111),
	LINE(00001),
	ZERO};
	
static smallfont_at u = {
	LINE(01111),
	LINE(10000),
	LINE(10000),
	LINE(01111),
	ZERO};
	
static smallfont_at v = {
	LINE(01111),
	LINE(10000),
	LINE(01111),
	ZERO};
	
static smallfont_at w = {
	LINE(01111),
	LINE(10000),
	LINE(01100),
	LINE(10000),
	LINE(01111),
	ZERO};
	
static smallfont_at x = {
	LINE(11011),
	LINE(00100),
	LINE(11011),
	ZERO};
	
static smallfont_at y = {
	LINE(00011),
	LINE(11100),
	LINE(00011),
	ZERO};
	
static smallfont_at z = {
	LINE(11001),
	LINE(10101),
	LINE(10011),
	ZERO};
	
static smallfont_at left_bracket = {
	LINE(11111),
	LINE(10001),
	ZERO};
	
static smallfont_at back_slash = {
	LINE(00010),
	LINE(00100),
	LINE(01000),
	LINE(10000),
	ZERO};
	
static smallfont_at right_bracket = {
	LINE(10001),
	LINE(11111),
	ZERO};

static smallfont_at circumflex = {
	LINE(00010),
	LINE(00001),
	LINE(00010),
	ZERO};

static smallfont_at underscore = {
	LINE(10000),
	LINE(10000),
	LINE(10000),
	ZERO};

static smallfont_at vertical_bar = {
	LINE(11111),
	ZERO};

static smallfont_at tilde = {
	LINE(00100),
	LINE(01000),
	LINE(00100),
	LINE(00010),
	LINE(00100),
	ZERO};

static smallfont_pt PROGMEM table[] = {
	space,                  // 32
	exclamation_mark,       // 33
	quote,                  // 34
	number_sign, // #       // 35
	dollar_sign,            // 36
	percent,                // 37
	plus, // &              // 38
	quote,                  // 39
	left_parenthesis, // (  // 40
	right_parenthesis, // ) // 41
	asterisk, // *          // 42
	plus,                   // 43
	comma,                  // 44
	minus,                  // 45
	dot,                    // 46
	slash,                  // 47

	zero,                   // 48
	one,                    // 49
	two,                    // 50
	three,                  // 51
	four,                   // 52
	five,                   // 53
	six,                    // 54
	seven,                  // 55
	eight,                  // 56
	nine,                   // 57

	colon,                  // 58
	semi_colon,             // 59
	less_than, // <         // 60
	equal_sign,             // 61
	greater_sign,           // 62
	question_mark,          // 63
	unknown, // @           // 64

	a,                      // 65 // upper alpha
	b,                      // 66
	c,                      // 67
	d,                      // 68
	e,                      // 69
	f,                      // 70
	g,                      // 71
	h,                      // 72
	i,                      // 73
	j,                      // 74
	k,                      // 75
	l,                      // 76
	m,                      // 77
	n,                      // 78
	o,                      // 79
	p,                      // 80
	q,                      // 81
	r,                      // 82
	s,                      // 83
	t,                      // 84
	u,                      // 85
	v,                      // 86
	w,                      // 87
	x,                      // 88
	y,                      // 89
	z                       // 90
#if SMALLFONT_PUNCT2
	,
	left_bracket, // [      // 91
	back_slash,             // 92
	right_bracket, // ]     // 93
	circumflex, // ^        // 94
	underscore,             // 95
	quote,                  // 96
/* convert lower to upper
	a,                      // 97 // lower alpha
	b,                      // 98
	c,                      // 99
	d,                      // 100
	e,                      // 101
	f,                      // 102
	g,                      // 103
	h,                      // 104
	i,                      // 105
	j,                      // 106
	k,                      // 107
	l,                      // 108
	m,                      // 109
	n,                      // 110
	o,                      // 111
	p,                      // 112
	q,                      // 113
	r,                      // 114
	s,                      // 115
	t,                      // 116
	u,                      // 117
	v,                      // 118
	w,                      // 119
	x,                      // 120
	y,                      // 121
	z,                      // 122
*/
	left_bracket, // {      // 123
	vertical_bar, // |      // 124
	right_bracket, // }     // 125
	tilde //              // 126
#endif // SMALLFONT_PUNCT2
	,
};

#define FIRST ' '
#if SMALLFONT_PUNCT2
#	define LAST '~'
#else // SMALLFONT_PUNCT2
#	define LAST 'z'
#endif // SMALLFONT_PUNCT2

smallfont_pt smallfont_lookup(char c) {
	if (c < FIRST || c > LAST) {
		return unknown;
	}
	if (c >= 'a' && c <= 'z') {
		c -= 'a'-'A'; // convert lower to upper
#if SMALLFONT_PUNCT2
	} else if (c >= '{') {
		// compensate hole in table
		c -= 'Z' - 'A' + 1;
#endif // SMALLFONT_PUNCT2
	}
	return (smallfont_pt)pgm_read_word(table+c-' ');
}

