stage('Build') {
    parallel linux: {
        node('linux') {
            timeout(30) {
                deleteDir()
                checkout scm
                sh 'git submodule update --init --recursive'
                sh 'mkdir -p build'
                dir("build") {
                    sh 'cmake .. -G "Unix Makefiles"'
                    sh 'make -j4'
                }
            }
        }
    },
    windows: {
        node('windows') {
            timeout(30) {
                // We specify the workspace directory manually to reduce the path length and thus try to avoid MSB3491 on Visual Studio
                ws("${env.JENKINS_BASE}/G/${env.BRANCH_NAME}") {
                    deleteDir()
                    checkout scm
                    bat 'git submodule update --init --recursive'
                    bat 'mkdir build 2> NUL'
                    dir ('build') {
                        bat 'cmake .. -G "Visual Studio 15 2017 Win64"'
                        bat 'msbuild.exe Ghoul.sln /nologo /verbosity:minimal /m:2 /p:Configuration=Debug'
                    }
                }
            }
        }
    },
    osx: {
        node('osx') {
            timeout(30) {
                deleteDir()
                checkout scm
                sh 'git submodule update --init --recursive'
                sh '''
                    mkdir -p build
                    export PATH=${PATH}:/usr/local/bin:/Applications/CMake.app/Contents/bin
                    srcDir=$PWD
                    if [ ! -d ${srcDir} ]; then
                      mkdir ${srcDir}
                    fi
                    if [ ! -d ${srcDir}/build ]; then
                      mkdir ${srcDir}/build
                    fi
                '''
                dir("build") {
                    sh '/Applications/CMake.app/Contents/bin/cmake -G "Xcode" ..'
                    sh 'xcodebuild -quiet'
                }
            }
        }
    }
}
