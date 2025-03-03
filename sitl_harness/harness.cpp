#include "config.hpp"
#include "state.hpp"
#include <chrono>
#include <cstdint>
#include <cstdlib>

extern "C" {
#include "ads1263.h"
}

#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>

std::ifstream data_file;
std::string curr_line;

double parse_timestamp(const std::string &timestamp) {
    // Parse the main part of the timestamp
    std::tm tm = {};
    std::istringstream ss(timestamp);

    // Parse date and time
    ss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");

    if (ss.fail()) {
        throw std::runtime_error("Failed to parse timestamp");
    }

    // Parse microseconds
    char dot;
    double fractional_seconds = 0.0;
    ss >> dot;
    if (dot == '.') {
        std::string usec_str;
        for (char c; ss >> c && std::isdigit(c);) {
            usec_str.push_back(c);
        }

        fractional_seconds = std::stod("0." + usec_str);

        // Back up one character since we read the timezone sign
        ss.unget();
    }

    // Parse timezone offset
    char sign;
    int tz_hour = 0, tz_min = 0;
    ss >> sign >> tz_hour >> tz_min;

    // Adjust for timezone
    int tz_offset = (sign == '-' ? -1 : 1) * (tz_hour * 3600 + tz_min * 60);

    // Convert to time_t (UTC)
    std::time_t time = std::mktime(&tm);

    // std::mktime assumes the time is in local timezone, adjust for that
    time -=
        (std::mktime(std::localtime(&time)) - std::mktime(std::gmtime(&time)));

    // Now adjust for the specified timezone in the string
    time -= tz_offset;

    // Return as double (seconds + fractional part)
    return static_cast<double>(time) + fractional_seconds;
}

double get_time() {
    auto now = std::chrono::high_resolution_clock::now();
    auto duration = now.time_since_epoch();
    return std::chrono::duration<double>(duration).count();
}

std::string get_curr_col(int col) {
    std::string token;
    std::istringstream ss(curr_line);
    for (int i = 0; i <= col; ++i) {
        std::getline(ss, token, ',');
    }
    return token;
}

double file_time() {
    std::string time_str = get_curr_col(0);
    return parse_timestamp(time_str);
}

double real_t0;
double file_t0;

extern "C" {
void pspl_gpio_init(void) {
    BB_State::bb_fu_state = BB_State::State::REGULATE;
    BB_State::bb_ox_state = BB_State::State::REGULATE;

    // load the data file
    data_file.open("sitl_data/sensornet_data_delta_cf_2_out_cut.csv");
    if (!data_file.is_open()) {
        std::cerr << "Error opening file" << std::endl;
        exit(EXIT_FAILURE);
    }

    std::string header;
    std::getline(data_file, header);
    // std::cout << "Header: " << header << std::endl;

    // read the first line of data
    std::getline(data_file, curr_line);
    file_t0 = file_time();
    real_t0 = get_time();
}
void pspl_spi_init(void) {}

uint8_t ADS1263_init_ADC1(ADS1263_DRATE rate) {
    return 0;
}

int usleep(__useconds_t usec);
uint32_t ADS1263_GetChannalValue(uint8_t Channel) {
    usleep(200); // should be 200us between samples
    double dt = get_time() - real_t0;
    while (file_time() - file_t0 < dt) {
        std::getline(data_file, curr_line);
        // std::cout << "Curr line: " << curr_line << std::endl;
        if (data_file.eof()) {
            exit(0);
        }
    }

    int col_idx;
    if (Channel == Telemetry::CHANNEL_PT_FU) {
        col_idx = 1;
    } else if (Channel == Telemetry::CHANNEL_PT_OX) {
        col_idx = 2;
    } else {
        return 0;
    }

    const std::string psi_str = get_curr_col(1);
    const double psi = std::stod(psi_str);
    return static_cast<uint32_t>(psi * 1000000.0);
}
void ADS1263_SetDiffChannal(uint8_t Channal) {}
void ADS1263_SetMode(uint8_t Mode) {}
}

void fsw_gpio_init() {}
void fsw_gpio_cleanup() {}

int fsw_gpio_set_fu(int value) {
    if (BB_State::bb_fu_state == BB_State::State::ISOLATE) {
        return 0;
    }

    double dt = get_time() - real_t0;
    std::cout << "FU," << dt << "," << value << "\n";
    return 0;
}
int fsw_gpio_set_ox(int value) {
    if (BB_State::bb_ox_state == BB_State::State::ISOLATE) {
        return 0;
    }

    double dt = get_time() - real_t0;
    std::cout << "OX," << dt << "," << value << "\n";
    return 0;
}