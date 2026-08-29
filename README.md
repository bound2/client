# client

If you're a player, rather than a developer, just follow the [单机版教程](./standalone_version.md)

TL;DR

## Build on Windows (from scratch)

The current build is **CMake + SDL2 + Visual Studio 2022**. The old DirectX code
has been migrated to SDL2, so no DirectX SDK is needed any more.

### 1. Prerequisites

Install **Visual Studio 2022** with the *Desktop development with C++* workload.
That gives you the MSVC toolset (v143) and a Windows 10/11 SDK.

You also need the **C++ ATL** individual component, which that workload does
*not* install by default. `VS_UI/src/header/VS_UI_WebBrowser.h` includes
`atlbase.h` for the legacy in-game IE browser control, and those sources are
only excluded on non-Windows builds — so on Windows ATL is mandatory. In the
Visual Studio Installer: *Modify → Individual components → "C++ ATL for latest
v143 build tools (x86 & x64)"*. Or from an **elevated** terminal (`--quiet`
silently refuses with exit code 5007 if the shell isn't already elevated — it
does not prompt for UAC):

```powershell
& "C:\Program Files (x86)\Microsoft Visual Studio\Installer\setup.exe" modify --installPath "C:\Program Files\Microsoft Visual Studio\2022\Community" --add Microsoft.VisualStudio.Component.VC.ATL --quiet --norestart
```

Close Visual Studio first. The installer aborts with exit code 8006
(`Pre-check verification failed with warning(s) : VSProcessesRunning`) if
anything VS-related is alive — including idle `MSBuild.exe` worker nodes, which
linger for ~15 minutes after a `cmake --build ... -- -m` run. `taskkill /F /IM
MSBuild.exe` clears those.

Verify it landed before building — this should print an install path, not
nothing:

```powershell
& "C:\Program Files (x86)\Microsoft Visual Studio\Installer\vswhere.exe" -products * -requires Microsoft.VisualStudio.Component.VC.ATL -property installationPath
```

Install CMake 3.20+ and reopen your terminal so it lands on `PATH`:

```bash
winget install Kitware.CMake
```

### 2. Install the dependencies with vcpkg

`CMakeLists.txt` calls `find_package(SDL2 REQUIRED)`, so configure will fail
until these exist. Clone and bootstrap vcpkg:

```bash
git clone https://github.com/microsoft/vcpkg.git C:\vcpkg
```

```bash
C:\vcpkg\bootstrap-vcpkg.bat
```

Then install the libraries (this compiles from source and takes a while):

```bash
C:\vcpkg\vcpkg install sdl2:x64-windows sdl2-image:x64-windows sdl2-ttf:x64-windows sdl2-mixer:x64-windows libiconv:x64-windows libjpeg-turbo:x64-windows
```

| Package | Used for |
| --- | --- |
| `sdl2`, `sdl2-image`, `sdl2-ttf` | rendering, sprites, text (required) |
| `sdl2-mixer` | sound and music |
| `libiconv` | encoding conversion in `MString.cpp` / `TextService.cpp` |
| `libjpeg-turbo` | JPG load/save in `UtilityFunction.cpp` |

vcpkg can live anywhere — just point `-DCMAKE_TOOLCHAIN_FILE` at wherever you
put it in the next step. `vcpkg integrate install` is optional.

### 3. Generate the solution

The toolchain file must be passed on the **first** configure, or vcpkg's
packages won't be found:

```bash
cmake -S . -B build/vs2022 -G "Visual Studio 17 2022" -A x64 -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake
```

### 4. Build

```bash
cmake --build build/vs2022 --config Debug
```

Or open `build/vs2022/DarkEdenClient.sln` in Visual Studio, set **DarkEden** as
the startup project, and build from there.

The executable lands in `build/vs2022/bin/Debug/DarkEden.exe`.

### Never commit `build/`

CMake bakes **absolute paths** into the generated `.sln`/`.vcxproj` files, along
with the exact Windows SDK version and toolset of the machine that generated
them. Those files are not portable between clones, which is why `build/` is
gitignored. Always regenerate locally.

### Troubleshooting

**`The Windows SDK version 10.0.xxxxx.0 was not found`** or
**`The referenced project <some other path>\ZERO_CHECK.vcxproj does not exist`**

You're opening a solution that was generated on a different machine. "Retarget
solution" won't fix it — it only rewrites the SDK and toolset, leaving the
foreign absolute paths behind. Delete the stale build tree and regenerate with
step 3:

```powershell
Remove-Item -Recurse -Force build\vs2022
```

**`find_package(SDL2 REQUIRED)` fails / configure stops at `CMakeLists.txt:90`**

Either vcpkg packages are missing (step 2), or the toolchain file wasn't passed
on the first configure. Delete `build/vs2022` and redo step 3 — adding the
toolchain to an existing cache does not work.

**`fatal error C1083: Cannot open include file: 'ATLBASE.H'`**

The C++ ATL component is missing — see step 1. CMake locates `atlmfc/include` by
globbing installed VS instances at **configure** time, so after installing ATL
you must re-run step 3 (an incremental build alone won't pick it up).

**Link errors mentioning `iconv.lib`**

`libiconv:x64-windows` isn't installed. See step 2.

**`LNK1104: cannot open file 'atls.lib'`**

Same cause as the `ATLBASE.H` error above — ATL headers embed a
`#pragma comment(lib, "atls.lib")`. Install the ATL component and re-configure.

## Legacy VC6 build (deprecated)

The original VC6 + DirectX 9 build. Kept for reference only — use the CMake
build above.

Get the xerces-c lib 3.2.3, unzip it to the client project root path.
You can get it from the internet or here is a [mirror file](https://github.com/opendarkeden/client/raw/data/xerces-c-3.2.3.zip).

The directory then looks like this:

```
$ ls -lah
total 202K
drwxr-xr-x 1 genius 197121   0 Jun  9 10:21 ./
drwxr-xr-x 1 genius 197121   0 Jun  9 09:53 ../
drwxr-xr-x 1 genius 197121   0 Jun  9 10:21 .git/
-rw-r--r-- 1 genius 197121 384 Jun  9 10:13 .gitignore
...
drwxr-xr-x 1 genius 197121   0 Oct 10  2015 xerces/
```

Get the directx9 lib, it's staled and hard to find, here is a [mirror file](https://github.com/opendarkeden/client/raw/data/dx90bsdk.zip).
Extract it to someplace, and add it to the VC6 Include & Library directory:

![image](https://user-images.githubusercontent.com/1420062/121283362-8949b900-c90d-11eb-8a7e-eeac6eb4135b.png)
![image](https://user-images.githubusercontent.com/1420062/121283745-3a505380-c90e-11eb-91cc-2c6ecfd76479.png)

You can put it to any path, not necessarily Game(800)


Open workspace client/Client/Client.dsw in VC6, everything should work.

## run

Download the Data files from [Mediafire](https://www.mediafire.com/file/017bif66kyieviw/DARKEDEN.zip/file) or [Baidu Netdisk](https://pan.baidu.com/s/1-DufSEmnydMbOtTwOo_h8A) (extract code 6bcl), extract it to `client/DARKEDEN`, run "window.bat"

Modify the IP/Port in `DARKEDEN/Data/Info/GameClient.inf` to your own login server IP/Port.
If you are using docker on the local Windows system, the IP should be `127.0.0.1`.

You can see "window.bat" is a simple script that run "fengshen.exe" with a argument:

```
fengshen.exe 0000000001 
```

The meaning of the argument:

- 0000000001 windowmode
- 0000000002 fullscreen
- 0000000003 windowmode 1024
- 0000000004 fullscreen 1024

For the CMake build the executable is `DarkEden.exe` instead, and it takes the
same argument. Its working directory is set to its own folder, so the `Data/`
directory must sit next to it in `build/vs2022/bin/Debug/`.

You need a [server](https://github.com/opendarkeden/server) to play the game.
You may follow the [docker install guide](https://github.com/opendarkeden/server/blob/master/docker_install.md) to deploy server.
