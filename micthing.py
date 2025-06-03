#!/usr/bin/env python3

# micthing.py
# A simple audio visualizer that reads from the microphone and displays
# the raw audio samples in color-coded format.

# Requires: sounddevice, numpy, colorama
import sounddevice as sd
import numpy as np
import sys
from colorama import init, Fore, Back, Style

# Initialize colorama
init()

# Configuration
SAMPLE_RATE = 44100
BLOCK_SIZE = 1024
CHANNELS = 1
DISPLAY_SAMPLES = 20  # Number of samples to show
PEAK_HISTORY = 10     # For peak detection

# Color thresholds (adjust based on your mic sensitivity)
COLOR_THRESHOLDS = {
    'low': 0.1,
    'medium': 0.3,
    'high': 0.6
}

class AudioVisualizer:
    def __init__(self):
        self.peak_values = []
        self.rms_values = []

    def get_color(self, value):
        abs_val = abs(value)
        if abs_val > COLOR_THRESHOLDS['high']:
            return Fore.RED + Style.BRIGHT
        elif abs_val > COLOR_THRESHOLDS['medium']:
            return Fore.YELLOW
        elif abs_val > COLOR_THRESHOLDS['low']:
            return Fore.GREEN
        return Fore.WHITE + Style.DIM

    def update_stats(self, buffer):
        self.peak_values.append(np.max(np.abs(buffer)))
        self.rms_values.append(np.sqrt(np.mean(buffer**2)))
        if len(self.peak_values) > PEAK_HISTORY:
            self.peak_values.pop(0)
            self.rms_values.pop(0)

    def print_stats(self):
        avg_peak = np.mean(self.peak_values) if self.peak_values else 0
        avg_rms = np.mean(self.rms_values) if self.rms_values else 0
        return (f"{Fore.CYAN}Peak:{avg_peak:.3f} "
                f"{Fore.MAGENTA}RMS:{avg_rms:.3f}")

    def callback(self, indata, frames, time, status):
        self.update_stats(indata)
        
        sys.stdout.write("\r")
        # Print colored samples
        for sample in indata[:DISPLAY_SAMPLES].flatten():
            color = self.get_color(sample)
            sys.stdout.write(f"{color}{sample:+.3f}{Style.RESET_ALL} ")
        
        # Print statistics
        sys.stdout.write(f"| {self.print_stats()}")
        sys.stdout.flush()

if __name__ == "__main__":
    print(f"{Fore.BLUE}Raw Audio Visualizer (Ctrl+C to stop){Style.RESET_ALL}")
    print(f"Sample Rate: {SAMPLE_RATE}Hz | Buffer: {BLOCK_SIZE} samples")
    print("Color Key: ", end="")
    print(f"{Fore.GREEN}Low{Style.RESET_ALL}/", end="")
    print(f"{Fore.YELLOW}Medium{Style.RESET_ALL}/", end="")
    print(f"{Fore.RED}High{Style.RESET_ALL} amplitude")
    
    visualizer = AudioVisualizer()
    with sd.InputStream(callback=visualizer.callback,
                      samplerate=SAMPLE_RATE,
                      blocksize=BLOCK_SIZE,
                      channels=CHANNELS):
        while True:
            sd.sleep(100)
