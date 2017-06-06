stage('Build') {
	parallel linux: {
		node('linux') {
			timeout(time: 15, unit: 'MINUTES') {
				checkout scm
				sh '''
					git submodule update --init --recursive
					mkdir -p build
					cd build 
					cmake .. 
					make
				'''
			}
		}
	},
	windows: {
		node('windows') {
			timeout(time: 15, unit: 'MINUTES') {
				checkout scm
				bat '''
					git submodule update --init --recursive
					if not exist "build" mkdir "build"
					cd build
					cmake -G "Visual Studio 14 2015 Win64" .. 
					msbuild.exe Ghoul.sln /m:8 /p:Configuration=Debug
				'''
			}
		}
	},
	osx: {
		node('osx') {
			timeout(time: 15, unit: 'MINUTES') {
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
					xcodebuild
				'''
			}
		}
	}
}