# MicF*ckery

Small projects related to RAW audio (Inputs & Ouputs) also related to microphones.

## Folder Structure
- `micthing.cpp` — C++ implementation (uses PortAudio)
- `original/micthing.py` — Python implementation (uses sounddevice, numpy, colorama)

## Requirements

### C++
- [PortAudio](http://www.portaudio.com/) (install via your package manager)

### Python
- `sounddevice`
- `numpy`
- `colorama`

Install Python dependencies with:
```bash
pip install sounddevice numpy colorama
```

## Building and Running (C++)

1. Install PortAudio:
   - **Debian/Ubuntu:** `sudo apt-get install portaudio19-dev`
   - **Arch:** `sudo pacman -S portaudio`
   - **macOS (Homebrew):** `brew install portaudio`

2. Build:
```bash
g++ micthing.cpp -o micthing -lportaudio -std=c++17
```

3. Run:
```bash
./micthing
```

## Running (Python)
```bash
python3 original/micthing.py
```

## Usage
- Press `Ctrl+C` to stop the visualizer.
- The color key is shown at startup.

## License
MIT License
