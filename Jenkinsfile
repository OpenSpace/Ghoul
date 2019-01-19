stage('Build') {
    parallel linux: {
        node('linux') {
            timeout(time: 45, unit: 'MINUTES') {
                deleteDir()
                checkout scm
                sh '''
                    git submodule update --init --recursive
                    mkdir -p build
                    cd build 
                    cmake .. 
                    make -j2
                '''
            }
        }
    },
    windows: {
        node('windows') {
            timeout(time: 45, unit: 'MINUTES') {
                // We specify the workspace directory manually to reduce the path length and thus try to avoid MSB3491 on Visual Studio
                ws("${env.JENKINS_BASE}/G/${env.BRANCH_NAME}") {
                    deleteDir()
                    checkout scm
                    bat '''
                        git submodule update --init --recursive
                        if not exist "build" mkdir "build"
                        cd build
                        cmake -G "Visual Studio 15 2017 Win64" .. 
                        msbuild.exe Ghoul.sln /nologo /verbosity:minimal /m:2 /p:Configuration=Debug
                    '''
                }
            }
        }
    },
    osx: {
        node('osx') {
            timeout(time: 45, unit: 'MINUTES') {
                deleteDir()
                checkout scm
                sh '''
                    git submodule update --init --recursive
                    mkdir -p build
                    export PATH=${PATH}:/usr/local/bin:/Applications/CMake.app/Contents/bin
                    export CMAKE_BUILD_TOOL=/Applications/CMake.app/Contents/bin/CMake
                    srcDir=$PWD
                    if [ ! -d ${srcDir} ]; then
                      mkdir ${srcDir}
                    fi
                    if [ ! -d ${srcDir}/build ]; then
                      mkdir ${srcDir}/build
                    fi
                    cd ${srcDir}/build
                    /Applications/CMake.app/Contents/bin/cmake -G Xcode ..
                    xcodebuild -quiet
                '''
            }
        }
    }
}
