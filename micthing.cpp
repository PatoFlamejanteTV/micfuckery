// micthing.cpp

// Raw Audio Visualizer in C++
// Requires: PortAudio

// Build: g++ micthing.cpp -o micthing -lportaudio -std=c++17
#include <numeric>
#include <iostream>
#include <vector>
#include <cmath>
#include <portaudio.h>
#include <iomanip>
#include <algorithm>
#include <csignal>

// ANSI color codes for terminal output
#define RESET       "\033[0m"
#define BRIGHT      "\033[1m"
#define DIM         "\033[2m"
#define FG_RED      "\033[31m"
#define FG_GREEN    "\033[32m"
#define FG_YELLOW   "\033[33m"
#define FG_CYAN     "\033[36m"
#define FG_MAGENTA  "\033[35m"
#define FG_WHITE    "\033[37m"
#define FG_BLUE     "\033[34m"

// Configuration
constexpr int SAMPLE_RATE       = 44100;
constexpr int BLOCK_SIZE        = 1024;
constexpr int CHANNELS          = 1;
constexpr int DISPLAY_SAMPLES   = 20;
constexpr int PEAK_HISTORY      = 10;

// Color thresholds
constexpr float COLOR_THRESH_LOW    = 0.1f;
constexpr float COLOR_THRESH_MED    = 0.3f;
constexpr float COLOR_THRESH_HIGH   = 0.6f;

volatile sig_atomic_t keep_running = 1;

void intHandler(int dummy) {
    keep_running = 0;
}

class AudioVisualizer {
public:
    std::vector<float> peak_values;
    std::vector<float> rms_values;

    std::string get_color(float value) {
        float abs_val = std::abs(value);
        if (abs_val > COLOR_THRESH_HIGH) return std::string(FG_RED) + BRIGHT;
        if (abs_val > COLOR_THRESH_MED) return FG_YELLOW;
        if (abs_val > COLOR_THRESH_LOW) return FG_GREEN;
        return std::string(FG_WHITE) + DIM;
    }

    void update_stats(const float* buffer, int len) {
        float peak = 0.0f;
        float sum_sq = 0.0f;
        for (int i = 0; i < len; ++i) {
            float abs_val = std::abs(buffer[i]);
            if (abs_val > peak) peak = abs_val;
            sum_sq += buffer[i] * buffer[i];
        }
        float rms = std::sqrt(sum_sq / len);
        peak_values.push_back(peak);
        rms_values.push_back(rms);
        if ((int)peak_values.size() > PEAK_HISTORY) {
            peak_values.erase(peak_values.begin());
            rms_values.erase(rms_values.begin());
        }
    }

    std::string print_stats() {
        float avg_peak = 0.0f, avg_rms = 0.0f;
        if (!peak_values.empty()) {
            avg_peak = std::accumulate(peak_values.begin(), peak_values.end(), 0.0f) / peak_values.size();
            avg_rms = std::accumulate(rms_values.begin(), rms_values.end(), 0.0f) / rms_values.size();
        }
        std::ostringstream oss;
        oss << FG_CYAN << "Peak:" << std::fixed << std::setprecision(3) << avg_peak << " "
            << FG_MAGENTA << "RMS:" << std::fixed << std::setprecision(3) << avg_rms << RESET;
        return oss.str();
    }
};

static int audio_callback(const void* inputBuffer, void* /*outputBuffer*/, unsigned long framesPerBuffer,
                         const PaStreamCallbackTimeInfo* /*timeInfo*/, PaStreamCallbackFlags /*statusFlags*/, void* userData) {
    auto* visualizer = static_cast<AudioVisualizer*>(userData);
    const float* in = static_cast<const float*>(inputBuffer);
    if (!in) return paContinue;
    visualizer->update_stats(in, framesPerBuffer);
    std::cout << "\r";
    for (int i = 0; i < std::min(DISPLAY_SAMPLES, (int)framesPerBuffer); ++i) {
        std::cout << visualizer->get_color(in[i]) << std::showpos << std::fixed << std::setprecision(3) << in[i] << RESET << " ";
    }
    std::cout << "| " << visualizer->print_stats() << std::flush;
    return paContinue;
}

int main() {
    std::signal(SIGINT, intHandler);
    std::cout << FG_BLUE << "Raw Audio Visualizer (Ctrl+C to stop)" << RESET << std::endl;
    std::cout << "Sample Rate: " << SAMPLE_RATE << "Hz | Buffer: " << BLOCK_SIZE << " samples" << std::endl;
    std::cout << "Color Key: "
              << FG_GREEN << "Low" << RESET << "/"
              << FG_YELLOW << "Medium" << RESET << "/"
              << FG_RED << "High" << RESET << " amplitude" << std::endl;

    PaError err = Pa_Initialize();
    if (err != paNoError) {
        std::cerr << "PortAudio error: " << Pa_GetErrorText(err) << std::endl;
        return 1;
    }

    AudioVisualizer visualizer;
    PaStream* stream;
    err = Pa_OpenDefaultStream(&stream, CHANNELS, 0, paFloat32, SAMPLE_RATE, BLOCK_SIZE, audio_callback, &visualizer);
    if (err != paNoError) {
        std::cerr << "PortAudio error: " << Pa_GetErrorText(err) << std::endl;
        Pa_Terminate();
        return 1;
    }
    err = Pa_StartStream(stream);
    if (err != paNoError) {
        std::cerr << "PortAudio error: " << Pa_GetErrorText(err) << std::endl;
        Pa_CloseStream(stream);
        Pa_Terminate();
        return 1;
    }
    while (keep_running) {
        Pa_Sleep(100);
    }
    Pa_StopStream(stream);
    Pa_CloseStream(stream);
    Pa_Terminate();
    std::cout << std::endl << "Exiting..." << std::endl;
    return 0;
}
