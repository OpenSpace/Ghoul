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
          [ args: '-j4 --target Ghoul', withCmake: true, ]
          // [ args: '-j4 --target GhoulTest', withCmake: true, ]
        ]
      ])
    }
  } // node('linux')
},
windows: {
  node('windows') {
    // We specify the workspace directory manually to reduce the path length and thus try to avoid MSB3491 on Visual Studio
    ws("${env.JENKINS_BASE}/G/${env.BRANCH_NAME}") {
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
            [ args: 'Ghoul.sln /nologo /verbosity:minimal /m:2', withCmake: true, ]
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
          [ args: '-quiet', withCmake: true, ]
        ]
      ])
    }
  } // node('osx')
}
