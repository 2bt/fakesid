# Fake SID - Claude Code Context

This is a chiptune tracker app with shared C++ core logic and platform-specific UI layers.

## Project Overview

**What it is:** A music tracker application for creating Commodore 64-style chiptune music using SID chip emulation.

**Platforms:**
- Linux desktop (primary development platform)
- Android (published on Play Store)

**Architecture:** C++ core with platform-specific UI layers
- Desktop: SDL2 + OpenGL
- Android: GLSurfaceView + JNI bridge to C++

## Project Structure

```
src/                    # Shared C++ source code
  ├── resid-0.16/      # SID chip emulation library
  ├── *.cpp/*.hpp      # Core tracker logic
android/               # Android app project
  ├── app/
  │   ├── src/main/
  │   │   ├── cpp/     # Android-specific C++ code (JNI glue)
  │   │   ├── java/    # Java activity and view classes
  │   │   └── res/     # Android resources
  │   └── build.gradle.kts
lib/                   # Third-party dependencies (glm, libsndfile)
docs/                  # User documentation
```

## Build Systems

### Linux Desktop
- **Build tool:** CMake
- **Dependencies:** GLEW, SDL2, libsndfile (found via pkg-config)
- **Build command:** `mkdir build && cd build && cmake .. && make`
- **Output:** `build/fakesid` executable

### Android
- **Build tool:** Gradle with Kotlin DSL
- **Native build:** CMake (invoked by Gradle)
- **Dependencies:**
  - `com.google.oboe:oboe` - Audio output
  - `libsndfile` - Bundled in `lib/` folder
- **Build:** Open `android/` in Android Studio or `./gradlew assembleDebug`
- **Key files:**
  - `android/app/build.gradle.kts` - App build config
  - `android/app/src/main/CMakeLists.txt` - Native C++ build
  - `android/gradle/libs.versions.toml` - Dependency versions

## Code Architecture

### C++ Core (src/)
- **main.cpp** - Entry point (desktop) or JNI initialization (Android)
- **sid_engine.cpp** - SID chip emulation (wrapper around resid-0.16)
- **player.cpp** - Music playback engine
- **song.cpp/song.hpp** - Song data structures
- **gui.cpp** - Main UI logic
- **gfx.cpp** - OpenGL rendering
- **app.cpp, edit.cpp, settings.cpp** - Various app features
- ***_view.cpp** - Different UI screens (track, song, project, jam, instrument effect)

### Android Java Layer
- **MainActivity.java** - Activity lifecycle, permissions, keyboard control
- **MainView.java** - GLSurfaceView wrapper, touch input handling
- **Lib.java** - JNI native method declarations

### JNI Bridge
The Java layer calls into C++ via these native methods (defined in `Lib.java`):
- `init(AssetManager, String storageDir)` - Initialize C++ with Android assets and storage directory
- `resize(int, int)` - Handle surface resize
- `draw()` - Render OpenGL frame
- `touch(int, int, int)` - Forward touch events to C++
- `key(int, int)` - Forward key events to C++
- `startAudio()/stopAudio()` - Audio lifecycle
- `setInsets(int, int)` - Handle system insets (status bar, navigation bar)
- `getSettingName(int)/getSettingValue(int)/setSettingValue(int, int)` - Settings management
- `importSong(String path)` - Import song from SAF-provided path
- `exportSong(String path, String title)` - Export song (currently unused, export handled via platform layer)

## Important Implementation Notes

### Audio
- **Desktop:** Uses SDL2 audio
- **Android:** Uses Oboe library for low-latency audio
- Audio runs on separate thread, coordinated by C++ layer

### Rendering
- Both platforms use OpenGL (desktop: full GL, Android: ES 2)
- `stb_image.h` is used for texture loading
- Custom immediate-mode style UI in `gfx.cpp`

### File Storage
- **Desktop:** Direct file I/O using libsndfile
- **Android:** Storage Access Framework (SAF) - fully implemented
  - **Storage Location:** `getExternalFilesDir()` - `/storage/emulated/0/Android/data/com.twobit.fakesid/files/`
  - **Song Import:** SAF file picker (`ACTION_OPEN_DOCUMENT`) → copies to cache → loads into app
  - **Song Export:** Renders to app storage → shares via FileProvider → user chooses destination
  - **WAV/OGG Export:** Renders to app storage → shares via FileProvider → user chooses destination
  - **No permissions required** - Uses SAF for all user-facing file operations
  - **FileProvider configured** in `AndroidManifest.xml` with `res/xml/file_paths.xml`

### Dependencies
- **resid-0.16:** SID chip emulation (kb's TinySID)
- **libsndfile:** Sound file I/O (WAV output)
- **glm:** Math library (header-only)
- **oboe:** Android audio only

## Common Tasks

### Building for Linux
```bash
mkdir -p build && cd build
cmake ..
make
./fakesid
```

### Building for Android
```bash
cd android
./gradlew assembleDebug
# Or open in Android Studio and click "Run"
```

### Adding a new native dependency
1. For desktop: Add to CMakeLists.txt `pkg_search_module()`
2. For Android: Add to `android/app/build.gradle.kts` or use Prefab (if available)
3. Update both `CMakeLists.txt` files if needed

### Debugging JNI issues
- Check `MainActivity.java` and `Lib.java` for method signatures
- JNI implementations are in C++ files (look for `Java_com_twobit_fakesid_...` functions)
- Use `adb logcat` to see native crashes

## Known Issues

1. **No CI/CD** - Manual builds only
2. **Song file export** - Can save .sng files to app storage, but no UI button to export/share them yet (only WAV/OGG export implemented)

## Recent Changes

- Migrated to Storage Access Framework (SAF) for file operations
- Removed storage permissions, files now user-accessible via FileProvider
- Migrated Android build from Groovy to Kotlin DSL
- Added edge-to-edge display support
- Updated to targetSdk/compileSdk 34 with AndroidX libraries

