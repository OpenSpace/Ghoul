# Ghoul

![Build Status](http://dev.openspaceproject.com/buildStatus/icon?job=OpenSpace%2FGhoul%2Fmaster&style=flat-square)
[![GitHub Issues](https://img.shields.io/github/issues/OpenSpace/Ghoul.svg)](https://github.com/OpenSpace/Ghoul/issues)
[![Average time to resolve an issue](http://isitmaintained.com/badge/resolution/OpenSpace/Ghoul.svg)](http://isitmaintained.com/project/OpenSpace/Ghoul "Average time to resolve an issue")

## Dependencies

Ghoul resolves its dependencies through [vcpkg](https://vcpkg.io) in manifest mode, so the
only prerequisites are a C++ compiler, CMake 4.0 or newer, and a vcpkg checkout. The
dependencies are declared in `vcpkg.json`; the embedded `vcpkg-configuration` block pins
the registry baseline and registers the overlay ports under `support/vcpkg/ports`.

Three dependencies are built from those overlay ports: `websocketpp` and
`tiny-process-library` track the OpenSpace forks, and `glbinding` pins 3.5.0 because the
version in the vcpkg registry declares `glClampColor` with the wrong parameter type. The
`GHOUL_MODULE_*` options are translated into vcpkg manifest features, so disabling a module
also stops its dependency from being built.

`stb` and RenderDoc stay vendored under `ext/`, because each is a single translation unit
or header that has to be compiled with Ghoul-specific settings.

## Building standalone

Point `VCPKG_ROOT` at your vcpkg checkout and use one of the CMake presets. The first
configure downloads and builds the dependencies, which takes a while; subsequent runs are
served from vcpkg's binary cache.

```
cmake --preset windows        # or linux
cmake --build --preset windows
ctest --preset windows
```

The presets also offer `windows-debug`, `windows-release`, `linux-debug`, and
`linux-release`, plus `windows-static` for linking the CRT statically as well. The `windows`
triplet is `x64-windows-static-md` rather than the fully dynamic `x64-windows` that SGCT
uses, because the `glbinding` overlay port only supports static linkage. `ctest` runs the
unit tests and `VcpkgManifestSync`, which checks that the root manifest and the `ghoul`
port stay in step.

## Consuming Ghoul

A vcpkg port lives in `support/vcpkg/ports/ghoul`. Register it as an overlay from your own
project's manifest:

```jsonc
// vcpkg.json
{
  "dependencies": [
    { "name": "ghoul", "features": [ "assimp", "fontrendering", "lua", "opengl" ] }
  ],
  "overrides": [ { "name": "lua", "version": "5.4.7" } ],
  "vcpkg-configuration": {
    "overlay-ports": [ "./ext/ghoul/support/vcpkg/ports" ]
  }
}
```

The `overrides` have to be repeated because a port cannot express them. Then link against
Ghoul:

```cmake
find_package(ghoul CONFIG REQUIRED)
target_link_libraries(my_app PRIVATE Ghoul::Ghoul)
```

`support/consumer-test` is a minimal project that exercises this path and is used as a
regression test for the port and the install and export rules.

Ghoul can still be added through `add_subdirectory` (`GHOUL_HAVE_TESTS` and
`GHOUL_ENABLE_INSTALL` default off when Ghoul is not the top-level project), but the
superproject then has to provide Ghoul's dependencies itself, since vcpkg only reads the
top-level manifest. Depending on the `ghoul` port is the supported way to do that.
