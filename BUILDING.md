# Building SlitScanGenerator

## Requirements

- [CMake 3.30 or later](https://cmake.org/)
- [Qt 6, with a kit that matches the chosen compiler](https://www.qt.io/development/qt-framework/qt6)
- [vcpkg](https://learn.microsoft.com/de-de/vcpkg/get_started/get-started?pivots=shell-powershell)
- [NSIS 3](https://nsis.sourceforge.io/Main_Page), if you want to create the Windows installer (`makensis` must be discoverable by CMake).

The root `vcpkg.json` manifest installs CImg and FFmpeg at its pinned vcpkg registry baseline. Set `VCPKG_ROOT` to the vcpkg checkout and `QT_ROOT` to the matching Qt kit directory. The first configure builds the manifest dependencies and may take several minutes.

## Command Line

For Visual Studio 2022 with an MSVC Qt kit:

```powershell
cmake --preset vcpkg-msvc
cmake --build --preset vcpkg-msvc-release
```

For MinGW, put the matching Qt MinGW compiler and `mingw32-make` on `PATH`, then run:

```powershell
cmake --preset vcpkg-mingw
cmake --build --preset vcpkg-mingw-release
```

Use a Qt kit that matches the selected compiler. The presets use separate build directories and vcpkg triplets for MSVC and MinGW.

To build the NSIS installer after configuring, run the `package` target:

```powershell
cmake --build --preset vcpkg-msvc-release --target package
cmake --build --preset vcpkg-mingw-release --target package
```

The target builds the application, installs its runtime files and vcpkg license notices into the configured `dist/` staging directory, then invokes NSIS. The versioned setup executable is written to the repository root.

## Qt Creator

Use a Qt Creator version that supports CMake configure presets. In **Preferences/Options > Kits > CMake**, select a CMake 3.30-or-newer executable. In the Qt kit's build environment, add:

- `VCPKG_ROOT`: the path to your vcpkg checkout.
- `QT_ROOT`: the Qt kit install directory, such as `C:/Qt/6.x.x/msvc2022_64` or `C:/Qt/6.x.x/mingw_64`.

Open the repository's root `CMakeLists.txt` and choose the matching configure preset in the project configuration page: `vcpkg-msvc` for an MSVC kit or `vcpkg-mingw` for a MinGW kit. Build and run the `SlitScanGenerator` target from Qt Creator as usual. The initial configure installs and builds the manifest dependencies.

If your Qt Creator version cannot select CMake presets, configure a build with these CMake cache values instead, replacing the placeholders with absolute paths:

- `CMAKE_TOOLCHAIN_FILE`: `<VCPKG_ROOT>/scripts/buildsystems/vcpkg.cmake`
- `VCPKG_TARGET_TRIPLET`: `x64-windows` for MSVC or `x64-mingw-dynamic` for MinGW.
- `CMAKE_PREFIX_PATH`: the matching Qt kit directory (`QT_ROOT`).
- `CMAKE_BUILD_TYPE`: `Release` for a single-configuration generator. For Visual Studio, select `Release` as the build configuration.

Use a separate build directory for each compiler and triplet. The install staging directory is under `dist/` and includes the Qt version and compiler; single-configuration generators also include the build type.
