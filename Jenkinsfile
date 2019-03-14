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


currentBuild.result = 'UNSTABLE';

//
// Post-build actions
//
// Returns a list of the commit messages that led to this build being triggered
// All messages are in the format:
// <date> [Author (optionally: @author for slac)] (commit id): commit message
// NonCPS is required as the changeSets of the current build are not serializable
@NonCPS
def changeString(build) {
  // def authors = load("${JENKINS_HOME}/slack_users.groovy").slackAuthors;
  def authors = [];

  def res = [];

  for (int i = 0; i < build.changeSets.size(); i++) {
    def entries = build.changeSets[i].items;
    for (int j = 0; j < entries.length; j++) {
      def entry = entries[j];

      def date = "${new Date(entry.timestamp).format("yyyy-MM-dd HH:mm:ss")}";
      def author = "${entry.author}";
      def authorHandle = authors[author];
      def fullAuthor = authorHandle ? "${author} (@${authorHandle})" : "${author}";
      def commit = "${entry.commitId.take(8)}";
      def message = "${entry.msg}";

      def fullMessage = "${date} [${fullAuthor}] (${commit}): ${message}";
      if (res.contains(fullMessage)) {
        // Having multiple parallel builds will cause messages to appear multiple times
        continue;
      }
      res.add(fullMessage);
    }
  }

  return res.join('\n');
}

def sendSlackMessage(build, changes) {
  def colors = [ 'SUCCESS': 'good', 'UNSTABLE': 'warning', 'FAILURE': 'danger' ];
  def humanReadable = [ 'SUCCESS': 'Success', 'UNSTABLE': 'Unstable', 'FAILURE': 'Failure' ];

  // The JOB_NAME will be 'GitHub/Ghoul/feature%2Fbranch-name'.  And we are interesting in
  // the project name here
  def (discard, job) = env.JOB_NAME.tokenize('/');

  def msgHeader = "*${job}*";
  def msgBuildTime = "Build time: ${build.duration / 1000}s";
  def msgBranch = "Branch: ${job}/${env.BRANCH_NAME}";
  def msgUrl = "URL: ${env.BUILD_URL}";
  // def changes = changeString(build);
  def msgChanges = "Changes:\n${changes}";

  if (!build.resultIsWorseOrEqualTo(build.previousBuild.currentResult)) {
    // The build is better than before
    def msgStatusPrev = humanReadable[build.previousBuild.currentResult];
    def msgStatusCurr = humanReadable[build.currentResult];
    def msgStatus = "Build status improved (${msgStatusPrev} -> ${msgStatusCurr})";

    slackSend(
      color: colors[build.currentResult],
      channel: 'Jenkins',
      message: "${msgHeader}\n\n${msgStatus}\n${msgBuildTime}\n${msgBranch}\n${msgUrl}\n${msgChanges}"
    );
  }
  else if (!build.resultIsBetterOrEqualTo(build.previousBuild.currentResult)) {
    // The build is worse than before
    def msgStatusPrev = humanReadable[build.previousBuild.currentResult];
    def msgStatusCurr = humanReadable[build.currentResult];
    def msgStatus = "Build status worsened (${msgStatusPrev} -> ${msgStatusCurr})";

    slackSend(
      color: colors[build.currentResult],
      channel: 'Jenkins',
      message: "${msgHeader}\n\n${msgStatus}\n${msgBuildTime}\n${msgBranch}\n${msgUrl}\n${msgChanges}"
    );
  }
  else if (build.currentResult != 'SUCCESS' && build.previousBuild.currentResult != 'SUCCESS') {
    // Only send another message if the build is still unstable or still failing
    def msgStatus = build.currentResult == "UNSTABLE" ? "Build still unstable" : "Build still failing";

    slackSend(
      color: colors[build.currentResult],
      channel: 'Jenkins',
      message: "${msgHeader}\n\n${msgStatus}\n${msgBuildTime}\n${msgBranch}\n${msgUrl}\n${msgChanges}"
    );
  }
  // Ignore the rest (FAILURE -> FAILURE  and SUCCESS -> SUCCESS)
}
node('master') {
  stage('master/SCM') {
    deleteDir();
    checkoutGit();
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
    // def workspace = pwd();
    // def slackPlugin = load("${workspace}/support/jenkins/slack_notification.groovy");
    sendSlackMessage(currentBuild, changeString(currentBuild));
    // slackPlugin.sendSlackMessage(currentBuild, changeString(currentBuild));
  }




}
