# Ghoul

![Build Status](http://dev.openspaceproject.com/buildStatus/icon?job=OpenSpace%2FGhoul%2Fmaster&style=flat-square)
[![GitHub Issues](https://img.shields.io/github/issues/OpenSpace/Ghoul.svg)](https://github.com/OpenSpace/Ghoul/issues)
[![Average time to resolve an issue](http://isitmaintained.com/badge/resolution/OpenSpace/Ghoul.svg)](http://isitmaintained.com/project/OpenSpace/Ghoul "Average time to resolve an issue")

## Dependencies

All third-party dependencies are provided by [vcpkg](https://vcpkg.io) and are declared in
`vcpkg.json`. Three dependencies are built from the overlay ports in
`support/vcpkg/ports`, which `vcpkg-configuration.json` registers automatically:
`websocketpp` and `tiny-process-library` track the OpenSpace forks, and `glbinding` pins
3.5.0 because the version in the vcpkg registry declares `glClampColor` with the wrong
parameter type. The `GHOUL_MODULE_*` options are translated into vcpkg manifest features,
so disabling a module also stops its dependency from being built.

## Building standalone

Set `VCPKG_ROOT` to a vcpkg checkout and use one of the presets:

```
cmake --preset windows      # or linux / macos
cmake --build --preset windows
ctest --preset windows
```

## Consuming Ghoul as an external project

`cmake --install` produces a package that can be consumed with:

```cmake
find_package(Ghoul CONFIG REQUIRED)
target_link_libraries(my_app PRIVATE Ghoul::Ghoul)
```

`support/consumer-test` is a minimal project that exercises this path and is used as a
regression test for the install and export rules.

## Consuming Ghoul as a subproject

vcpkg only reads the manifest of the top-level project, so a superproject that adds Ghoul
through `add_subdirectory` has to declare Ghoul's dependencies itself. Depend on the
`ghoul-deps` meta-port to do that with a single entry:

```jsonc
// vcpkg.json of the superproject
{
  "dependencies": [
    { "name": "ghoul-deps", "features": [ "assimp", "fontrendering", "lua", "opengl" ] }
  ],
  "overrides": [ { "name": "lua", "version": "5.4.7" } ]
}
```

```jsonc
// vcpkg-configuration.json of the superproject
{ "overlay-ports": [ "./ext/ghoul/support/vcpkg/ports" ] }
```

The `overrides` have to be repeated because a port cannot express them. `ghoul-deps` is
kept in sync with `vcpkg.json` by `support/vcpkg/check-manifest-sync.cmake`, which runs as
part of the test suite.

