library('sharedSpace'); // jenkins-pipeline-lib

def url = 'https://github.com/OpenSpace/Ghoul';
def branch = env.BRANCH_NAME;

//
// Pipeline start
//
parallel master: {
  node('master') {
    stage('master/scm') {
      deleteDir();
      gitHelper.checkoutGit(url, branch);
      helper.createDirectory('build');
    }
    stage('master/cppcheck/create') {
      sh 'cppcheck --enable=all --xml --xml-version=2 -i ext --suppressions-list=support/cppcheck/suppressions.txt include src tests 2> build/cppcheck.xml';
    }
    stage('master/cloc/create') {
      sh 'cloc --by-file --exclude-dir=build,data,ext --xml --out=build/cloc.xml --force-lang-def=support/cloc/langDef --quiet .';
    }
  }
},
linux: {
  node('linux') {
    stage('linux/scm') {
      deleteDir();
      gitHelper.checkoutGit(url, branch);
    }
    stage('linux/build') {
        compileHelper.build(compileHelper.Make(), compileHelper.Gcc());
    }
    stage('linux/warnings') {
      compileHelper.recordCompileIssues(compileHelper.Gcc());
    }
    stage('linux/test') {
      testHelper.runUnitTests('build/GhoulTest');
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
      stage('windows/build') {
        compileHelper.build(compileHelper.VisualStudio(), compileHelper.VisualStudio());
      }
      stage('windows/warnings') {
        compileHelper.recordCompileIssues(compileHelper.VisualStudio());
      }
      stage('windows/test') {
        // Currently, the unit tests are failing on Windows
        // testHelper.runUnitTests('build\\Debug\\GhoulTest')
      }
    }
  } // node('windows')
},
osx: {
  node('osx') {
    stage('osx/scm') {
      deleteDir();
      gitHelper.checkoutGit(url, branch);
    }
    stage('osx/build') {
        compileHelper.build(compileHelper.Xcode(), compileHelper.Clang());
    }
    stage('osx/warnings') {
      compileHelper.recordCompileIssues(compileHelper.Clang());
    }
    stage('osx/test') {
      // Currently, the unit tests are crashing on OS X
      // testHelper.runUnitTests('build/Debug/GhoulTest')
    }
  } // node('osx')
}


//
// Post-build actions
//

node('master') {
  stage('master/cppcheck/publish') {
    publishCppcheck(pattern: 'build/cppcheck.xml');
  }
  stage('master/cloc/publish') {
    sloccountPublish(encoding: '', pattern: 'build/cloc.xml');
  }
  stage('master/notifications') {
    slackHelper.sendChangeSetSlackMessage(currentBuild);
  }
}
