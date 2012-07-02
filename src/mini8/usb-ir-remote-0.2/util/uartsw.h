/*
 * Copyright 2010 Jorgen Birkler
 * jorgen.birkler)a(gmail.com
 *
 *
 * software uart using timer1
 */

///see http://www.rn-wissen.de/index.php/Software-UART_mit_avr-gcc


void uartsw1_putc (const char c);
int uartsw1_getc_nowait(void);
int uartsw1_getc_wait(void);


void uartsw2_putc (const char c);
int uartsw2_getc_nowait(void);
int uartsw2_getc_wait(void);


void uartsw_init(void);
