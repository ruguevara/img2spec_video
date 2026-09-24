# Image Spectrumizer 5.5

![Screenshot](img2spec2.jpg)

GUI tool for converting images to ZX Spectrum and retro-platform formats, with **video processing**, **keyframe interpolation**, and **CLI pipe mode**.

Originally by [Jari Komppa](https://github.com/jarikomppa/img2spec). Extended with video mode, keyframes, CLI pipe mode, and export pipeline by [nodeus](https://nodeus.ru).

**[Описание на русском языке / Russian description](README_ru.md)**

### Screenshots

| Main window | Keyframes UI | Export |
|-------------|-------------|--------|
| ![Main](img/main-window.png) | ![Keyframes](img/keyframes-ui.png) | ![Export](img/export-window.png) |

---

## Features

### Image Conversion

Convert images to retro-platform bitmap formats with real-time interactive preview:

| Device | Resolution | Description |
|--------|-----------|-------------|
| **ZX Spectrum** | 256x192 | Standard Speccy screen (8x8 cells, 2 colors per cell) |
| **ZX 3x64** | 256x192 | Three attribute sets, flicker-blended for ~192 colors on CRT |
| **C64 HiRes** | 320x200 | Commodore 64 high-resolution bitmap mode |
| **C64 Multicolor** | 160x200 | C64 multicolor mode (4 colors per 8x8 cell) |

### Modifiers (14 stackable, real-time)

ScalePos, Quantize, Ordered Dither, Error Diffusion Dither, Edge, Blur, Min/Max, HSV, YIQ, RGB, Contrast, Curve, Noise, SuperBlack

### Export Formats

PNG, raw binary SCR (`.scr`), C header (`.h`), assembler include (`.inc`)

### Video Mode

- Load video files (MP4, MOV, AVI, etc.) via ffmpeg
- Timeline slider with frame-by-frame navigation
- Play/pause with forward/backward skip buttons
- All modifiers apply to every frame in real time
- GUI export on Windows with NVIDIA NVENC (HEVC), AMD AMF (HEVC), or software x264 (H.264)
- Configurable quality (CRF/QP) and scale multiplier (1x-32x)
- Audio re-muxed from source after export

GUI export, progress, and cancellation are currently implemented only on Windows. On macOS, use the [Terminal video pipeline](#macos); the GUI's **Start export** button does not start an export.

### Video Keyframes

- Save full modifier + device snapshots at specific frames
- Auto-capture: any parameter change on current frame creates/updates a keyframe
- Hold semantics: settings apply from keyframe until the next one
- **Interpolation**: smooth parameter transitions between keyframes (checkbox + `--interpolate` CLI flag)
- Timeline markers (red diamonds) show keyframe positions
- Navigation buttons: `|< key`, `< key`, `> key`, `>| key`
- Sidecar storage: `<video>.keyframes.json` next to the video file
- Full snapshots: modifier stack, device type, and all options

### CLI & Pipe Mode

```
img2spec_video input.png workspace.isw -p output.png
```

| Flag | Description |
|------|-------------|
| `-p <file>` | Save PNG output |
| `-h <file>` | Save C header output |
| `-i <file>` | Save assembler include output |
| `-s <file>` | Save SCR output |
| `--pipe --width W --height H` | Read raw RGB24 frames from stdin and write RGBA frames to stdout |
| `--interpolate` | Enable keyframe interpolation in pipe mode |
| `--keys <file>` | Load keyframes for per-frame parameter switching |

`-p` writes a PNG image, not a video. `--batch-stdin` and `--headless` are not implemented. `--pipe` processes frames without opening the GUI. See the complete [macOS example](#macos) below.

### Video Export Pipeline

```
ffmpeg (decode) -> img2spec_video --pipe (process) -> ffmpeg (encode + scale)
```

- Frames pass through anonymous pipes (no disk I/O)
- img2spec_video processes at device resolution (256x192 for the default ZX Spectrum device)
- Final ffmpeg scales output to `device_resolution x scale_multiplier`
- Windows GUI export re-muxes audio after video encoding; the macOS example includes audio during encoding
- Video loading requires both ffmpeg and ffprobe in PATH; on Windows they can also be placed in the program folder

---

## Building

### CMake (cross-platform)

```bash
mkdir build && cd build
cmake ..
make
```

Dependencies: SDL2, OpenGL. On Linux: GTK3. On macOS: AppKit.

### macOS

Install the Xcode Command Line Tools (`xcode-select --install`) if needed. With Homebrew installed, run these commands from the repository root:

```bash
brew install cmake sdl2 ffmpeg
cmake -S . -B build-macos -DCMAKE_BUILD_TYPE=Release
cmake --build build-macos -j 4
./build-macos/img2spec_video
```

Launch from Terminal so the program inherits the PATH containing Homebrew's ffmpeg and ffprobe. OpenGL and AppKit come from the macOS SDK. The `build-macos/` directory is ignored by Git.

To export from Terminal, the following example resizes the input to 256x192 at 25 fps, converts it to the default ZX Spectrum format, and enlarges the result to 512x384. It uses CPU x264 and includes the first source audio stream when present. Run it from the repository root in zsh or bash, replacing `input.mp4` with your video path:

```bash
set -o pipefail
input="input.mp4"

ffmpeg -nostdin -i "$input" -map 0:v:0 \
  -vf "fps=25,scale=256:192" -f rawvideo -pix_fmt rgb24 - |
./build-macos/img2spec_video --pipe --width 256 --height 192 |
ffmpeg -nostdin -f rawvideo -pix_fmt rgba \
  -s 256x192 -framerate 25 -i - -i "$input" \
  -map 0:v:0 -map '1:a:0?' \
  -vf "scale=512:384:flags=neighbor" \
  -c:v libx264 -crf 17 -pix_fmt yuv420p \
  -c:a aac -shortest output.mp4
```

For saved modifiers, add `workspace.isw` before `--pipe`. Set `--width` and `--height` to the decoder's output size and the final ffmpeg `-s` to the workspace's device resolution. To reproduce settings made against the original video, remove the decoder's `-vf "fps=25,scale=256:192"`, use the original decoded frame dimensions, and set `-framerate` to the source frame rate (for example, `24000/1001`). This also preserves the frame numbering expected by `--keys "input.mp4.keyframes.json"`; add `--interpolate` to interpolate keyframes. Keep the device resolution constant throughout the export.

### Visual Studio

Open `img2spectrum.vcxproj`. v120 toolset (VS2013). Win32 and x64 configs. SDL2 expected at `\libraries\sdl2\`.

### MinGW cross-compile (Linux to Windows)

```bash
./build_w32.sh   # i686, static
./build_w64.sh   # x86_64, static
```

---

## Libraries & Licenses

| Library | License | URL |
|---------|---------|-----|
| **img2spec** | zlib/libpng | https://github.com/jarikomppa/img2spec |
| **SDL2** | zlib | https://www.libsdl.org/ |
| **Dear ImGui** | MIT | https://github.com/ocornut/imgui |
| **Parson** | MIT | https://github.com/kgabis/parson |
| **stb libraries** | Public Domain | https://github.com/nothings/stb |
| **ffmpeg** | GPL/LGPL | https://ffmpeg.org/ |

---

## Links

- Original project: https://github.com/jarikomppa/img2spec
- Fork (video + keyframes + CLI): https://github.com/nodeus/img2spec_video
- Author: https://nodeus.ru
