/**
 * Bootloader um dem Mikrocontroller Bootloader von Peter Dannegger anzusteuern
 * Teile des Codes sind vom original Booloader von Peter Dannegger (danni@alice-dsl.net)
 *
 * @author Bernhard Michler, based on linux source of Andreas Butti
 */


/// Includes
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include <signal.h>
#include <sys/times.h>
#include <sys/ioctl.h>

#include "com.h"
#include "protocol.h"


/**************************************************************/
/*                          CONSTANTS                         */
/**************************************************************/
#define AVR_PROGRAM     0x01
#define AVR_VERIFY      0x02
#define AVR_TERMINAL    0x04
#define AVR_CLEAN       0x08

#define AUX     1
#define CON     2
#define TRUE    1
#define FALSE   0

#define ACK     0x06
#define NACK    0x15
#define ESC     0x1b
#define CTRLC   0x03
#define CTRLP   0x10
#define CTRLE   0x05
#define CTRLF   0x06
#define CTRLV   0x16

// Definitions
#define TIMEOUT   3   // 0.3s
#define TIMEOUTP  40  // 4s


#define ELAPSED_TIME(a) {                   \
    struct tms    time;                     \
    clock_t       stop;                     \
    stop = times (&time);                   \
    (a) = (double) (stop - start) / ticks;  \
}

// enum for autoreset
typedef enum {
    NO_AUTORESET  = 0,      // don't reset
    HIGH_AUTORESET,         // reset is active high
    LOW_AUTORESET           // reset is active low
} autoreset_t;


/**************************************************************/
/*                          GLOBALS                           */
/**************************************************************/
static struct termios   curr_term, old_term;
static FILE             *fp_stdio = NULL; /* filepointer to normal terminal */
static int              running = TRUE;
static int              esc_seq = 0;

static int              bsize = 16;

    // pointer to password...
    // must contain one of
    // 0x0A - LF,  0x0B - VT,  0x0D - CR,  0x0F - SI
    // 0x21 - '!', 0x43 - 'C', 0x61 - 'a', 0x85, 0x87
    // 0xC3 - 'A~',0xE1 - 'a´' - ISO8859-1
static char             *password = "Peda";
static char             *device = "/dev/ttyS0";
static int              baud = 9600;

/* variables for stopwatch */
static clock_t  start  = 0;
static double   ticks = 1;

// Filename of the HEX File
static const char * hexfile = NULL;


typedef struct bootInfo
{
    long    revision;
    long    signature;
    long    buffsize;
    long    flashsize;
    int     crc_on;
    int     blocksize;
} bootInfo_t;


typedef struct
{
    unsigned long   id;
    const char      *name;
} avrdev_t;
avrdev_t avr_dev[] = {
    { 0x01e9007,    "ATtiny13" },
    { 0x01e910a,    "ATtiny2313" },
    { 0x01e9205,    "ATmega48" },
    { 0x01e9206,    "ATtiny45" },
    { 0x01e9207,    "ATtiny44" },
    { 0x01e9208,    "ATtiny461" },
    { 0x01e9306,    "ATmega8515" },
    { 0x01e9307,    "ATmega8" },
    { 0x01e9308,    "ATmega8535" },
    { 0x01e930a,    "ATmega88" },
    { 0x01e930b,    "ATtiny85" },
    { 0x01e930c,    "ATtiny84" },
    { 0x01e930d,    "ATtiny861" },
    { 0x01e930f,    "ATmega88P" },
    { 0x01e9403,    "ATmega16" },
    { 0x01e9404,    "ATmega162" },
    { 0x01e9406,    "ATmega168" },
    { 0x01e9501,    "ATmega323" },
    { 0x01e9502,    "ATmega32" },
    { 0x01e950f,    "ATmega328" },
    { 0x01e9609,    "ATmega644" },
    { 0x01e9802,    "ATmega2561" }
};
    


/*****************************************************************************
 *
 *      Signal handler - reset terminal
 *
 ****************************************************************************/
void sig_handler(int signal)
{
    running = FALSE;
#if 0
    if (fp_stdio != NULL)
    {
        printf("\nSignal %d (%d) resetting terminal !\n", signal, getpid());
        tcsetattr (fileno (fp_stdio), TCSAFLUSH, &old_term);
    }
#endif
}


/*****************************************************************************
 *
 *      Set timeout on tty input to new value, returns old timeout
 *
 ****************************************************************************/
static int set_tty_timeout (int    fd,
                            int    timeout)
{
    struct termios      terminal;
    int                 old_timeout;

    /* get current settings */
    tcgetattr (fd, &terminal);

    /* get old timeout */
    old_timeout = terminal.c_cc[VTIME];

    /* set timeout in 10th of seconds */
    terminal.c_cc[VTIME] = timeout;

    /* set new status */
    tcsetattr (fd, TCSANOW, &terminal);

    return (old_timeout);
}


/**
 * reads hex data from string
 */
int sscanhex (char          *str,
              unsigned int  *hexout,
              int           n)
{
    unsigned int hex = 0, x = 0;

    for(; n; n--)
    {
        x = *str;
        if(x >= 'a')
        {
            x += 10 - 'a';
        }
        else if(x >= 'A')
        {
            x += 10 - 'A';
        }
        else
        {
            x -= '0';
        }

        if(x >= 16)
        {
            break;
        }

        hex = hex * 16 + x;
        str++;
    }

    *hexout = hex;
    return n; // 0 if all digits
}


/**
 * Reads the hex file
 *
 * @return 1 to 255 number of bytes, -1 file end, -2 error or no HEX File
 */
int readhex (FILE           *fp,
             unsigned long  *addr,
             unsigned char  *data)
{
    char hexline[524]; // intel hex: max 255 byte
    char *hp = hexline;
    unsigned int byte;
    int i;
    unsigned int num;
    unsigned int low_addr;

    if(fgets( hexline, 524, fp ) == NULL)
    {
        return -1; // end of file
    }

    if(*hp++ != ':')
    {
        return -2; // no hex record
    }

    if(sscanhex(hp, &num, 2))
    {
        return -2; // no hex number
    }

    hp += 2;

    if(sscanhex(hp, &low_addr, 4))
    {
        return -2;
    }

    *addr &= 0xF0000L;
    *addr += low_addr;
    hp += 4;

    if(sscanhex( hp, &byte, 2))
    {
        return -2;
    }

    if(byte == 2)
    {
        hp += 2;
        if(sscanhex(hp, &low_addr, 4))
        {
            return -2;
        }
        *addr = low_addr * 16L;
        return 0; // segment record
    }

    if(byte == 1)
    {
        return 0; // end record
    }

    if(byte != 0)
    {
        return -2; // error, unknown record
    }

    for(i = num; i--;)
    {
        hp += 2;
        if(sscanhex(hp, &byte, 2))
        {
            return -2;
        }
        *data++ = byte;
    }
    return num;
}

/**
 * Read a hexfile
 */
char * read_hexfile(const char * filename, unsigned long * lastaddr)
{
    char    *data;
    FILE    *fp;
    int     len;
    int     x;
    unsigned char line[256];
    unsigned long addr = 0;

    data = malloc(MAXFLASH);
    if (data == NULL)
    {
        printf("Memory allocation error, could not get %d bytes for flash-buffer!\n",
               MAXFLASH);
        return NULL;
    }

    *lastaddr = 0;
    memset (data, 0xff, MAXFLASH);

    if(NULL == (fp = fopen(filename, "r")))
    {
        printf("File \"%s\" open failed: %s!\n\n", filename, strerror(errno));
        free(data);
        return NULL;
    }

    printf("Reading       : %s... ", filename);


    // reading file to "data"
    while((len = readhex(fp, &addr, line)) >= 0)
    {
        if(len)
        {
            if( addr + len > MAXFLASH )
            {
                fclose(fp);
                free(data);
                printf("\n  Hex-file too large for target!\n");
                return NULL;
            }
            for(x = 0; x < len; x++)
            {
                data[x + addr] = line[x];
            }

            addr += len;

            if(*lastaddr < (addr-1))
            {
                *lastaddr = addr-1;
            }
            addr++;
        }
    }

    fclose(fp);

    printf("File read.\n");
    return data;
}


/**
 * Reads a value from bootloader
 *
 * @return value; -2 on error; exit on timeout
 */
long readval(int fd)
{
    int i;
    int j = 257;
    long val = 0;

    while(1)
    {
        i = com_getc(fd, TIMEOUT);
        if(i == -1)
        {
            printf("readval: ...Device does not answer!\n");
            return -3;
        }

        switch(j)
        {
            case 1:
                if(i == SUCCESS)
                {
                    return val;
                }
                break;

            case 2:
            case 3:
            case 4:
                val = val * 256 + i;
                j--;
                break;

            case 256:
                j = i;
                break;

            case 257:
                if(i == FAIL) {
                    return -2;
                }
                else if(i == ANSWER) {
                    j = 256;
                }
                break;

            default:
                printf("\nError: readval, i = %i, j = %i, val = %li\n", i, j, val);
                return -2;
        }
    }
    return -1;
}

/**
 * Print percentage line
 */
void print_perc_bar (char          *text,
                     unsigned long full_val,
                     unsigned long cur_val)
{
    int         i;
    int         cur_perc;
    int         cur100p;
    int         txtlen = 8;    // length of the add. text 2 * " [" "100%"
    unsigned short columns = 80;
    struct winsize win_size;

    if (text)
        txtlen += strlen (text);

    if (ioctl (STDIN_FILENO, TIOCGWINSZ, &win_size) >= 0)
    {
        // number of columns in terminal
        columns = win_size.ws_col;
    }
    cur100p  = columns - txtlen;
    cur_perc = (cur_val * cur100p) / full_val;

    printf ("%s [", text ? text : "");

    for (i = 0; i < cur_perc; i++) printf ("#");
    for (     ; i < cur100p;  i++) printf (" ");

    printf ("] %3d%%\r", (int)((cur_val * 100) / full_val));

    fflush(stdout);
}


/**
 * Verify the controller
 */
int verifyflash (int           fd,
                 char        * data,
                 unsigned long lastaddr,
                 bootInfo_t  * bInfo)
{
    struct tms  timestruct;
    clock_t     start_time;       //time
    clock_t     end_time;         //time
    float       seconds;

    unsigned char d1;
    unsigned long addr = 0;

    start_time = times (&timestruct);

    // Sending commands to MC
    sendcommand(fd, VERIFY);

    if(com_getc(fd, TIMEOUT) == BADCOMMAND)
    {
        printf("Verify not available\n");
        return 0;
    }
    printf( "Verify        : 0x00000 - 0x%05lX\n", lastaddr);

    // Sending data to MC
    do
    {
        if ((addr % 16) == 0)
            print_perc_bar ("Verifying", lastaddr, addr);

        d1 = data[addr];

        if ((d1 == ESCAPE) || (d1 == 0x13))
        {
            com_putc(fd, ESCAPE);
            d1 += ESC_SHIFT;
        }
        if (addr % bInfo->blocksize)
            com_putc_fast (fd, d1);
        else
            com_putc (fd, d1);

    } while (addr++ < lastaddr);


    print_perc_bar ("Verifying", 100, 100);

    end_time = times (&timestruct);
    seconds  = (float)(end_time-start_time)/sysconf(_SC_CLK_TCK);

    printf("\nElapsed time  : %3.2f seconds, %.0f Bytes/sec.\n",
           seconds,
           (float)lastaddr / seconds);

    com_putc(fd, ESCAPE);
    com_putc(fd, ESC_SHIFT); // A5,80 = End

    if (com_getc(fd, TIMEOUTP) == SUCCESS)
        return 1;

    return 0;
}


/**
 * Flashes the controller
 */
int programflash (int           fd,
                  char        * data,
                  unsigned long lastaddr,
                  bootInfo_t *  bInfo)
{
    struct tms  timestruct;
    clock_t start_time;       //time
    clock_t end_time;         //time
    float   seconds;

    int    i;
    unsigned char d1;
    unsigned long addr = 0;

    start_time = times (&timestruct);

    // Sending commands to MC
    printf("Programming   : 0x00000 - 0x%05lX\n", lastaddr);
    sendcommand(fd, PROGRAM);

    // Sending data to MC
    i = bInfo->buffsize;

    do
    {
        if ((addr % 16) == 0)
            print_perc_bar ("Writing", lastaddr, addr);

        d1 = data[addr];

        if ((d1 == ESCAPE) || (d1 == 0x13))
        {
            com_putc(fd, ESCAPE);
            d1 += ESC_SHIFT;
        }
        if (i % bInfo->blocksize)
            com_putc_fast (fd, d1);
        else
            com_putc (fd, d1);

        if (--i == 0)
        {
            if (com_getc (fd, TIMEOUTP) != CONTINUE)
            {
                printf("\n ---------- Failed! ----------\n");
                free(data);
                return 0;
            }

            // set nr of bytes with next block
            i = bInfo->buffsize;
        }
    } while (addr++ < lastaddr);

    print_perc_bar ("Writing", 100, 100);

    end_time = times (&timestruct);
    seconds  = (float)(end_time-start_time)/sysconf(_SC_CLK_TCK);

    printf("\nElapsed time  : %3.2f seconds, %.0f Bytes/sec.\n",
           seconds,
           (float)lastaddr / seconds);

    com_putc(fd, ESCAPE);
    com_putc(fd, ESC_SHIFT); // A5,80 = End

    if (com_getc(fd, TIMEOUTP) == SUCCESS)
        return 1;

    return 0;
}



/**
 * prints usage
 */
void usage(char *name)
{
    printf("%s [-d /dev/ttyS0] [-b 9600] -[v|p] file.hex\n"
           "-d /dev/ttynn   Device\n"
           "-b nn           Baudrate\n"
           "-t nn           TxD Blocksize (i.e. number of bytes written in one block)\n"
           "-w nn           do not use tcdrain, wait nn times byte transmission time instead\n"
           "-r              toggle DTR to reset device:\n"
           "                set DTR low for reset, wait, set DTR high\n"
           "-R              toggle DTR to reset device:\n"
           "                set DTR high for reset, wait, set DTR low\n"
           "-v              Verify\n"
           "-p              Program\n"
           "-e              Erase, use together with -p to erase controller,\n"
           "                with -v to check if it is erased\n"
           "-P pwd          Password\n"
           "-T              enter terminal mode\n"
           "Author: Bernhard Michler (based on code from Andreas Butti)\n", name);

    exit(1);
}

/**
 * Resets the connected device to put it into bootloader mode
 */
void prog_reset(int         fd,
                autoreset_t res)
{
    switch (res) 
    {
        case LOW_AUTORESET:
            com_set_dtr(fd, 0);
            usleep(10000);
            com_set_dtr(fd, 1);
            break;

        case HIGH_AUTORESET:
            com_set_dtr(fd, 1);
            usleep(10000);
            com_set_dtr(fd, 0);
            break;

        default:
            break;
    }
}


/**
 * Try to connect a device
 */
int connect_device ( int fd,
                     const char *password )
{
    const char * ANIM_CHARS = "-\\|/";

    int state = 0;
    int in = 0;

    char passtring[32];

    sprintf (passtring, "%s%c", password, 0xff);

    printf("Waiting for device...  ");

    while (running)
    {
        const char *s = passtring; //password;

        usleep (25000);     // just to slow animation...
        printf("\b%c", ANIM_CHARS[state++ & 3]);
        fflush(stdout);

        do 
        {
            if (*s)
                com_putc(fd, *s);
            else
                com_putc(fd, 0x0ff);

            in = com_getc(fd, 0);

            if (in == CONNECT)
            {
                printf ("\bconnected");

                // clear buffer from echo...
                while (com_getc(fd, TIMEOUT) != -1);

                sendcommand( fd, COMMAND );

                while (1)
                {
                    switch(com_getc(fd, TIMEOUT))
                    {
                        case COMMAND:
                            com_localecho();
                            printf (" (one wire)");
                            break;
                        case SUCCESS:
                        case -1:
                            printf ("!\n");
                            return 1;
                    }
                }
            }
        } while (*s++);
    }
    printf ("\nTerminated by user.\n");

    return 0;
}


/**
 * Checking CRC Support
 *
 * @return 2 if no crc support, 0 if crc supported, 1 fail, exit on timeout
 */
int check_crc(int fd)
{
    int i;
    unsigned int crc1;

    sendcommand(fd, CHECK_CRC);
    crc1 = crc;
    com_putc(fd, crc1);
    com_putc(fd, crc1 >> 8);

    i = com_getc(fd, TIMEOUT);
    switch (i)
    {
        case SUCCESS:
            return 0;
        case BADCOMMAND:
            return 2;
        case FAIL:
            return 1;
        case -1:
            printf("check_crc: ...Device does not answer!\n\n");
            // FALLTHROUGH
        default:
            return i;
    }
}

/**
 * prints the device signature
 *
 * @return true on success; exit on error
 */
int read_info (int fd, 
               bootInfo_t *bInfo)
{
    long i, j;
    char s[256];
    FILE *fp;

    bInfo->crc_on = check_crc(fd);
    if (bInfo->crc_on < 0)
        return (0);

    sendcommand(fd, REVISION);

    i = readval(fd);
    if(i < 0)
    {
        printf("Bootloader Version unknown (Fail)\n");
        bInfo->revision = -1;
    }
    else
    {
        printf("Bootloader    : V%lX.%lX\n", i>>8, i&0xFF);
        bInfo->revision = i;
    }

    sendcommand(fd, SIGNATURE);

    i = readval(fd);
    if (i < 0)
    {
        printf("Reading device SIGNATURE failed!\n\n");
        return (0);
    }
    bInfo->signature = i;

    if((fp = fopen("devices.txt", "r")) != NULL)
    {
        while(fgets(s, 256, fp))
        {
            if(sscanf(s, "%lX : %s", &j, s) == 2)
            { // valid entry
                if(i == j)
                {
                    break;
                }
            }
            *s = 0;
        }
        fclose(fp);
    }
    else
    {
        // search locally...
        for (j = 0; j < (sizeof (avr_dev) / sizeof (avrdev_t)); j++)
        {
            if (i == avr_dev[j].id)
            {
                strcpy (s, avr_dev[j].name);
                break;
            }
        }
        if (j == (sizeof (avr_dev) / sizeof (avrdev_t)))
        {
            sscanf ("(?)" , "%s", s);
            printf("File \"devices.txt\" not found!\n");
        }
    }
    printf("Target        : %06lX %s\n", i, s);

    sendcommand(fd, BUFFSIZE);

    i = readval(fd);
    if (i < 0)
    {
        printf("Reading BUFFSIZE failed!\n\n");
        return (0);
    }
    bInfo->buffsize = i;

    printf("Buffer        : %ld Byte\n", i );

    sendcommand(fd, USERFLASH);

    i = readval(fd);
    if (i < 0)
    {
        printf("Reading FLASHSIZE failed!\n\n");
        return (0);
    }
    if( i > MAXFLASH)
    {
        printf("Device and flashsize do not match!\n");
        return (0);
    }
    bInfo->flashsize = i;

    printf("Size available: %ld Byte\n", i );

    if(bInfo->crc_on != 2)
    {
        bInfo->crc_on = check_crc(fd);
        switch(bInfo->crc_on)
        {
            case 2:
                printf("No CRC support.\n");
                break;
            case 0:
                printf("CRC enabled and OK.\n");
                break;
            case 3:
                printf("CRC check failed!\n");
                break;
            default:
                printf("Checking CRC Error (%i)!\n", bInfo->crc_on);
                break;
        }
    }
    else
    {
        printf("No CRC support.\n\n");
    }

    return 1;
}//int read_info()



int prog_verify (int            fd,
                 int            mode,
                 int            baud,
                 int            block_size,
                 const char     *password,
                 const char     *device,
                 const char     *hexfile)
{
    char        *data = NULL;
    bootInfo_t  bootinfo;

    // last address in hexfile
    unsigned long last_addr = 0;

    // init bootinfo
    memset (&bootinfo, 0, sizeof (bootinfo));

    // set to maximun, is later in read_info corrected to the
    // size available in the controller...
    bootinfo.flashsize = MAXFLASH;
    bootinfo.blocksize = block_size;

    printf ("Now: ");
    if (mode & AVR_CLEAN)
        printf ("erase, ");
    if (mode & AVR_PROGRAM)
        printf ("program, ");
    if (mode & AVR_VERIFY)
        printf ("verify, ");
    printf ("\b\b device.\n");

    printf("Port          : %s\n", device);
    printf("Baudrate      : %d\n", baud);

    if (mode & AVR_CLEAN)
    {
        data = malloc(MAXFLASH);

        if (data == NULL)
            printf("Memory allocation error, could not get %d bytes for flash-buffer!\n",
                   MAXFLASH);
        else
            memset (data, 0xff, MAXFLASH);

        last_addr = MAXFLASH - 1;
    }
    else
    {
        printf("File          : %s\n", hexfile);

        // read the file
        data = read_hexfile (hexfile, &last_addr);

        printf("Size          : %ld Bytes\n", last_addr + 1);
    }

    if (data == NULL)
    {
        printf ("ERROR: no buffer allocated and filled, exiting!\n");
        return (-1);
    }

    printf("-------------------------------------------------\n");

    // now start with target...
    if (connect_device (fd, password))
    {
        if (!read_info (fd, &bootinfo))
        {
            return (-3);
        }

        if (mode & AVR_CLEAN)
        {
            last_addr = bootinfo.flashsize - 1;
        }
        else
        // now check if program fits into flash
        if (last_addr >= bootinfo.flashsize)
        {
            printf ("ERROR: Hex-file too large for target!\n"
                    "       (needs flash-size of %ld bytes, we have %ld bytes)\n",
                    last_addr + 1, bootinfo.flashsize);
            return (-2);
        }

        if (mode & AVR_PROGRAM)
        {
            if (programflash (fd, data, last_addr, &bootinfo))
            {
                if ((bootinfo.crc_on != 2) && (check_crc(fd) != 0))
                    printf("\n ---------- Programming failed (wrong CRC)! ----------\n\n");
                else if (mode & AVR_CLEAN)
                    printf("\n ++++++++++ Device successfully erased! ++++++++++\n\n");
                else
                    printf("\n ++++++++++ Device successfully programmed! ++++++++++\n\n");
            }
            else
            {
                printf("\n ---------- Programming failed! ----------\n\n");
                return (-5);
            }
        }
        if (mode & AVR_VERIFY)
        {
            if (verifyflash (fd, data, last_addr, &bootinfo))
            {
                if ((bootinfo.crc_on != 2) && (check_crc(fd) != 0))
                    printf("\n ---------- Verification failed (wrong CRC)! ----------\n\n");
                else
                    printf("\n ++++++++++ Device successfully verified! ++++++++++\n\n");
            }
            else
                printf("\n ---------- Verification failed! ----------\n\n");
        }

        if (!(mode & AVR_CLEAN))
            printf("...starting application\n\n");

        sendcommand(fd, START);         //start application
        sendcommand(fd, START);
    }
    free (data);

    return 0;
}


/*****************************************************************************
 *
 *      Handle keyboard input
 *
 ****************************************************************************/
static int handle_keyboard (FILE  *input,
                            int   output,
                            int   autoreset)
{
    static char fname[1024+1] = "";
    int         char_in = EOF;
    int         desc_in = fileno (input);

    if (desc_in < 0)
    {
        fprintf (stderr, "Error: Invalid stream for input (keyboard?) (errno %d: %s)!\n",
                 errno, strerror (errno));
        return (FALSE);
    }

    if ((fname[0] == '\0') &&
        (hexfile  != NULL))
        strcpy (fname, hexfile);


    while (EOF != (char_in = getc (input)))
    {
        switch (char_in)
        {
            case '\r':
                /* ignore... */
                break;

            case '\n':
                com_putc (output, '\r');
                break;

            case CTRLF:
                tcsetattr (desc_in, TCSAFLUSH, &old_term);
                printf ("\nEnter Filename: ");
                scanf ("%s", fname);
                printf ("\nstart programming with CTRL P, verifying with CTRL V...\n");
                tcsetattr (desc_in, TCSAFLUSH, &curr_term);
                break;

            case CTRLP:
                tcsetattr (desc_in, TCSAFLUSH, &old_term);
                if (autoreset != NO_AUTORESET)
                {
                    printf("\n== PROGRAM:  Resetting Target Device ==========\n");
                    prog_reset(output, autoreset);
                }
                else
                {
                    printf("\n== PROGRAM:  Reset Target Device ==============\n");
                }
                prog_verify (output, AVR_PROGRAM, 
                             baud, bsize, password, device, fname);
                tcsetattr (desc_in, TCSAFLUSH, &curr_term);
                break;

            case CTRLV:
                tcsetattr (desc_in, TCSAFLUSH, &old_term);
                if (autoreset != NO_AUTORESET)
                {
                    printf("\n== VERIFY:   Resetting Target Device ==========\n");
                    prog_reset(output, autoreset);
                }
                else
                {
                    printf("\n== VERIFY:   Reset Target Device ==============\n");
                }
                prog_verify (output, AVR_VERIFY, 
                             baud, bsize, password, device, fname);
                tcsetattr (desc_in, TCSAFLUSH, &curr_term);
                break;

            case CTRLE:
                tcsetattr (desc_in, TCSAFLUSH, &old_term);
                printf("\n== ERASE:   Reset Target Device ==============\n");
                prog_verify (output, AVR_PROGRAM | AVR_CLEAN, 
                             baud, bsize, password, device, fname);
                tcsetattr (desc_in, TCSAFLUSH, &curr_term);
                break;

            case EOF:
                break;

            case CTRLC:
                return (FALSE);
                break;

            default:
                com_putc (output, (char) char_in);
                break;
        }
    }

    return (TRUE);
}


/*****************************************************************************
 *
 *      Handle V24 input
 *
 ****************************************************************************/
static void handle_input (int          input,
                          FILE         *output)
{
    static char         readbuf[1024+1];
    int                 i;
    int                 bytes_read;

    /* handle V24 input here */
    do
    {
        bytes_read = com_read (input, readbuf, sizeof (readbuf) - 1);

        /* replace possible CR/LF with LF only */
        for (i = 0; i < bytes_read; i++)
        {
            if (esc_seq)
            {
                esc_seq = 0;
            }
            else
            {
                switch (readbuf[i])
                {
                    case '\r':
                        break;
                    case 27:    /* Escape, ignore next as well */
                        esc_seq++;
                        break;
                    default:
                        putc (readbuf[i], output);
                }
            }
        }
    } while (bytes_read > 0);
    fflush (output);
}

/*****************************************************************************
 *
 *      Program loop
 *
 ****************************************************************************/
static void do_v24 (int iFd,
                    int autoreset)
{
    int                 old_timeout;
    int                 stdio;
    int                 ok;
    int                 max_select;
    struct timeval      timeout;
    fd_set              fdset;
    int                 ret_val;

    /* "open" terminal -> get filedescriptor to it */
    fp_stdio = fopen (ctermid (NULL), "r+");
    if (fp_stdio == NULL) {
        fprintf (stderr, "\nCan not open terminal (errno: %s) !\n", strerror (errno));
        return;
    }

    stdio = fileno (fp_stdio);
    if (stdio < 0)
    {
        fprintf (stderr, "Error: Invalid stream for terminal (errno %d: %s)!\n",
                 errno, strerror (errno));
        return;
    }

    printf("\n");
    printf("=================================================\n");
    printf("|           BOOTLOADER, Terminal mode           |\n");
    printf("| CTRL C: exit program                          |\n");
    printf("| CTRL F: enter filename                        |\n");
    printf("| CTRL P: program file                          |\n");
    printf("| CTRL V: verify file                           |\n");
    printf("| CTRL E: erase device                          |\n");
    printf("=================================================\n");

    tcgetattr (fileno (fp_stdio), &old_term);

    curr_term = old_term;
    curr_term.c_lflag &= ~(ECHO | ECHOE | ECHOK | ECHONL | ICANON);

    /* Set maximum wait time on input */
    curr_term.c_cc[VTIME] = 0;

    /* Set minimum bytes to read */
    curr_term.c_cc[VMIN]  = 0;

    tcsetattr (fileno (fp_stdio), TCSAFLUSH, &curr_term);

    /* set new timeout on V24, we want responsive system */
    old_timeout = set_tty_timeout (iFd, 0);   /* no wait */

    ok = TRUE;

    do
    {
        FD_ZERO (&fdset);
        FD_SET (iFd, &fdset);
        FD_SET (stdio, &fdset);
        max_select = (iFd > stdio) ? iFd : stdio;

        /* -- set max. waittime -- */
        timeout.tv_sec  = 1;
        timeout.tv_usec = 500000;

        errno = 0;

        ret_val = select (max_select + 1, &fdset, NULL, NULL, &timeout);

        if (ret_val > 0)
        {
            /* -- we got something on stdin ?? -- */
            if (FD_ISSET (stdio, &fdset))
            {
                /* stdin: someone hacked the keyboard */
                ok = handle_keyboard (fp_stdio, iFd, autoreset);
            }

            /* -- we got something from serial line -- */
            if (FD_ISSET (iFd, &fdset))
            {
                /* handle V24 input here */
                handle_input (iFd, fp_stdio);
            }
        }
        else if (ret_val < 0)
        {
            /*
            ** Check if SIGINT was received. This is handled further down,
            ** so we ignore the error from select in that case.
            */
            if (errno != EINTR)
            {
                fprintf (stderr, "Error reading from select: (%d) %s\n",
                         errno, strerror (errno));
                break;
            }
        }
        else
        {
            /* just timeout */
            esc_seq = 0;
        }
    } while (ok && running);

    /* reset old timeout */
    set_tty_timeout (iFd, old_timeout);

    tcsetattr (stdio, TCSAFLUSH, &old_term);
    fclose (fp_stdio);
}


/**
 * Main, startup
 */
int main(int argc, char *argv[])
{
    int     fd = 0;
    int     mode = 0;
    int     wait_bytetime = 0;  // as default, use tcdrain instead of waiting

    // default values
    speed_t     baudid = B0;
    autoreset_t autoreset = NO_AUTORESET;

    struct tms timestruct;
    struct sigaction sa;

    sa.sa_handler = sig_handler;
    sa.sa_flags = SA_NOMASK;

    sigaction (SIGHUP, &sa, NULL);
    sigaction (SIGINT, &sa, NULL);
    sigaction (SIGQUIT, &sa, NULL);
    sigaction (SIGTERM, &sa, NULL);

    /* set start time for stopwatch */
    start  = times (&timestruct);
    ticks = (double) sysconf (_SC_CLK_TCK);

    // print header
    printf("\n");
    printf("=================================================\n");
    printf("|           BOOTLOADER, Target: V2.1            |\n");
    printf("|            (" __DATE__ " " __TIME__ ")             |\n");
    printf("=================================================\n");

    // Parsing / checking parameter
    int i;

    for(i = 1; i < argc; i++)
    {
        if ((strcmp (argv[i], "-h") == 0) ||
            (strcmp (argv[i], "-?") == 0))
        {
            usage (argv[0]);
            exit (0);
        }
        else if (strcmp (argv[i], "-d") == 0)
        {
            i++;
            if (i < argc)
                device = argv[i];
        }
        else if (strcmp (argv[i], "-b") == 0)
        {
            i++;
            if (i < argc)
                baud = atoi(argv[i]);
        }
        else if (strcmp (argv[i], "-v") == 0)
        {
            mode |= AVR_VERIFY;
        }
        else if (strcmp (argv[i], "-p") == 0)
        {
            mode |= AVR_PROGRAM;
        }
        else if (strcmp (argv[i], "-e") == 0)
        {
            mode |= AVR_CLEAN;
        }
        else if (strcmp (argv[i], "-T") == 0)
        {
            mode |= AVR_TERMINAL;
        }
        else if (strcmp (argv[i], "-r") == 0)
        {
            autoreset = LOW_AUTORESET;
        }
        else if (strcmp (argv[i], "-R") == 0)
        {
            autoreset = HIGH_AUTORESET;
        }
        else if (strcmp (argv[i], "-t") == 0)
        {
            i++;
            if (i < argc)
                bsize = atoi(argv[i]);
            if (bsize <= 0)
            {
                printf ("Blocksize %d not allowed, setting it to 1\n", bsize);
                bsize = 1;
            }
        }
        else if (strcmp (argv[i], "-P") == 0)
        {
            i++;
            if (i < argc)
                password = argv[i];
        }
        else if (strcmp (argv[i], "-w") == 0)
        {
            i++;
            if (i < argc)
                wait_bytetime = atoi(argv[i]);
        }
        else
        {
            hexfile = argv[i];
        }
    }

    if ((hexfile == NULL) && (mode & (AVR_PROGRAM | AVR_VERIFY)))
    {
        printf("No hexfile specified!\n");
        usage(argv[0]);
    }

    if (mode == 0)
    {
        printf("No Verify / Program specified!\n");
        usage(argv[0]);
    }

    // Checking baudrate
    baudid = get_baudid (baud);

    if (baudid == B0)
    {
        printf("Unknown baudrate (%i)!\nUse standard like: "
               "50, 110, 150, 300, 600, 1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200, 230400\n",
               baud);
        usage(argv[0]);
    }

    fd = com_open(device, baudid, wait_bytetime); 

    if (fd < 0)
    {
        printf("Opening com port \"%s\" failed (%s)!\n", 
               device, strerror (errno));
        exit(2);
    }

    if (mode & (AVR_PROGRAM | AVR_VERIFY))
    {
        prog_reset (fd, autoreset);
        prog_verify (fd, mode, baud, bsize, password, device, hexfile);
    }
    else if (mode & (AVR_CLEAN))
    {
        printf ("You specified '-e' (for erase) without further option.\n"
                "Please add    '-p' if you want to erase the controller, or\n"
                "              '-v' if you want to check if the controller is empty!\n");
    }


    if (mode & AVR_TERMINAL)
        do_v24 (fd, autoreset);

    com_close(fd);                //close open com port
    return 0;
}

/* end of file */


