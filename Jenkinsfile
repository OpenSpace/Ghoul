def createDirectory(String dir) {
  cmake([installation: 'InSearchPath', arguments: "-E make_directory ${dir}"])
}

def runCMake(String generator) {
  cmake([ installation: 'InSearchPath', arguments: "-G \"${generator}\" .." ])
}

def build(String compileOptions) {
  cmakeBuild([
    installation: 'InSearchPath',
    steps: [
      [ args: "-- ${compileOptions}", withCmake: true ]
    ]
  ])
}

// Map defaultCMakeOptions() {
//   return [

//   ]
// }

parallel linux: {
  node('linux') {
    stage('linux/SCM') {
      deleteDir()
      checkout scm
      sh 'git submodule update --init --recursive'
    }
    stage('linux/Build') {
      createDirectory('build')
      dir('build') {
        runCMake('Unix Makefiles')
        build('-j4')
      }
    }
    stage('linux/test') {
      sh 'build/GhoulTest --gtest_output=xml:test_results.xml'
      junit([testResults: 'test_results.xml'])
    }
  } // node('linux')
},
windows: {
  node('windows') {
    // We specify the workspace directory manually to reduce the path length and thus try to avoid MSB3491 on Visual Studio
    ws("${env.JENKINS_BASE}/G/${env.BRANCH_NAME}/${env.BUILD_ID}") {
      stage('windows/SCM') {
        deleteDir()
        checkout scm
        bat 'git submodule update --init --recursive'
      }
      stage('windows/Build') {
        createDirectory('build')
        dir('build') {
          runCMake('Visual Studio 15 2017 Win64')
          build('/nologo /verbosity:minimal /m:4')
        }
      }
      // Currently, the unit tests are failing on Windows
      // stage('windows/test') {
      //   bat 'build\\Debug\\GhoulTest --gtest_output=xml:test_results.xml'
      //   junit([testResults: 'test_results.xml'])
      // }
    }
  } // node('windows')
},
osx: {
  node('osx') {
    stage('osx/SCM') {
      deleteDir()
      checkout scm
      sh 'git submodule update --init --recursive'
    }
    stage('osx/Build') {
      createDirectory('build')
      dir('build') {
        runCMake('Xcode')
        build('-parallelizeTargets -jobs 4')
      }
    }
    // Currently, the unit tests are crashing on OS X
    // stage('osx/test') {
    //   sh 'build/Debug/GhoulTest --gtest_output=xml:test_results.xml'
    //   junit([testResults: 'test_results.xml'])
    // }
  } // node('osx')
}
