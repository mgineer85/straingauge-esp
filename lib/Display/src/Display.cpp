#include <Display.hpp>

///
namespace Display
{

    Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

    double _force = 0;
    int32_t _reading = 0;
    float _battery = 0;

    ///
    void initialize()
    {

        // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
        if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS))
        {
            Serial.println(F("SSD1306 allocation failed, freezing..."));
            for (;;)
                ; // Don't proceed, loop forever
        }

        // Init
        display.clearDisplay();
        display.setTextColor(SSD1306_WHITE, SSD1306_BLACK);
        display.cp437(true);           // Use full 256 char 'Code Page 437' font
        display.setTextSize(1);        // Normal 1:1 pixel scale
        display.setCursor(0, 32 - 16); // Start at top-left corner
        display.print("starting...");

        // Show the display buffer on the screen. You MUST call display() after
        // drawing commands to make them visible on screen!
        display.display();
    }

    void set_variables(double force, int32_t reading, float battery)
    {
        _force = force;
        _reading = reading;
        _battery = battery;
    }

    void static_content()
    {
        display.clearDisplay();

        // force
        display.setTextSize(2);  // Normal 1:1 pixel scale
        display.setCursor(0, 8); // Start at top-left corner
        display.print("F=");

        // N
        display.setTextSize(1);         // Normal 1:1 pixel scale
        display.setCursor(100, 32 - 8); // Start at top-left corner
        display.print("[N]");

        // Battery
        display.setTextSize(1);       // Normal 1:1 pixel scale
        display.setCursor(0, 32 - 8); // Start at top-left corner
        display.print("Battery");

        display.display();
    }

    void update_loop()
    {
        // force
        display.setTextSize(3);   // Normal 1:1 pixel scale
        display.setCursor(20, 0); // Start at top-left corner
        display.printf("%5.0f", _force);

        // N
        display.setTextSize(1);        // Normal 1:1 pixel scale
        display.setCursor(60, 32 - 8); // Start at top-left corner
        display.printf("%3.0f%%", _battery);

        display.display();
    }

}