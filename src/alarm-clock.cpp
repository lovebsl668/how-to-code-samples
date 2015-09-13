#include <stdlib.h>
#include <iostream>
#include <unistd.h>
#include <sstream>
#include <thread>
#include <chrono>

#include <grove.h>
#include <buzzer.h>
#include <jhd1313m1.h>

#include "../lib/restclient-cpp/include/restclient-cpp/restclient.h"
#include "../lib/crow/crow_all.h"

upm::GroveRotary* rotary;
upm::GroveButton* button;
upm::Buzzer* buzzer;
upm::Jhd1313m1* screen;

float rotary_value = 0;

void message(upm::Jhd1313m1* screen, const std::string& input, const std::size_t color = 0x0000ff) {
	std::size_t red   = (color & 0xff0000) >> 16;
	std::size_t green = (color & 0x00ff00) >> 8;
	std::size_t blue  = (color & 0x0000ff);

	// TODO: pad input string to fill up display
	std::stringstream text;
	text << input;
	screen->setCursor(0,0);
	screen->write(text.str());
	screen->setColor(red, green, blue);
}

void runner(upm::GroveRotary* rotary, upm::GroveButton* button, float& rot) {
	bool wasPressed = false;
	bool currentlyPressed = false;

	for (;;) {
		currentlyPressed = button->value();
		if ( currentlyPressed && ! wasPressed ) {
			std::cerr << "Pressed" << std::endl;
		} else if (! currentlyPressed && wasPressed ) {
			std::cerr << "Released" << std::endl;
		}

		rot = rotary->abs_value();
		std::cerr << "Rotary: " << rot << std::endl;

		wasPressed = currentlyPressed;
		std::this_thread::sleep_for(std::chrono::milliseconds(500));
	}
}

void initDevices() {
	// rotary connected to A0 (analog in)
	rotary = new upm::GroveRotary(0);

	// button connected to D4 (digital out)
	button = new upm::GroveButton(4);

	// buzzer connected to D5 (digital out)
	buzzer = new upm::Buzzer(5);

	// screen connected to the default I2C bus
	screen = new upm::Jhd1313m1(0);
}

void deleteDevices() {
	delete rotary;
	delete button;
	delete buzzer;
	delete screen;
}

int main() {
	initDevices();

	std::thread t1(runner, rotary, button, std::ref(rotary_value));

	crow::SimpleApp app;

	CROW_ROUTE(app, "/")
	([]() {
		std::stringstream text;
		text << "Rotary: " << rotary_value;
		return text.str();
	});

	app.port(4567).multithreaded().run();

	t1.join();

	deleteDevices();

	return 0;
}
