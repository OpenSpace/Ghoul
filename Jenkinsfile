parallel linux: {
  node('linux') {
    stage('SCM') {
      deleteDir()
      checkout scm
      sh 'git submodule update --init --recursive'
    }
    // sh 'mkdir -p build'
    // dir("build") {
      // stage('CMake') {
      //   sh 'cmake .. -G "Unix Makefiles"'
      // }
      stage('Build') {
        cmakeBuild([
          generator: "Unix Makefiles",
          buildDir: "build",
          installation: 'cmake'
        ])
      }

    //   stage('Build') {
    //     timeout(time: 30, unit: 'MINUTES') {
    //       sh 'make -j4'
    //     }
    //   }
    // }
  }
},
windows: {
  node('windows') {
    // We specify the workspace directory manually to reduce the path length and thus try to avoid MSB3491 on Visual Studio
    ws("${env.JENKINS_BASE}/G/${env.BRANCH_NAME}") {
      stage('SCM') {
        deleteDir()
        checkout scm
        bat 'git submodule update --init --recursive'
      }
      stage('Build') {
        cmakeBuild([
          generator: "Visual Studio 15 2017 Win64",
          buildDir: "build",
          installation: 'cmake'
        ])
        // cmakeBuild(generator: "Visual Studio 15 2017 Win64", buildDir: "build", withCMake: true)
      }
      // bat 'mkdir build 2> NUL'
      // dir ('build') {
      //     stage('CMake') {
      //         cmake()
      //         bat 'cmake .. -G "Visual Studio 15 2017 Win64"'
      //     }
      //     stage('Build') {
      //         timeout(time: 30, unit: 'MINUTES') {
      //             bat 'msbuild.exe Ghoul.sln /nologo /verbosity:minimal /m:2 /p:Configuration=Debug'
      //         }
      //     }
      // }
    }
  }
},
osx: {
  node('osx') {
    stage('SCM') {
      deleteDir()
      checkout scm
      sh 'git submodule update --init --recursive'
    }

      stage('Build') {
        cmakeBuild([
          generator: "Xcode",
          buildDir: "build",
          installation: 'cmake'
        ])

        // cmakeBuild(generator: "Xcode", buildDir: "build", withCMake: true)
      }

    // sh 'mkdir -p build'
    // dir("build") {
      // stage('CMake') {
      //   sh '/Applications/CMake.app/Contents/bin/cmake -G "Xcode" ..'
      // }
      // stage('Build') {
      //   timeout(time: 30, unit: 'MINUTES') {
      //     sh 'xcodebuild -quiet'
      //   }
      // }
    // }
  }
}
