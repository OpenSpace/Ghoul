library('sharedSpace'); // jenkins-pipeline-lib

def url = 'https://github.com/OpenSpace/Ghoul';
def branch = env.BRANCH_NAME;

//
// All third-party dependencies are provided by vcpkg. Every build node has to export
// VCPKG_ROOT pointing at a vcpkg checkout; the CMake presets in CMakePresets.json pick up
// the toolchain file from there. A shared vcpkg binary cache on the nodes keeps the
// dependency build from dominating the CI time.
//

//
// Pipeline start
//
parallel tools: {
  node('tools') {
    stage('tools/scm') {
      deleteDir();
      gitHelper.checkoutGit(url, branch, false);
    }
    stage('tools/cppcheck/create') {
      sh(
        script: 'cppcheck --enable=all --xml --xml-version=2 -i ext --suppressions-list=support/cppcheck/suppressions.txt include src tests 2> cppcheck.xml',
        label: 'CPPCheck'
      )
      recordIssues(
        id: 'tools-cppcheck',
        tool: cppCheck(pattern: 'cppcheck.xml')
      )
    }
    // stage('master/cloc/create') {
    //   sh 'cloc --by-file --exclude-dir=build,data,ext --xml --out=build/cloc.xml --force-lang-def=support/cloc/langDef --quiet .';
    // }
    cleanWs()
  }
},
linux_gcc: {
  if (env.USE_BUILD_OS_LINUX == 'true') {
    node('linux-gcc') {
      stage('linux-gcc/scm') {
        deleteDir();
        gitHelper.checkoutGit(url, branch);
      }
      stage('linux-gcc/build') {
        sh(
          script: 'cmake --preset linux && cmake --build --preset linux',
          label: 'Configure and build (Ninja, gcc)'
        );
        recordIssues(id: 'linux-gcc', tool: gcc());
      }
      stage('linux-gcc/test') {
        sh(script: 'ctest --preset linux', label: 'ctest');
      }
      cleanWs()
    } // node('linux-gcc')
  }
},
linux_clang: {
  if (env.USE_BUILD_OS_LINUX == 'true') {
    node('linux-clang') {
      stage('linux-clang/scm') {
        deleteDir();
        gitHelper.checkoutGit(url, branch);
      }
      stage('linux-clang/build') {
        sh(
          script: 'cmake --preset linux && cmake --build --preset linux',
          label: 'Configure and build (Ninja, clang)'
        );
        recordIssues(id: 'linux-clang', tool: clang());
      }
      stage('linux-clang/test') {
        sh(script: 'ctest --preset linux', label: 'ctest');
      }
      cleanWs()
    } // node('linux-clang')
  }
},
windows_msvc: {
  if (env.USE_BUILD_OS_WINDOWS == 'true') {
    node('windows') {
      stage('windows-msvc/scm') {
        deleteDir();
        gitHelper.checkoutGit(url, branch);
      }
      stage('windows-msvc/build') {
        bat(
          script: 'cmake --preset windows && cmake --build --preset windows',
          label: 'Configure and build (Visual Studio)'
        );
        recordIssues(id: 'windows-msvc', tool: msBuild());
      }
      stage('windows-msvc/test') {
        bat(script: 'ctest --preset windows', label: 'ctest');
      }
      cleanWs()
    } // node('windows')
  }
}
