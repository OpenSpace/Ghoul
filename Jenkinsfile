// Header

// Checks out Ghoul from Git with recursive submodules
// unfortunately, as of now (2019-03-12), there is no good way of specifying recursive
// submodules directly to the git Jenkins plugin
def checkoutGit() {
  checkout([
    $class: 'GitSCM',
    branches: [[name: "${env.BRANCH_NAME}"]],
    doGenerateSubmoduleConfigurations: false,
    extensions: [[
      $class: 'SubmoduleOption',
      disableSubmodules: false,
      parentCredentials: false,
      recursiveSubmodules: true,
      reference: '',
      trackingSubmodules: false
    ]],
    submoduleCfg: [],
    userRemoteConfigs: [[
      url: 'https://github.com/OpenSpace/Ghoul'
    ]]
  ])
}

// Create the specified directory
def createDirectory(String dir) {
  cmake([installation: 'InSearchPath', arguments: "-E make_directory ${dir}"])
}

// Run the initial CMake for the specified generator
def runCMake(String generator) {
  cmake([ installation: 'InSearchPath', arguments: "-G \"${generator}\" .." ])
}

// Build all targets by calling through the cmakeBuild tool
def build(String compileOptions) {
  cmakeBuild([
    installation: 'InSearchPath',
    steps: [
      [ args: "-- ${compileOptions}", withCmake: true ]
    ]
  ])
}

// Runs the provided binary path (GhoulTest) and analyses the results using junit
def runTests(bin) {
  if (isUnix()) {
    sh "${bin} --gtest_output=xml:test_results.xml"
  }
  else {
    bat "${bin} --gtest_output=xml:test_results.xml"
  }
  junit([testResults: 'test_results.xml'])
}



//
// Pipeline start
//
parallel linux: {
  node('linux') {
    stage('linux/SCM') {
      deleteDir()
      checkoutGit()
    }
    stage('linux/Build') {
      cmakeBuild([
        installation: 'InSearchPath',
        generator: 'Unix Makefiles',
        buildDir: 'build',
        steps: [[args: '-j4', withCmake: true]]
      ])
      // createDirectory('build')
      // dir('build') {
      //   runCMake('Unix Makefiles')
      //   build('-j4')
      // }
    }
    stage('linux/test') {
      runTests('build/GhoulTest')
    }
  } // node('linux')
},
windows: {
  node('windows') {
    // We specify the workspace directory manually to reduce the path length and thus try to avoid MSB3491 on Visual Studio
    ws("${env.JENKINS_BASE}/G/${env.BRANCH_NAME}/${env.BUILD_ID}") {
      stage('windows/SCM') {
        deleteDir()
        checkoutGit()
      }
      stage('windows/Build') {
        cmakeBuild([
          installation: 'InSearchPath',
          generator: 'Visual Studio 15 2017 Win64',
          buildDir: 'build',
          steps: [[args: '/nologo /verbosity:minimal /m:4', withCmake: true]]
        ])

        // createDirectory('build')
        // dir('build') {
        //   runCMake('Visual Studio 15 2017 Win64')
        //   build('/nologo /verbosity:minimal /m:4')
        // }
      }
      stage('windows/test') {
        // Currently, the unit tests are failing on Windows
        // runTests('build\\Debug\\GhoulTest')
      }
    }
  } // node('windows')
},
osx: {
  node('osx') {
    stage('osx/SCM') {
      deleteDir()
      checkoutGit()
    }
    stage('osx/Build') {
      cmakeBuild([
        installation: 'InSearchPath',
        generator: 'Xcode',
        buildDir: 'build',
        steps: [[args: '-parallelizeTargets -jobs 4', withCmake: true]]
      ])

      // createDirectory('build')
      // dir('build') {
      //   runCMake('Xcode')
      //   build('-parallelizeTargets -jobs 4')
      // }
    }
    stage('osx/test') {
      // Currently, the unit tests are crashing on OS X
      // runTests('build/Debug/GhoulTest')
    }
  } // node('osx')
}
