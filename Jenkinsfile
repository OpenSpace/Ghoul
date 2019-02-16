parallel linux: {
    node('linux') {
        stage('SCM') {
            deleteDir()
            checkout scm
            sh 'git submodule update --init --recursive'
        }
        sh 'mkdir -p build'
        dir("build") {
            stage('CMake') {
                sh 'cmake .. -G "Unix Makefiles"'
            }
            stage('Build') {
                timeout(30) {
                    sh 'make -j4'
                }
            }
        }
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
            bat 'mkdir build 2> NUL'
            dir ('build') {
                stage('CMake') {
                    bat 'cmake .. -G "Visual Studio 15 2017 Win64"'
                }
                stage('Build') {
                    timeout(30) {
                        bat 'msbuild.exe Ghoul.sln /nologo /verbosity:minimal /m:2 /p:Configuration=Debug'
                    }
                }
            }
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
        sh 'mkdir -p build'
        dir("build") {
            stage('CMake') {
                sh '/Applications/CMake.app/Contents/bin/cmake -G "Xcode" ..'

            }
            stage('Build') {
                timeout(30) {
                    sh 'xcodebuild -quiet'
                }
            }
        }
    }
}
