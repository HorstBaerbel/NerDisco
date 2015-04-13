// Arduino interface for the use of WS2812 operated LEDs with the NerDisco software.
// Uses Adalight protocol and is compatible with Boblight, Prismatik etc.
// The "magic word" for synchronisation is "Ada" followed by LED count high and low byte and a checksum (the low and high byte XORed with 0x55).
// The interface sends the string "Ada\n" in 1000ms intervals when idle, so the software can detect the display.

#include <FastLED.h> //Tested with FastLED v3.0

#define NUM_LEDS 540 //Define the number of LEDs (my configuration is 30*18=540)
#define PIN 6 //Define (software) LED data port pin
#define CALIBRATION_TEMPERATURE TypicalLEDStrip
#define MAX_BRIGHTNESS 255 // 0-255
#define SERIALRATE 500000//250000//230400//115200 //Define serial port baud rate. Make sure this is high enough for a decent refresh rate
#define SERIAL_TIMEOUT 1000 //turn display off after a second. this is also the SCK send interval

//Adalight sends a "magic word" (defined in /etc/boblight.conf) before sending the pixel data
static const uint8_t magic[] = { 'A', 'd', 'a' };
uint8_t hi, lo, checksum;
uint16_t i;

// initialise LED-array
CRGB leds[NUM_LEDS];

void setup()
{
	//clear led color buffer to black
	memset(leds, 0, NUM_LEDS * sizeof(struct CRGB));
	//initialize FastLed library for the WS2812B strip
	FastLED.addLeds<WS2812B, PIN, RGB>(leds, NUM_LEDS);
	FastLED.setTemperature(CALIBRATION_TEMPERATURE);
	FastLED.setBrightness(MAX_BRIGHTNESS);
	//initial RGB flash
	LEDS.showColor(CRGB(127, 0, 0));
	delay(333);
	LEDS.showColor(CRGB(0, 127, 0));
	delay(333);
	LEDS.showColor(CRGB(0, 0, 127));
	delay(333);
	LEDS.showColor(CRGB(0, 0, 0));
	//open serial port
	Serial.begin(SERIALRATE);
	Serial.print("Ada\n");
}

void loop() {
	//synchronize transmission using magic word, sizes and checksum
	//read individual bytes of magic word from serial port and compare them
	for (i = 0; i < sizeof magic; ++i) {
		if (waitForBytes(1) == false) {
			return;
		}
		// Check next byte in magic word
		if (magic[i] == Serial.read()) continue;
		// otherwise, wait for first byte again...
		i = 0;
	}
	//read count high and low byte and checksum
	if (waitForBytes(3) == false) {
		return;
	}
	hi = Serial.read();
	lo = Serial.read();
	checksum = Serial.read();
	// if checksum does not match go back synchronize again
	if (checksum != (hi ^ lo ^ 0x55)) {
		return;
	}
	//clear led color buffer to black
	memset(leds, 0, NUM_LEDS * sizeof(struct CRGB));
	// read the transmission data and set LED values
	for (i = 0; i < NUM_LEDS;) {
		if (waitForBytes(45) == false) {
			return;
		}
		Serial.readBytes((uint8_t*)(&leds[i]), 45);
                i += 15;
	}
	// shows new values
	FastLED.show();
	//If you have problems, try one of these two:
	//#1 - short delay to make sure that if buffer is overrun, no corrupted data is shown
	//delayMicroseconds(200);
	//#2 - drain serial buffer
	/*while (Serial.available() > 0) {
            Serial.read();
	}*/
}

boolean waitForBytes(int numberOfBytes)
{
	unsigned long lastReceiveTime = millis();
	while (Serial.available() < numberOfBytes)
	{
		if ((millis() - lastReceiveTime) >= SERIAL_TIMEOUT)
		{
			//erase display
			memset(leds, 0, NUM_LEDS * sizeof(struct CRGB));
			FastLED.show();
			//Send standard identifier string to host
			Serial.print("Ada\n");
			return false;
		}
	}
	return true;
}
