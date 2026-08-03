// pwm_pin32.cpp
// Jetson Orin Nano, Linux
// Generates a 50% duty-cycle PWM signal on physical pin 32.
// The PWM source clock is left at its default setting.

#include <chrono>
#include <csignal>
#include <fstream>
#include <iostream>
#include <string>
#include <thread>
#include <unistd.h>

constexpr const char* PWM_CHIP = "/sys/class/pwm/pwmchip3";
constexpr int PWM_CHANNEL = 0;

// 2,000,000 ns period = 500 Hz
constexpr long PERIOD_NS = 2'000'000;
constexpr long DUTY_NS = PERIOD_NS / 2;

volatile std::sig_atomic_t running = true;

void stopPWM(int)
{
    running = false;
}

bool writeFile(const std::string& path, const std::string& value)
{
    std::ofstream file(path);

    if (!file) {
        std::cerr << "Failed to open: " << path << '\n';
        return false;
    }

    file << value;

    if (!file) {
        std::cerr << "Failed to write to: " << path << '\n';
        return false;
    }

    return true;
}

int main()
{
    std::signal(SIGINT, stopPWM);
    std::signal(SIGTERM, stopPWM);

    const std::string pwmPath =
        std::string(PWM_CHIP) + "/pwm" + std::to_string(PWM_CHANNEL);

    // Export the PWM channel if it has not already been exported.
    if (access(pwmPath.c_str(), F_OK) != 0) {
        if (!writeFile(
                std::string(PWM_CHIP) + "/export",
                std::to_string(PWM_CHANNEL))) {
            return 1;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    // Disable PWM before changing its settings.
    writeFile(pwmPath + "/enable", "0");

    // Duty cycle must not exceed the period while reconfiguring.
    if (!writeFile(pwmPath + "/duty_cycle", "0")) {
        return 1;
    }

    if (!writeFile(pwmPath + "/period", std::to_string(PERIOD_NS))) {
        return 1;
    }

    if (!writeFile(pwmPath + "/duty_cycle", std::to_string(DUTY_NS))) {
        return 1;
    }

    if (!writeFile(pwmPath + "/enable", "1")) {
        return 1;
    }

    std::cout << "PWM running on physical pin 32\n";
    std::cout << "Frequency: 500 Hz\n";
    std::cout << "Duty cycle: 50%\n";
    std::cout << "Press Ctrl+C to stop.\n";

    while (running) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    writeFile(pwmPath + "/enable", "0");

    std::cout << "\nPWM stopped.\n";

    return 0;
}