uint8_t sendIntOut64(const uint8_t *buf) {
    for (uint16_t tries = 1; tries <= 200; tries++) {
        uint8_t r = Hid.SndRpt(64, (uint8_t*)buf);
        if (r == 0) { Serial.print(F("✅ tries=")); Serial.println(tries); return r; }
        if (r == HRNAK) { Usb.Task(); delay(5); continue; }
        return r;
    }
    return HRNAK;
}

uint8_t sendIntOutPadded64(const uint8_t *buf, uint16_t len) {
    uint8_t tmp[64] = {0};
    if (buf && len) {
        if (len > sizeof(tmp)) len = sizeof(tmp);
        memcpy(tmp, buf, len);
    }
    return sendIntOut64(tmp);
}

void SendTestChar(uint8_t ch) {
    // Display behavior notes:
    // - Sending the main display frames below (rpt1+rpt2+rpt3) makes the wheel
    //   show the content for about ~2 seconds.

    // Report #1: contains the character and a bargraph value.
    //   Layout sketch (approx):
    //   [0] 0x60 [2] 0x00       : report header
    //   [2] 0x42                : display-command
    //   [3] 0x21                : payload length?
    //   [4] 0xD0 [5] 0x63       : ?
    //   [6] 0x21                : payload length?
    //   ...                     : ?
    //   [16] 0x43 [17] 0x06     : identifier, key? 
    //   [18] 0x00               : ASCII character to display
    //   [19] 0x54 [20] 0x07     : identifier, key?
    //   [21] 0x00 [22] 0x00     : RPM Data values max max 65535
    //   [23] 0x54 [24] 0x08     : identifier, key?
    //   [25] 0x00 [26] 0x00     : bargraph values
    //   [27] 0x54 [28] 0x09     : identifier, key?
    //   [29] 0x00 [30] 0x00     : SPEED Data values max 4070
    //   [31] 0x46 [32] 0x0b     : ?
    uint8_t rpt1[] = {
        0x60, 0x00, 0x42, 0x21, 0xd0, 0x63, 0x21,
        0x00, 0xa2, 0x88, 0x46, 0x05, 0x01, 0x00, 0x00, 0x00,
        0x43, 0x06, 0x00,
        0x54, 0x07, 0xff, 0xff,
        0x54, 0x08, 0x00, 0x00,
        0x54, 0x09, 0x00, 0x00,
        0x46, 0x0b
    };

    static uint16_t speed = 200;
    uint16_t ref = 4070;                            //65535
    uint32_t num = (uint32_t)speed * wheelMapper.WHEEL_MAX_VALUE + ref / 2;
    uint16_t v = num / ref;
    rpt1[29] = (uint8_t)(v & 0xFF);
    rpt1[30] = (uint8_t)(v >> 8);

    // Report #2:
    //   Layout skecht (approx):
    //   [0] 0x60 [2] 0x00       : report header
    //   [2] 0x42                : display-command
    //   [3] 0x1C                : payload length?
    //   [4] 0xD0 [5] 0x63       : ?
    //   [6] 0x1C                : payload length?
    //   ...                     : ?
    //   [16] 0x54 [17] 0x31     : identifier, key?
    //   [18] 0x00 [19] 0x00     : POS DRI first data values (example: 0x1a,0xa0 = 0xa01a = 40986; the display shows only the last 2 digits: 40986 % 100 = 86 -> "86/00")
    //                           : efective renge 0x00,0x00 .. 0x63,0x00 (0 .. 99)
    //   [20] 0x54 [21] 0x32     : identifier, key?
    //   [22] 0x00 [23] 0x00     : POS DRI second data values
    //   [24] 0x54 [25] 0x33     : identifier, key?
    //   [26] 0x00 [27] 0x00     : POS LAP first data values
    //   [28] 0x54 [29] 0x34     : identifier, key?
    //   [30] 0x00 [31] 0x00     : POS LAP second data values
    uint8_t rpt2[] = {
        0x60, 0x00, 0x42, 0x1c, 0xd0, 0x63, 0x1c,
        0x00, 0xc2, 0x2a, 0x46, 0x0a, 0x00, 0x00, 0x00, 0x00,
        0x54, 0x31, 0x62, 0x00,
        0x54, 0x32, 0x63, 0x00,
        0x54, 0x33, 0x00, 0x00,
        0x54, 0x34, 0x00, 0x00,
    };

    // Report #3: i don't understand what this does, but it's needed.
    uint8_t rpt3[] = {
        0x60, 0x00, 0x42, 0x0c, 0xd0, 0x63, 0x0c,
        0x00, 0xd2, 0x0c, 0x46, 0x36
    };

    // Patch the ASCII character to display in the center of the Display.
    rpt1[18] = ch;

    // Optional demo behavior:
    // If we send ASCII '0'..'9', also set the bottom bargraph to that level.
    //
    // About the bargraph value (b):
    // - It's not just 0..9; it behaves like a *range* (likely ~10-bit).
    // - Higher values increase fill intensity; very high values (~0x03FF)
    //   appear to make all 9 bars blink.
    //
    // The table below is a set of empirically found "nice" values for 0..9.
    if (ch >= '0' && ch <= '9') {
        uint8_t bars = (uint8_t)(ch - '0');
        uint16_t value = 0x0000;
        switch (bars) {
            case 0: value = 0x0055; break;
            case 1: value = 0x00CC; break;
            case 2: value = 0x0155; break;
            case 3: value = 0x01CC; break;
            case 4: value = 0x0211; break;
            case 5: value = 0x02AA; break;
            case 6: value = 0x02CC; break;
            case 7: value = 0x0311; break;
            case 8: value = 0x0355; break;
            case 9: value = 0x03AA; break;
        }

        rpt1[25] = (uint8_t)(value & 0xFF);
        rpt1[26] = (uint8_t)(value >> 8);
    }

    sendIntOutPadded64(rpt1, sizeof(rpt1));
    sendIntOutPadded64(rpt2, sizeof(rpt2));
    sendIntOutPadded64(rpt3, sizeof(rpt3));
}
void SendDisplayKeepAlive() {
    uint8_t rpt1[] = {
        0x60, 0x00, 0x42, 0x06, 0xd0, 0x63, 0x06,
        0x00, 0xe6, 0xb8
    };
    sendIntOutPadded64(rpt1, sizeof(rpt1));
}
