# Build and Verification

## Windows / Visual Studio Build Tools

Use the Visual Studio bundled CMake in this workspace:

```powershell
$cmake = 'C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe'
$ctest = 'C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\ctest.exe'
$env:PATH = $env:Path
Remove-Item Env:Path -ErrorAction SilentlyContinue
& $cmake -S . -B build-codex-clean
& $cmake --build build-codex-clean --config Debug
& $ctest --test-dir build-codex-clean -C Debug --output-on-failure
```

The game executable is produced at:

```text
build-codex-clean\Debug\BunkerProtocolISO.exe
```

## MapEditor

```powershell
& $cmake -S tools/MapEditor -B build-codex-mapeditor-vs -G "Visual Studio 17 2022" -A x64
& $cmake --build build-codex-mapeditor-vs --config Debug
```

## Notes

- The repository expects bundled SFML under `source/SFML-3.1.0`.
- Local generated dependency/build files stay ignored by Git.
- `smoke_assets` checks that the executable exists and that shader assets were copied beside it.
