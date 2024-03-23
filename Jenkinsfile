library('sharedSpace'); // jenkins-pipeline-lib

def url = 'https://github.com/OpenSpace/Ghoul';
def branch = env.BRANCH_NAME;

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
linux_gcc_make: {
  if (env.USE_BUILD_OS_LINUX == 'true') {
    node('linux-gcc') {
      stage('linux-gcc-make/scm') {
        deleteDir();
        gitHelper.checkoutGit(url, branch);
      }
      stage('linux-gcc-make/build') {
        compileHelper.build(compileHelper.Make(), compileHelper.Gcc(), '', '', 'build-make');
        compileHelper.recordCompileIssues(compileHelper.Gcc());
      }
      stage('linux-gcc-make/test') {
        testHelper.runUnitTests('build-make/tests/GhoulTest');
      }
      cleanWs()
    } // node('linux')
  }
},
linux_gcc_ninja: {
  if (env.USE_BUILD_OS_LINUX == 'true') {
    node('linux-gcc') {
      stage('linux-gcc-ninja/scm') {
        deleteDir();
        gitHelper.checkoutGit(url, branch);
      }
      stage('linux-gcc-ninja/build') {
          compileHelper.build(compileHelper.Ninja(), compileHelper.Gcc(), '', '', 'build-ninja');
      }
      stage('linux-gcc-ninja/test') {
        testHelper.runUnitTests('build-ninja/tests/GhoulTest');
      }
      cleanWs()
    } // node('linux')
  }
},
linux_clang_make: {
  if (env.USE_BUILD_OS_LINUX == 'true') {
    node('linux-clang') {
      stage('linux-clang-make/scm') {
        deleteDir();
        gitHelper.checkoutGit(url, branch);
      }
      stage('linux-clang-make/build') {
        compileHelper.build(compileHelper.Make(), compileHelper.Clang(), '', '', 'build-make');
        compileHelper.recordCompileIssues(compileHelper.Clang());
      }
      stage('linux-clang-make/test') {
        testHelper.runUnitTests('build-make/tests/GhoulTest');
      }
      cleanWs()
    } // node('linux')
  }
},
linux_clang_ninja: {
  if (env.USE_BUILD_OS_LINUX == 'true') {
    node('linux-clang') {
      stage('linux-clang-ninja/scm') {
        deleteDir();
        gitHelper.checkoutGit(url, branch);
      }
      stage('linux-clang-ninja/build') {
          compileHelper.build(compileHelper.Ninja(), compileHelper.Clang(), '', '', 'build-ninja');
      }
      stage('linux-clang-ninja/test') {
        testHelper.runUnitTests('build-ninja/tests/GhoulTest');
      }
      cleanWs()
    } // node('linux')
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
        compileHelper.build(compileHelper.VisualStudio(), compileHelper.VisualStudio(), '', '', 'build-msvc');
        compileHelper.recordCompileIssues(compileHelper.VisualStudio());
      }
      stage('wiwindows-msvcndows/test') {
        testHelper.runUnitTests('build-msvc\\tests\\Debug\\GhoulTest')
      }
      cleanWs()
    } // node('windows')
  }
},
windows_ninja: {
  if (env.USE_BUILD_OS_WINDOWS == 'true') {
    node('windows') {
      stage('windows-ninja/scm') {
        deleteDir();
        gitHelper.checkoutGit(url, branch);
      }
      stage('windows-ninja/build') {
        compileHelper.build(compileHelper.Ninja(), compileHelper.VisualStudio(), '', '', 'build-ninja');
      }
      stage('windows-ninja/test') {
        testHelper.runUnitTests('build-ninja\\tests\\GhoulTest')
      }
      cleanWs()
    } // node('windows')
  }
},
macos_make: {
  if (env.USE_BUILD_OS_MACOS == 'true') {
    node('macos') {
      stage('macos-make/scm') {
        deleteDir();
        gitHelper.checkoutGit(url, branch);
      }
      stage('macos-make/build') {
          compileHelper.build(compileHelper.Make(), compileHelper.Clang(), '', '', 'build-make');
      }
      stage('macos-make/test') {
        testHelper.runUnitTests('build-make/tests/GhoulTest')
      }
      cleanWs()
    } // node('osx')
  }
},
macos_ninja: {
  if (env.USE_BUILD_OS_MACOS == 'true') {
    node('macos') {
      stage('macos-xcode/scm') {
        deleteDir();
        gitHelper.checkoutGit(url, branch);
      }
      stage('macos-xcode/build') {
          compileHelper.build(compileHelper.Xcode(), compileHelper.Xcode(), '', '', 'build-xcode');
      }
      stage('macos-xcode/test') {
        testHelper.runUnitTests('build-xcode/tests/GhoulTest')
      }
      cleanWs()
    } // node('osx')
  }
}
