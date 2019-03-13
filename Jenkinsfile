// Header

// Checks out Ghoul from Git with recursive submodules
// unfortunately, as of now (2019-03-12), there is no good way of specifying recursive
// submodules directly to the git Jenkins plugin
def checkoutGit() {
  checkout([
    $class: 'GitSCM',
    branches: [[name: "${env.BRANCH_NAME}"]],
    doGenerateSubmoduleConfigurations: false,
    extensions: [[
      $class: 'SubmoduleOption',
      disableSubmodules: false,
      parentCredentials: false,
      recursiveSubmodules: true,
      reference: '',
      trackingSubmodules: false
    ]],
    submoduleCfg: [],
    userRemoteConfigs: [[
      url: 'https://github.com/OpenSpace/Ghoul'
    ]]
  ])
}

// Runs the provided binary path (GhoulTest) and analyses the results using junit
def runTests(bin) {
  if (isUnix()) {
    sh "${bin} --gtest_output=xml:test_results.xml"
  }
  else {
    bat "${bin} --gtest_output=xml:test_results.xml"
  }
  junit([testResults: 'test_results.xml'])
}

def recordCompileIssues(compiler) {
  if (compiler.toLowerCase() == 'msvc') {
    recordIssues(tools: [msBuild()])
  }
  else if (compiler.toLowerCase() == 'clang') {
    recordIssues(tools: [clang()])
  }
  else if (compiler.toLowerCase() == 'gcc') {
    recordIssues(tools: [gcc4()])
  }
}

//
// Pipeline start
//
// parallel linux: {
//   node('linux') {
//     stage('linux/scm') {
//       deleteDir();
//       checkoutGit();
//     }
//     stage('linux/build') {
//       cmakeBuild([
//         installation: 'InSearchPath',
//         generator: 'Unix Makefiles',
//         buildDir: 'build',
//         steps: [[args: '-- -j4', withCmake: true]]
//       ]);
//     }
//     stage('linux/warnings') {
//       recordCompileIssues('gcc');
//     }
//     stage('linux/test') {
//       runTests('build/GhoulTest');
//     }
//   } // node('linux')
// },
// windows: {
//   node('windows') {
//     // We specify the workspace directory manually to reduce the path length and thus try to avoid MSB3491 on Visual Studio
//     ws("${env.JENKINS_BASE}/G/${env.BRANCH_NAME}/${env.BUILD_ID}") {
//       stage('windows/scm') {
//         deleteDir();
//         checkoutGit();
//       }
//       stage('windows/build') {
//         cmakeBuild([
//           installation: 'InSearchPath',
//           generator: 'Visual Studio 15 2017 Win64',
//           buildDir: 'build',
//           steps: [[args: '-- /nologo /verbosity:minimal /m:4', withCmake: true]]
//         ]);
//       }
//       stage('windows/warnings') {
//         recordCompileIssues('msvc');
//       }
//       stage('windows/test') {
//         // Currently, the unit tests are failing on Windows
//         // runTests('build\\Debug\\GhoulTest')
//       }
//     }
//   } // node('windows')
// },
// osx: {
//   node('osx') {
//     stage('osx/scm') {
//       deleteDir();
//       checkoutGit();
//     }
//     stage('osx/build') {
//       cmakeBuild([
//         installation: 'InSearchPath',
//         generator: 'Xcode',
//         buildDir: 'build',
//         steps: [[args: '-- -parallelizeTargets -jobs 4', withCmake: true]]
//       ]);
//     }
//     stage('osx/warnings') {
//       recordCompileIssues('clang');
//     }
//     stage('osx/test') {
//       // Currently, the unit tests are crashing on OS X
//       // runTests('build/Debug/GhoulTest')
//     }
//   } // node('osx')
// }


//
// Post-build actions
//

node('master') {
  stage('master/SCM') {
    deleteDir();
    checkoutGit();
  }
  stage('master/cppcheck') {
    sh 'cppcheck --enable=all --xml --xml-version=2 -i ext --suppressions-list=support/cppcheck/suppressions.txt include src tests 2> cppcheck.xml';
    publishCppcheck(pattern: 'cppcheck.xml');
  }
  stage('master/cloc') {
    sh 'cloc --by-file --exclude-dir=build,data,ext --xml --out=cloc.xml --force-lang-def=support/cloc/langDef --quiet .';
    sloccountPublish(encoding: '', pattern: 'cloc.xml');
  }
  stage('master/notifications') {
    echo pwd()
    def slackPlugin = load('support/jenkins/slack_notification.groovy');
    slackPlugin.sendSlackMessage();
  }
}
