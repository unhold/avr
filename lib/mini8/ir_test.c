// Quick & dirty test of the ir module.

#define F_CPU 8000000
#define IR_PRESCALER 1024
#define RC5_FREQUENCY 38000 // Frequency of the IR bursts 
#define RC5_ERROR 20

#define SIRC_ERROR 20

typedef unsigned char count_t;

#define _RC5_COUNT \
	((double)(F_CPU)*64/((double)(RC5_FREQUENCY)*(IR_PRESCALER)))
#define _RC5_ERROR ((double)(_RC5_COUNT)*(RC5_ERROR)/100)
#define _RC5_MAXCOUNT ((count_t)((_RC5_COUNT)+(_RC5_ERROR)+0.5))
#define _RC5_MINCOUNT ((count_t)((_RC5_COUNT)-(_RC5_ERROR)+0.5))
#define _RC5_COUNT075 ((count_t)((_RC5_COUNT)*0.75+0.5))
#define COUNT ((count_t)((_RC5_COUNT)+0.5))

#define _SIRC_COUNT (600e-6*(F_CPU)/(IR_PRESCALER))
	// 600 us in timer counts, the base time of SIRC (double)
#define _SIRC_MIN(_times_) \
	((count_t)((_SIRC_COUNT*(_times_)*(100-(SIRC_ERROR))/100)+0.5))
#define _SIRC_MAX(_times_) \
	((count_t)((_SIRC_COUNT*(_times_)*(100+(SIRC_ERROR))/100)+0.5))

#define RC5X_VALID(_data_) ((_data_)&1<<13)
	// Indicates if _data_ is valid (RC5x)

#define RC5_CLEAR(_data_) do{_data_=0;}while(0)
	// Clear rc5_data

#define RC5_TOGGLE(_data_) ((((_data_)&1<<11)?1:0))
	// Extract toggle bit from _data_

#define RC5_ADDRESS(_data_) ((uint8_t)((_data_)>>6)&0x1F)
	// Extract address from _data_

#define RC5_COMMAND(_data_) ((uint8_t)(_data_)&0x3F)
	// Extract command from _data_ (RC5)

#define RC5X_COMMAND(_data_) \
	(RC5_COMMAND(_data_)|(((_data_)&1<<12)?0:1<<6))
	// Extract command from _data_ (RC5x)
	
#include <stdio.h>

#include <inttypes.h>

uint16_t rc5_tmp;
uint16_t rc5_data = 0;
void rc5_handler(count_t newcount, unsigned char falling) {
	static count_t oldcount;
	count_t	count = newcount - oldcount; // Calculate differrence between counts
	if (count < _RC5_MINCOUNT/2 || count >_RC5_MAXCOUNT) {
		printf("reject ");
		// Pulse too short or too long
		rc5_tmp = 0;
	}
	if (rc5_tmp == 0 || count > _RC5_COUNT075) { // Start or long pulse
		oldcount = newcount; // Start counting from now on		rc5_tmp <<= 1;
		if (falling) {
			printf("falling ");
			rc5_tmp |= 1;
		} else {
			printf("rising ");
		}
		if (rc5_tmp&(1<<13)) { // 14 Bits received
			printf("rx14");
			rc5_data = rc5_tmp;
			rc5_tmp = 0;
		}
	}
}

int main() {
	count_t counts[] = {1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 1, 1, 1, 1, 1, 1};
	count_t counter = 234;
	int i = 0;

	printf("_RC5_COUNT = %f\n", _RC5_COUNT);
	printf("_RC5_ERROR = %f\n", _RC5_ERROR);
	printf("_RC5_MAXCOUNT = %d\n", _RC5_MAXCOUNT);
	printf("_RC5_MINCOUNT = %d\n", _RC5_MINCOUNT);
	printf(" _RC5_COUNT1_5 = %d\n\n",  _RC5_COUNT075);
	
	printf("_SIRC_COUNT = %f\n", _SIRC_COUNT);
	for (i = 0; i < 6; ++i) {
		printf("_SIRC_MIN(%d) = %d\n", i, _SIRC_MIN(i));	
		printf("_SIRC_MAX(%d) = %d\n", i, _SIRC_MAX(i));	
	}

	for (i = 0; i < sizeof(counts); ++i) {
		printf("%d ", counter);
		counter += COUNT*counts[i]/2;
		rc5_handler(counter, !(i&1));
		printf("\n");
	}
	printf("%#x ", rc5_data);
	if (RC5X_VALID(rc5_data))
		printf("valid ");
	else
		printf("invalid ");
	printf("toggle=%d address=%d command=%d\n", RC5_TOGGLE(rc5_data), RC5_ADDRESS(rc5_data), RC5X_COMMAND(rc5_data));

	return 0;
}

