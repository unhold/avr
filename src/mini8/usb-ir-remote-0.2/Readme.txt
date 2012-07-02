jorgen.birkler@gmail.com


Implements a USB HID device class device that receive IR remote 
and puts them to the computer as HID Remote control compatible codes.

Overview

/circuit/     is the hardware definition of the remote receiver for eagle cad (cadsoftusa.com)
/bootloader   is the USB compatible bootloader/flasher (once loaded the firmware can be uploaded using usb hid)
/irrx         is the ir receive code using interrupts
irrx_config.h configurated the /irrx code
/uart_io      provides a file io implementation for the uart to be used with fprintf
uart_io_config configs the /uart_io
/usbdrv       is the obdev.at firmware only usb driver
usbconfig.h   configs /usbdrv
ir_keyboard_2.hid a HID description file for the tool.



/obj          temp obj directory

