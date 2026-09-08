// The build relies on vcpkg in manifest mode. Each agent needs VCPKG_ROOT pointing at a
// vcpkg checkout; vcpkg's default binary cache lives outside the workspace and therefore
// survives cleanWs(). Set VCPKG_BINARY_SOURCES on the agent to share a cache between
// machines.

def checkoutGit() {
  checkout scm;

  // support/cmake/common-compile-settings is the only remaining submodule; every other
  // dependency now comes from vcpkg
  if (isUnix()) {
    sh(
      script: "git submodule update --init",
      label: "Init submodules"
    )
  }
  else {
    bat(
      script: "git submodule update --init",
      label: "Init submodules"
    )
  }
}

def buildWithPreset(preset) {
  def script = """
  cmake --preset ${preset}
  cmake --build --preset ${preset} --parallel 4
  """

  if (isUnix()) {
    sh(script: script, label: "Configure and build (${preset})")
  }
  else {
    bat(script: script, label: "Configure and build (${preset})")
  }
}

def runUnitTests(preset) {
  def script = "ctest --preset ${preset} --output-junit test_results.xml"

  if (isUnix()) {
    sh(script: script, label: "Run unit tests (${preset})")
  }
  else {
    bat(script: script, label: "Run unit tests (${preset})")
  }
  junit([testResults: "build/${preset}/test_results.xml"])
}

parallel tools: {
  node('tools') {
    stage('tools/scm') {
      deleteDir();
      checkoutGit();
    }
    stage('tools/cppcheck/run') {
      sh(
        script: 'cppcheck --enable=all --xml --xml-version=2 --suppressions-list=support/cppcheck/suppressions.txt -i ext -i support include src tests 2> cppcheck.xml',
        label: 'CPPCheck'
      )
    }
    stage("tools/cppcheck/record") {
      recordIssues(
        id: 'tools-cppcheck',
        tool: cppCheck(pattern: 'cppcheck.xml')
      )
    }
    cleanWs()
  } // node('tools')
},
linux_gcc: {
  if (env.USE_BUILD_OS_LINUX == 'true') {
    node('linux-gcc') {
      stage('linux-gcc/scm') {
        deleteDir();
        checkoutGit();
      }
      stage('linux-gcc/build') {
        buildWithPreset('linux');
        recordIssues(
          id: 'linux-gcc',
          tool: gcc()
        )
      }
      stage('linux-gcc/test') {
        runUnitTests('linux')
      }
      cleanWs()
    } // node('linux-gcc')
  }
},
linux_clang: {
  if (env.USE_BUILD_OS_LINUX == 'true') {
    node('linux-clang') {
      stage('linux-clang/scm') {
        deleteDir();
        checkoutGit();
      }
      stage('linux-clang/build') {
        buildWithPreset('linux');
        recordIssues(
          id: 'linux-clang',
          tool: clang()
        )
      }
      stage('linux-clang/test') {
        runUnitTests('linux')
      }
      cleanWs()
    } // node('linux-clang')
  }
},
windows_msvc: {
  if (env.USE_BUILD_OS_WINDOWS == 'true') {
    node('windows') {
      stage('windows-msvc/scm') {
        deleteDir();
        checkoutGit();
      }
      stage('windows-msvc/build') {
        buildWithPreset('windows');
        recordIssues(
          id: 'windows-msbuild-msvc',
          tool: msBuild()
        )
      }
      stage('windows-msvc/test') {
        runUnitTests('windows')
      }
      cleanWs()
    } // node('windows')
  }
}
