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
      helper.createDirectory('build');
    }
    stage('tools/cppcheck/create') {
      sh(
        script: 'cppcheck --enable=all --xml --xml-version=2 -i ext --suppressions-list=support/cppcheck/suppressions.txt include src tests 2> build/cppcheck.xml',
        label: 'CPPCheck'
      )
      recordIssues(
        id: 'tools-cppcheck',
        tool: cppCheck()
      )
    }
    // stage('master/cloc/create') {
    //   sh 'cloc --by-file --exclude-dir=build,data,ext --xml --out=build/cloc.xml --force-lang-def=support/cloc/langDef --quiet .';
    // }
  }
},
linux_gcc: {
  node('linux' && 'gcc') {
    stage('linux-gcc/scm') {
      deleteDir();
      gitHelper.checkoutGit(url, branch);
    }
    stage('linux-gcc/build(make)') {
      compileHelper.build(compileHelper.Make(), compileHelper.Gcc(), '', '', 'build-make');
      compileHelper.recordCompileIssues(compileHelper.Gcc());
    }
    stage('linux-gcc/build(ninja)') {
        compileHelper.build(compileHelper.Ninja(), compileHelper.Gcc(), '', '', 'build-ninja');
    }
    stage('linux-gcc/test') {
      testHelper.runUnitTests('build/tests/Debug/GhoulTest');
    }
  } // node('linux')
},
linux_clang: {
  node('linux' && 'clang') {
    stage('linux-clang/scm') {
      deleteDir();
      gitHelper.checkoutGit(url, branch);
    }
    stage('linux-clang/build(make)') {
      compileHelper.build(compileHelper.Make(), compileHelper.Clang(), '', '', 'build-make');
      compileHelper.recordCompileIssues(compileHelper.Clang());
    }
    stage('linux-clang/build(ninja)') {
        compileHelper.build(compileHelper.Ninja(), compileHelper.Clang(), '', '', 'build-ninja');
    }
    stage('linux-clang/test') {
      testHelper.runUnitTests('build/tests/Debug/GhoulTest');
    }
  } // node('linux')
},
windows: {
  node('windows') {
    // We specify the workspace directory manually to reduce the path length and thus try to avoid MSB3491 on Visual Studio
    ws("${env.JENKINS_BASE}/G/${env.BRANCH_NAME}/${env.BUILD_ID}") {
      stage('windows/scm') {
        deleteDir();
        gitHelper.checkoutGit(url, branch);
      }
      stage('windows/build(msvc)') {
        compileHelper.build(compileHelper.VisualStudio(), compileHelper.VisualStudio(), '', '', 'build-msvc');
        compileHelper.recordCompileIssues(compileHelper.VisualStudio());
      }
      stage('windows/build(ninja)') {
        compileHelper.build(compileHelper.Ninja(), compileHelper.VisualStudio(), '', '', 'build-ninja');
      }
      stage('windows/test') {
        // Currently, the unit tests are failing on Windows
        testHelper.runUnitTests('build\\tests\\Debug\\GhoulTest')
      }
    }
  } // node('windows')
},
macos: {
  node('macos') {
    stage('macos/scm') {
      deleteDir();
      gitHelper.checkoutGit(url, branch);
    }
    stage('macos/build(make)') {
        compileHelper.build(compileHelper.Make(), compileHelper.Clang(), '', '', 'build-make');
    }
    stage('macos/build(xcode)') {
        compileHelper.build(compileHelper.Xcode(), compileHelper.Xcode(), '', '', 'build-xcode');
    }
    stage('macos/test') {
      // Currently, the unit tests are crashing on OS X
      testHelper.runUnitTests('build/tests/Debug/GhoulTest')
    }
  } // node('osx')
}
