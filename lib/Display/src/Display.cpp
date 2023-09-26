#include <Display.hpp>

///
namespace Display
{

    // Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

    U8G2_SH1106_128X64_NONAME_F_HW_I2C display(U8G2_R0, /* reset=*/U8X8_PIN_NONE);

    double _force = 0;
    int32_t _reading = 0;
    float _battery = 0;

    ///
    void initialize()
    {

        // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
        if (!display.begin())
        {
            Serial.println(F("Display begin failed, freezing..."));
            for (;;)
                ; // Don't proceed, loop forever
        }
        Serial.println(F("Display initialized"));

        // Init
        display.clearBuffer();                 // clear the internal memory
        display.setFont(u8g2_font_ncenB08_tr); // choose a suitable font
        display.drawStr(0, 10, "starting..."); // write something to the internal memory
        display.sendBuffer();                  // transfer internal memory to the display
    }

    void set_variables(double force, int32_t reading, float battery)
    {
        _force = force;
        _reading = reading;
        _battery = battery;
    }

    void static_content()
    {
        display.clearBuffer();

        display.setFont(u8g2_font_ncenB08_tr); // choose a suitable font

        // force
        display.drawStr(0, 8, "F=");

        // N
        display.drawStr(100, 32 - 8, "[N]");

        // Battery Icon
        display.setFont(u8g2_font_unifont_t_symbols);
        display.drawGlyph(5, 64, 0x2603); /* dec 9731/hex 2603 Snowman */
    }

    void update_loop()
    {
        char buf[10];

        // force
        display.setFont(u8g2_font_inr24_mn); // choose a suitable font
        sprintf(buf, "%5.0f", _force);
        display.drawStr(0, 25, buf);
        display.print(_force, 0);

        // battery
        display.setFont(u8g2_font_ncenB08_tr); // choose a suitable font
        sprintf(buf, "%3.0f%%", _battery);
        display.drawStr(60, 64 - 8, buf);

        display.sendBuffer();
    }

}