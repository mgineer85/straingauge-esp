#include <Display.hpp>

#include <Fuelgauge.hpp> // -->g_Fuelgauge
#include <Loadcell.hpp>  // -->g_Loadcell

namespace Display
{

    // Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

    U8G2_SH1106_128X64_NONAME_F_HW_I2C display(U8G2_R0, /* reset=*/U8X8_PIN_NONE); // AZ-Delivery 1.3 display.
    // U8G2_SSD1327_WS_128X128_F_HW_I2C display(U8G2_R0, /* reset=*/U8X8_PIN_NONE); // Adafruit 1.5 display // Too slow with I2C

    u8g2_uint_t display_height = display.getDisplayHeight();
    u8g2_uint_t display_width = display.getDisplayWidth();

    ulong lastMillisStatusMessage = 0;

    const u8g2_uint_t line1 = 8;
    const u8g2_uint_t line2 = (line1 + 4) + 25;
    const u8g2_uint_t line3 = (line2 + 4) + 10;
    const u8g2_uint_t line4 = display_height;

    ///
    void initialize()
    {

        // display.setI2CAddress(0x7A); // 0x3D translates to  0x7A (reference says multiply by 2) //adafruit 1.5 display

        // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
        if (!display.begin())
        {
            log_e("Display begin failed, freezing...");
            for (;;)
                ; // Don't proceed, loop forever
        }
        log_v("Display initialized");

        // Init
        status_message("pls wait, starting...");
    }

    void status_message(String message)
    {
        display.clearBuffer();                                     // clear the internal memory
        display.setFont(u8g2_font_ncenB08_tr);                     // choose a suitable font
        display.drawStr(0, (display_height - 8), message.c_str()); // write something to the internal memory
        display.sendBuffer();                                      // transfer internal memory to the display

        // for debug: also print to serial
        log_i("%s", message.c_str());

        lastMillisStatusMessage = millis();
    }

    void draw_battery_icon(float battery_percent)
    {
        uint8_t symbol_offset = int(battery_percent / 9);
        symbol_offset = symbol_offset < 0 ? 0 : symbol_offset; // limit to 0
        symbol_offset = symbol_offset > 9 ? 9 : symbol_offset; // limit to +9

        uint16_t _symbol = 0xe242 + symbol_offset;

        // Battery Icon Font
        display.setFont(u8g2_font_siji_t_6x10);

        if (g_Fuelgauge.getIsCharging())                            // isCharging
            display.drawGlyph((display_width - 24), line1, 0xe216); /* Charge Symbol */

        display.drawGlyph((display_width - 12), line1 - 1, _symbol); /* Battery Bar */
    }

    void static_content()
    {
        display.clearBuffer();

        /*
        display: 128x64

        line1: 8: statusbar (displayunit, battery)
        line2: 24: value in displayunit
        line3: 8: line up to range

        line4: -8: statusmessage (up to 2 secs)
        */

        display.setFont(u8g2_font_ncenB08_tr); // choose a suitable font

        // displayunit unit
        display.drawStr(0, line1, (String("[") + String(g_Loadcell.sensor_config.displayunit) + String("]")).c_str());
        // fullrange
        display.drawStr(60, line1, (String(g_Loadcell.sensor_config.fullrange, 1) + String(g_Loadcell.sensor_config.displayunit)).c_str());

        // sensitivity
        display.drawStr(0, line3, String(g_Loadcell.sensor_config.sensitivity, 5).c_str());
        // zerobalance
        display.drawStr(64, line3, String(g_Loadcell.sensor_config.zerobalance, 5).c_str());
    }

    void update_loop()
    {
        char buf[10];

        // if ((millis() - lastMillisStatusMessage) > 2000)
        // {
        //     // clear last status message if older than xxxx milliseconds
        // }
        static_content();

        // value in displayunit
        display.setFont(u8g2_font_inr24_mn); // choose a suitable font
        sprintf(buf, "%5.0f", g_Loadcell.getReadingDisplayunitFiltered());
        display.drawStr(0, line2, buf);

        draw_battery_icon(g_Fuelgauge.getBatteryPercent());

        display.sendBuffer();
    }
}