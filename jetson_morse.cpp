#include <atomic>
#include <chrono>
#include <cctype>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <linux/gpio.h>
#include <stdexcept>
#include <string>
#include <sys/ioctl.h>
#include <thread>
#include <unistd.h>
#include <unordered_map>

using namespace std;
using namespace std::chrono;

constexpr unsigned GPIO_LINE = 41;          // Physical pin 32 on Orin Nano
constexpr int WPM = 18;
constexpr int UNIT_MS = 1200 / WPM;
constexpr int HOME_US = 1200;
constexpr int PRESS_US = 1800;
constexpr int FRAME_US = 20000;              // 50 Hz

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

string findGpioChip() {
    for (int i = 0; i < 16; ++i) {
        string path = "/dev/gpiochip" + to_string(i);
        int fd = open(path.c_str(), O_RDONLY | O_CLOEXEC);
        if (fd < 0) continue;

        gpiochip_info info{};
        bool match = ioctl(fd, GPIO_GET_CHIPINFO_IOCTL, &info) == 0 &&
                     string(info.label) == "tegra234-gpio";
        close(fd);
        if (match) return path;
    }
    throw runtime_error("Could not find tegra234-gpio");
}

class GpioOutput {
public:
    GpioOutput() {
        string chip = findGpioChip();
        int chipFd = open(chip.c_str(), O_RDONLY | O_CLOEXEC);
        if (chipFd < 0) throw runtime_error("Cannot open " + chip);

        gpiohandle_request request{};
        request.lineoffsets[0] = GPIO_LINE;
        request.flags = GPIOHANDLE_REQUEST_OUTPUT;
        request.lines = 1;
        request.default_values[0] = 0;
        strcpy(request.consumer_label, "morse-servo");

        if (ioctl(chipFd, GPIO_GET_LINEHANDLE_IOCTL, &request) < 0) {
            string error = strerror(errno);
            close(chipFd);
            throw runtime_error("Cannot use pin 32: " + error);
        }

        close(chipFd);
        fd = request.fd;
    }

    ~GpioOutput() {
        set(false);
        if (fd >= 0) close(fd);
    }

    void set(bool high) noexcept {
        if (fd < 0) return;
        gpiohandle_data data{};
        data.values[0] = high;
        ioctl(fd, GPIOHANDLE_SET_LINE_VALUES_IOCTL, &data);
    }

private:
    int fd = -1;
};

class Servo {
public:
    Servo() : worker(&Servo::run, this) {}

    ~Servo() {
        running = false;
        if (worker.joinable()) worker.join();
    }

    void home()  { pulseUs = HOME_US; }
    void press() { pulseUs = PRESS_US; }

private:
    void run() {
        auto nextFrame = steady_clock::now();

        while (running) {
            pin.set(true);
            this_thread::sleep_for(microseconds(pulseUs.load()));
            pin.set(false);

            nextFrame += microseconds(FRAME_US);
            this_thread::sleep_until(nextFrame);
        }
        pin.set(false);
    }

    GpioOutput pin;
    atomic<int> pulseUs{HOME_US};
    atomic<bool> running{true};
    thread worker;
};

const string* morseFor(unsigned char character) {
    auto found = MORSE.find(static_cast<char>(toupper(character)));
    return found == MORSE.end() ? nullptr : &found->second;
}

string toMorse(const string& text) {
    string output;
    bool haveLetter = false;
    bool wordGap = false;

    for (unsigned char character : text) {
        if (isspace(character)) {
            if (haveLetter) wordGap = true;
            continue;
        }

        const string* code = morseFor(character);
        if (!code) continue;

        if (haveLetter) output += wordGap ? " / " : " ";
        output += *code;
        haveLetter = true;
        wordGap = false;
    }
    return output;
}

void pauseUnits(int units) {
    this_thread::sleep_for(milliseconds(units * UNIT_MS));
}

void transmit(Servo& servo, const string& text) {
    bool haveLetter = false;
    bool wordGap = false;

    for (unsigned char character : text) {
        if (isspace(character)) {
            if (haveLetter) wordGap = true;
            continue;
        }

        const string* code = morseFor(character);
        if (!code) continue;

        if (haveLetter) pauseUnits(wordGap ? 7 : 3);

        for (size_t i = 0; i < code->size(); ++i) {
            servo.press();
            pauseUnits((*code)[i] == '.' ? 1 : 3);
            servo.home();
            if (i + 1 < code->size()) pauseUnits(1);
        }

        haveLetter = true;
        wordGap = false;
    }
    servo.home();
}

int main() {
    try {
        Servo servo;
        this_thread::sleep_for(milliseconds(500));

        cout << "Morse servo ready on physical pin 32\n"
             << "Type a message, or type exit.\n\n";

        string text;
        while (true) {
            cout << "Message: " << flush;
            if (!getline(cin, text) || text == "exit" || text == "quit") break;
            if (text.empty()) continue;

            cout << "Morse: " << toMorse(text) << '\n';
            transmit(servo, text);
            cout << "Done.\n\n";
        }
    } catch (const exception& error) {
        cerr << "Error: " << error.what() << '\n';
        cerr << "Configure pin 32 as GPIO and run with sudo.\n";
        return 1;
    }
}
