PROGMEM char usbHidReportDescriptor[63] = {
    0x05, 0x0c,                    // USAGE_PAGE (Consumer Devices)
    0x09, 0x01,                    // USAGE (Consumer Control)
    0xa1, 0x01,                    // COLLECTION (Application)
    0x09, 0x30,                    //   USAGE (Power)
    0x09, 0x32,                    //   USAGE (Sleep)
    0x09, 0x40,                    //   USAGE (Menu)
    0x09, 0x41,                    //   USAGE (Menu Pick)
    0x09, 0x42,                    //   USAGE (Menu Up)
    0x09, 0x43,                    //   USAGE (Menu Down)
    0x09, 0x44,                    //   USAGE (Menu Left)
    0x09, 0x45,                    //   USAGE (Menu Right)
    0x09, 0x46,                    //   USAGE (Menu Escape)
    0x09, 0x47,                    //   USAGE (Menu Value Increase)
    0x09, 0x48,                    //   USAGE (Menu Value Decrease)
    0x09, 0xb0,                    //   USAGE (Play)
    0x09, 0xb1,                    //   USAGE (Pause)
    0x09, 0xb2,                    //   USAGE (Record)
    0x09, 0xb3,                    //   USAGE (Fast Forward)
    0x09, 0xb4,                    //   USAGE (Rewind)
    0x09, 0xb5,                    //   USAGE (Scan Next Track)
    0x09, 0xb6,                    //   USAGE (Scan Previous Track)
    0x09, 0xb7,                    //   USAGE (Stop)
    0x09, 0xb9,                    //   USAGE (Random Play)
    0x09, 0xe2,                    //   USAGE (Mute)
    0x09, 0xe9,                    //   USAGE (Volume Up)
    0x09, 0xea,                    //   USAGE (Volume Down)
    0x15, 0x01,                    //   LOGICAL_MINIMUM (1)
    0x25, 0x17,                    //   LOGICAL_MAXIMUM (23)
    0x75, 0x08,                    //   REPORT_SIZE (8)
    0x95, 0x01,                    //   REPORT_COUNT (1)
    0x81, 0x40,                    //   INPUT (Data,Ary,Abs,Null)
    0xc0                           // END_COLLECTION
};

