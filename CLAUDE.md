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
- `init(AssetManager)` - Initialize C++ with Android assets
- `resize(int, int)` - Handle surface resize
- `draw()` - Render OpenGL frame
- `touch(int, int, int)` - Forward touch events to C++
- `key(int, int)` - Forward key events to C++
- `startAudio()/stopAudio()` - Audio lifecycle
- `saveSettings()` - Persist preferences

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
- **Android:** NOT YET WORKING - needs to be updated for scoped storage (Android 10+)
  - Old code used `WRITE_EXTERNAL_STORAGE` permission
  - Needs migration to app-specific storage or Storage Access Framework

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

1. **Android file I/O broken** - Song load/save doesn't work due to scoped storage restrictions
2. **No CI/CD** - Manual builds only
3. **Old gradle files removed** - Migration to Kotlin DSL completed (Dec 2024)


## Planned Migration: File Storage Modernization

**Status:** Planned, not yet started

**Goal:** Migrate from broken `WRITE_EXTERNAL_STORAGE` approach to Storage Access Framework (SAF) like GTMobile.

**Reference Implementation:** `/home/dlangner/Programming/c++/GTMobile`

### Migration Strategy

**Phase 1: Add Migration Logic (First Update)**
- Detect old files from legacy storage
- Copy `.fsid` files to app-specific storage
- Show migration dialog to users
- Request legacy permission one-time only for migration
- Mark migration complete in SharedPreferences

**Phase 2: Implement GTMobile-style File Handling**

**Java Layer Changes:**
- Add `FileUtils.java` class (copy from GTMobile)
- Add `startSongImport()` method - file picker via `ACTION_OPEN_DOCUMENT`
- Add `exportSong()` method - share intent with FileProvider
- Add migration helper methods
- Add FileProvider configuration to AndroidManifest.xml

**C++ Layer Changes:**
- Use cache directory for imported files
- Add export function to save to app-specific directory
- Keep song data in memory during editing (current behavior)

**Manifest Changes:**
- Add FileProvider authority
- Remove `WRITE_EXTERNAL_STORAGE` permission
- Update targetSdk to match (currently 34)

**User Experience Flow:**
- **Existing users:** One-time migration prompt → copy old songs → new SAF workflow
- **New users:** SAF from the start

**Benefits:**
- ✅ No storage permissions required
- ✅ Works on all Android versions including Android 10+
- ✅ User controls where files go
- ✅ No file access loss on app updates
- ✅ Backward compatible with existing users' songs

## Recent Changes (Dec 2024 - Jan 2025)

- Migrated Android build from Groovy to Kotlin DSL
- Removed unused Jetpack Compose dependencies
- Updated to targetSdk/compileSdk 34 with AndroidX libraries
- Restored original PNG app icons
- Fixed build configuration for modern Android Studio
