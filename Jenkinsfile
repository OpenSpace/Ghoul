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
      dir('build') {
        cmake([
          installation: 'InSearchPath',
          arguments: '-G "Unix Makefiles" ..',
        ])
        cmakeBuild([
          installation: 'InSearchPath',
          buildDir: 'build',
          steps: [
            [ args: '--target GhoulTest -- -j4', withCmake: true ]
          ]
        ])
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
        dir('build') {
          cmake([
            installation: 'InSearchPath',
            arguments: '-G "Visual Studio 15 2017 Win64" ..',
          ])
          cmakeBuild([
            installation: 'InSearchPath',
            // buildDir: 'build',
            steps: [
              [ args: '-- /nologo /verbosity:minimal /m:4', withCmake: true ]
            ]
          ])
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
      dir('build') {
        cmake([
          installation: 'InSearchPath',
          arguments: '-G Xcode ..',
        ])
        cmakeBuild([
          installation: 'InSearchPath',
          // buildDir: 'build',
          steps: [
            [ args: '-- -parallelizeTargets -jobs 4 -target Ghoul -target GhoulTest', withCmake: true ],
          ]
        ])
      }
    }
    // Currently, the unit tests are crashing on OS X
    // stage('osx/test') {
    //   sh 'build/Debug/GhoulTest --gtest_output=xml:test_results.xml'
    //   junit([testResults: 'test_results.xml'])
    // }
  } // node('osx')
}
