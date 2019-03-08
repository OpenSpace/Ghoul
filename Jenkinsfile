parallel linux: {
  node('linux') {
    stage('linux/SCM') {
      deleteDir()
      checkout scm
      sh 'git submodule update --init --recursive'
    }
    stage('linux/Build') {
      cmakeBuild([
        generator: 'Unix Makefiles',
        buildDir: 'build',
        installation: 'InSearchPath',
        steps: [
          [ args: '--target Ghoul -- -j4', withCmake: true ]
          // [ args: '--target Ghoul --target GhoulTest -- -j4', withCmake: true ]
        ]
      ])
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
        cmakeBuild([
          generator: 'Visual Studio 15 2017 Win64',
          buildDir: 'build',
          installation: 'InSearchPath',
          steps: [
            [ args: '-- /nologo /verbosity:minimal /m:4', withCmake: true ]
          ]
        ])
      }
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
      cmakeBuild([
        generator: 'Xcode',
        buildDir: 'build',
        installation: 'InSearchPath',
        steps: [
          [ args: '-- -parallelizeTargets -jobs 4 -target Ghoul -target GhoulTest', withCmake: true ],
        ]
      ])
    }
  } // node('osx')
}
