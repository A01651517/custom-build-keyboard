#ifndef USB
#define USB

#include "usbdrv.h"
#include "oddebug.h"

/* We use a simplifed keyboard report descriptor which does not support the
 * boot protocol. We don't allow setting status LEDs and we only allow one
 * simultaneous key press (except modifiers). We can therefore use short
 * 2 byte input reports.
 * The report descriptor has been created with usb.org's "HID Descriptor Tool"
 * which can be downloaded from http://www.usb.org/developers/hidpage/.
 * Redundant entries (such as LOGICAL_MINIMUM and USAGE_PAGE) have been omitted
 * for the second INPUT item.
 */
const PROGMEM char usbHidReportDescriptor[35] = {   /* USB report descriptor */
    0x05, 0x01,                    // USAGE_PAGE (Generic Desktop)
    0x09, 0x06,                    // USAGE (Keyboard)
    0xa1, 0x01,                    // COLLECTION (Application)
    0x05, 0x07,                    //   USAGE_PAGE (Keyboard)
    0x19, 0xe0,                    //   USAGE_MINIMUM (Keyboard LeftControl)
    0x29, 0xe7,                    //   USAGE_MAXIMUM (Keyboard Right GUI)
    0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
    0x25, 0x01,                    //   LOGICAL_MAXIMUM (1)
    0x75, 0x01,                    //   REPORT_SIZE (1)
    0x95, 0x08,                    //   REPORT_COUNT (8)
    0x81, 0x02,                    //   INPUT (Data,Var,Abs)
    0x95, 0x01,                    //   REPORT_COUNT (1)
    0x75, 0x08,                    //   REPORT_SIZE (8)
    0x25, 0x65,                    //   LOGICAL_MAXIMUM (101)
    0x19, 0x00,                    //   USAGE_MINIMUM (Reserved (no event indicated))
    0x29, 0x65,                    //   USAGE_MAXIMUM (Keyboard Application)
    0x81, 0x00,                    //   INPUT (Data,Array,Abs)
    0xc0                           // END_COLLECTION
};

/**
 * Official HID key values for keyboard
 * devices
 * https://usb.org/sites/default/files/hut1_2.pdf
 */
#define KEY_A       4
#define KEY_B       5
#define KEY_C       6
#define KEY_D       7
#define KEY_E       8
#define KEY_F       9
#define KEY_G       10
#define KEY_H       11
#define KEY_I       12
#define KEY_J       13
#define KEY_K       14
#define KEY_L       15
#define KEY_M       16
#define KEY_N       17
#define KEY_O       18
#define KEY_P       19
#define KEY_Q       20
#define KEY_R       21
#define KEY_S       22
#define KEY_T       23
#define KEY_U       24
#define KEY_V       25
#define KEY_W       26
#define KEY_X       27
#define KEY_Y       28
#define KEY_Z       29
#define KEY_1       30
#define KEY_2       31
#define KEY_3       32
#define KEY_4       33
#define KEY_5       34
#define KEY_6       35
#define KEY_7       36
#define KEY_8       37
#define KEY_9       38
#define KEY_0       39

#define KEY_UP      82
#define KEY_RIGHT   79
#define KEY_DOWN    81
#define KEY_LEFT    80

/* Keyboard usage values, see usb.org's HID-usage-tables document, chapter
 * 10 Keyboard/Keypad Page for more codes.
 */
#define NO_MOD 0
#define MOD_CONTROL_LEFT    (1<<0)
#define MOD_SHIFT_LEFT      (1<<1)
#define MOD_ALT_LEFT        (1<<2)
#define MOD_GUI_LEFT        (1<<3)
#define MOD_CONTROL_RIGHT   (1<<4)
#define MOD_SHIFT_RIGHT     (1<<5)
#define MOD_ALT_RIGHT       (1<<6)
#define MOD_GUI_RIGHT       (1<<7)

static uchar reportBuffer[2]; /* HID Buffer */
static uchar idleRate; /* HID Idle Rate */
static uint pressedKey = 0;

#define NUM_KEYS 10
static const uchar keys[NUM_KEYS + 1][2] = {
    {0, 0}, // Reserved (no event)
    {NO_MOD, (uchar) KEY_1},
    {NO_MOD, (uchar) KEY_2},
    {NO_MOD, (uchar) KEY_3},
    {NO_MOD, (uchar) KEY_4},
    {NO_MOD, (uchar) KEY_5},
    {NO_MOD, (uchar) KEY_6},
    {NO_MOD, (uchar) KEY_7},
    {NO_MOD, (uchar) KEY_8},
    {NO_MOD, (uchar) KEY_9},
    {NO_MOD, (uchar) KEY_0}
};

static void updateReportBuffer(uint key) {
    reportBuffer[0] = keys[key][0];
    reportBuffer[1] = keys[key][1];
}

/* Reserved function ran by the V-USB driver for the Control endpoint */
uchar usbFunctionSetup(uchar data[8]) {
    // We cast the data request
    usbRequest_t *request = (void *)data;
    // We make sure that the request is an HID class request
    if ((request->bmRequestType & USBRQ_TYPE_MASK) == USBRQ_TYPE_CLASS) {
        switch (request->bRequest) {
            // We send the last pressed key
            case USBRQ_HID_GET_REPORT:
                // We update the report buffer
                updateReportBuffer(pressedKey);
                // We set the reserved V-USB pointer to the report buffer
                usbMsgPtr = reportBuffer;
                return sizeof(reportBuffer);
            // We share our idle rate with the host
            case USBRQ_HID_GET_IDLE:
                // Send the delay to the computer
                usbMsgPtr = &idleRate;
                return 1;
            // We get the idle rate from the host
            case USBRQ_HID_SET_IDLE:
                 // Get the default delay from the computer
                idleRate = request->wValue.bytes[1];
                break;
        }
    }
    return 0;
}
#endif
