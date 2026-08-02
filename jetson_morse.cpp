#include <chrono>
#include <cctype>
#include <fstream>
#include <iostream>
#include <string>
#include <thread>
#include <unordered_map>

using namespace std;
using namespace std::chrono_literals;

constexpr int MORSE_PWM_PIN = 32;  // Physical Jetson Nano header pin
constexpr int WPM = 5;
constexpr int UNIT_MS = 1200 / WPM;
constexpr int HOME = 1000000;   // 1.0ms (0 degrees)
constexpr int PRESS = 2000000;  // 2.0ms (180 degrees for bigger swing)
constexpr int PWM_FREQUENCY = 50; // 50 Hz for standard servos

const unsigned long long PERIOD_NS =
    1'000'000'000ULL / PWM_FREQUENCY;

string pwmPathForPin(int pin) {
    // Physical pin 32 is configured as PWM using Jetson-IO.
    // On this setup it is exposed as pwmchip0, channel 0.
    if (pin == 32)
        return "/sys/class/pwm/pwmchip0/pwm0/";

    throw runtime_error("No PWM mapping defined for physical pin " +
                        to_string(pin));
}

const string PWM_PATH = pwmPathForPin(MORSE_PWM_PIN);

const unordered_map<char, string> MORSE = {
    {'A', ".-"}, {'B', "-..."}, {'C', "-.-."}, {'D', "-.."}, {'E', "."},
    {'F', "..-."}, {'G', "--."}, {'H', "...."}, {'I', ".."}, {'J', ".---"},
    {'K', "-.-"}, {'L', ".-.."}, {'M', "--"}, {'N', "-."}, {'O', "---"},
    {'P', ".--."}, {'Q', "--.-"}, {'R', ".-."}, {'S', "..."}, {'T', "-"},
    {'U', "..-"}, {'V', "...-"}, {'W', ".--"}, {'X', "-..-"},
    {'Y', "-.--"}, {'Z', "--.."},
    {'0', "-----"}, {'1', ".----"}, {'2', "..---"}, {'3', "...--"},
    {'4', "....-"}, {'5', "....."}, {'6', "-...."}, {'7', "--..."},
    {'8', "---.."}, {'9', "----."}
};

void writeFile(const string& file, unsigned long long value) {
    ofstream out(file);
    if (!out) throw runtime_error("Cannot open " + file);
    out << value;
    if (out.fail()) throw runtime_error("Failed to write value to " + file + " (Kernel rejected it)");
}

void setPWM(int duty_ns) {
    writeFile(PWM_PATH + "duty_cycle", duty_ns);
}

void initialisePWM() {
    writeFile(PWM_PATH + "enable", 0);
    writeFile(PWM_PATH + "duty_cycle", 0); // MUST be zero before changing period on Jetson
    writeFile(PWM_PATH + "period", PERIOD_NS);
    setPWM(HOME);
    writeFile(PWM_PATH + "enable", 1);
}

string toMorse(const string& text) {
    string result;

    for (unsigned char raw : text) {
        if (isspace(raw)) {
            if (!result.empty() && result.back() != '/') result += " / ";
            continue;
        }

        auto it = MORSE.find(static_cast<char>(toupper(raw)));
        if (it == MORSE.end()) continue;

        if (!result.empty() && result.back() != ' ') result += ' ';
        result += it->second;
    }

    return result;
}

void pauseUnits(int units) {
    this_thread::sleep_for(chrono::milliseconds(units * UNIT_MS));
}

void transmit(const string& text) {
    bool firstLetter = true;

    for (unsigned char raw : text) {
        if (isspace(raw)) {
            if (!firstLetter) pauseUnits(4);  // 3 already elapsed + 4 = 7 units.
            firstLetter = true;
            continue;
        }

        auto it = MORSE.find(static_cast<char>(toupper(raw)));
        if (it == MORSE.end()) continue;

        if (!firstLetter) pauseUnits(2);  // 1 already elapsed + 2 = 3 units.

        for (char symbol : it->second) {
            setPWM(PRESS);
            pauseUnits(symbol == '.' ? 1 : 3);
            setPWM(HOME);
            pauseUnits(1);
        }
        firstLetter = false;
    }

    setPWM(HOME);
}
int main() {
    try {
        initialisePWM();
        cout << "Morse servo ready: physical pin " << MORSE_PWM_PIN
             << ", " << PWM_FREQUENCY << " Hz\n";
        cout << "Type a message, or type exit.\n\n";

        string text;
        while (true) {
            cout << "Message: ";
            if (!getline(cin, text) || text == "exit" || text == "quit") break;

            string morse = toMorse(text);
            cout << "Morse: " << morse << '\n';
            transmit(text);
            cout << "Done.\n\n";
        }

        setPWM(HOME);
        writeFile(PWM_PATH + "enable", 0);
    } catch (const exception& error) {
        cerr << "Error: " << error.what() << '\n';
        cerr << "Check Jetson-IO, pwmchip/channel, and run with sudo.\n";
        return 1;
    }
}