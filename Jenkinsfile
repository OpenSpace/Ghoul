library('sharedSpace'); // jenkins-pipeline-lib

//
// Pipeline start
//
parallel linux: {
  node('linux') {
    stage('linux/scm') {
      deleteDir();
      gitHelper.checkoutGit('https://github.com/OpenSpace/Ghoul', env.BRANCH_NAME);
    }
    stage('linux/build') {
        compileHelper.build(compileHelper.generator.Make, compileHelper.compiler.GCC);
    }
    stage('linux/warnings') {
      compileHelper.recordIssues(compileHelper.compiler.GCC);
    }
    stage('linux/test') {
      testHelper.runTests('build/GhoulTest');
    }
  } // node('linux')
},
windows: {
  node('windows') {
    // We specify the workspace directory manually to reduce the path length and thus try to avoid MSB3491 on Visual Studio
    ws("${env.JENKINS_BASE}/G/${env.BRANCH_NAME}/${env.BUILD_ID}") {
      stage('windows/scm') {
        deleteDir();
        gitHelper.checkoutGit('https://github.com/OpenSpace/Ghoul', env.BRANCH_NAME);
      }
      stage('windows/build') {
        compileHelper.build(compileHelper.generator.VisualStudio, compileHelper.compiler.VisualStudio);
      }
      stage('windows/warnings') {
        compileHelper.recordIssues(compileHelper.compiler.VisualStudio);
      }
      stage('windows/test') {
        // Currently, the unit tests are failing on Windows
        // testHelper.runTests('build\\Debug\\GhoulTest')
      }
    }
  } // node('windows')
},
osx: {
  node('osx') {
    stage('osx/scm') {
      deleteDir();
      gitHelper.checkoutGit('https://github.com/OpenSpace/Ghoul', env.BRANCH_NAME);
    }
    stage('osx/build') {
        compileHelper.build(compileHelper.generator.Xcode, compileHelper.compiler.Clang);
    }
    stage('osx/warnings') {
      compileHelper.recordIssues(compileHelper.compiler.Clang);
    }
    stage('osx/test') {
      // Currently, the unit tests are crashing on OS X
      // testHelper.runTests('build/Debug/GhoulTest')
    }
  } // node('osx')
}


currentBuild.result = 'UNSTABLE';

//
// Post-build actions
//

node('master') {
  stage('master/SCM') {
    deleteDir();
    gitHelper.checkoutGit('https://github.com/OpenSpace/Ghoul', env.BRANCH_NAME);
    cmake([installation: 'InSearchPath', arguments: '-E make_directory build'])
  }
  stage('master/cppcheck') {
    sh 'cppcheck --enable=all --xml --xml-version=2 -i ext --suppressions-list=support/cppcheck/suppressions.txt include src tests 2> build/cppcheck.xml';
    publishCppcheck(pattern: 'build/cppcheck.xml');
  }
  stage('master/cloc') {
    sh 'cloc --by-file --exclude-dir=build,data,ext --xml --out=build/cloc.xml --force-lang-def=support/cloc/langDef --quiet .';
    sloccountPublish(encoding: '', pattern: 'build/cloc.xml');
  }
  stage('master/notifications') {
    slackHelper.sendChangeSetSlackMessage(currentBuild);
  }
}
