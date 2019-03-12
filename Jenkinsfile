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


//
// Pipeline start
//
// parallel linux: {
//   node('linux') {
//     stage('linux/SCM') {
//       deleteDir()
//       checkoutGit()
//     }
//     stage('linux/Build') {
//       cmakeBuild([
//         installation: 'InSearchPath',
//         generator: 'Unix Makefiles',
//         buildDir: 'build',
//         steps: [[args: '-- -j4', withCmake: true]]
//       ])
//     }
//     stage('linux/test') {
//       runTests('build/GhoulTest')
//     }
//   } // node('linux')
// },
// windows: {
//   node('windows') {
//     // We specify the workspace directory manually to reduce the path length and thus try to avoid MSB3491 on Visual Studio
//     ws("${env.JENKINS_BASE}/G/${env.BRANCH_NAME}/${env.BUILD_ID}") {
//       stage('windows/SCM') {
//         deleteDir()
//         checkoutGit()
//       }
//       stage('windows/Build') {
//         cmakeBuild([
//           installation: 'InSearchPath',
//           generator: 'Visual Studio 15 2017 Win64',
//           buildDir: 'build',
//           steps: [[args: '-- /nologo /verbosity:minimal /m:4', withCmake: true]]
//         ])
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
//     stage('osx/SCM') {
//       deleteDir()
//       checkoutGit()
//     }
//     stage('osx/Build') {
//       cmakeBuild([
//         installation: 'InSearchPath',
//         generator: 'Xcode',
//         buildDir: 'build',
//         steps: [[args: '-- -parallelizeTargets -jobs 4', withCmake: true]]
//       ])
//     }
//     stage('osx/test') {
//       // Currently, the unit tests are crashing on OS X
//       // runTests('build/Debug/GhoulTest')
//     }
//   } // node('osx')
// }

node('linux') {
  stage('linux/SCM') {
    deleteDir()
    checkoutGit()
  }
}
currentBuild.result = 'SUCCESS';

//
// Post-build options

@NonCPS
def changeString() {
  def authors = [
    'Alexanvder Bock': 'alex',
    'Emil Axelsson': 'emil',
    'Gene Payne': 'gpayne',
    'Jonathas Costa': 'jccosta',
    'Micah Acinapura': 'Micah'
  ];

  // def authors = [:]
  // authors['Alexander Bock'] = 'alex';
  // authors['Emil Axelsson'] = 'emil';
  // authors['Gene Payne'] = 'gpayne';
  // authors['Jonathas Costa'] = 'jccosta';
  // authors['Micah Acinapura'] = 'Micah';

  def res = "";

  for (int i = 0; i < currentBuild.changeSets.size(); i++) {
    def entries = currentBuild.changeSets[i].items;
    for (int j = 0; j < entries.length; j++) {
      def entry = entries[j];

      def date = "${new Date(entry.timestamp).format("yyyy-MM-dd HH:mm:ss")}";
      // def author = authors.containsKey(entry.author) ? "${entry.author} (@${authors[entry.author]})" : "${entry.author}";
      def author = "${entry.author}";
      def authorHandle = authors[author];
      def fullAuthor = authorHandle ? "${author} (@${authorHandle})" : "${author}";
      def commit = "${entry.commitId.take(8)}";
      def message = "${entry.msg}";
      res += "${date} [${fullAuthor}] (${commit}): ${message}\n"
    }
  }

  return res;
}

stage('Notifications/Slack') {
  def colors = [
    'SUCCESS': 'good',
    'UNSTABLE': 'warning',
    'FAILURE': 'danger'
  ]

  def worse = !currentBuild.resultIsBetterOrEqualTo(currentBuild.previousBuild.currentResult)
  def better = !currentBuild.resultIsWorseOrEqualTo(currentBuild.previousBuild.currentResult)

  def (discard, job) = env.JOB_NAME.tokenize('/');
  def changes = changeString();

  def msgBranch = "Branch: ${job}/${env.BRANCH_NAME}";
  def msgUrl = "URL: ${env.BUILD_URL}";
  def msgChanges = "Changes: ${changes}";

  if (better) {
    def msgStatus = "Build status improved (${currentBuild.previousBuild.currentResult} -> ${currentBuild.currentResult})";

    slackSend(
      color: colors[currentBuild.currentResult],
      channel: 'Jenkins',
      message: "${msgStatus}\n${msgBranch}\n${msgUrl}\n${msgChanges}"
    )
  }
  else if (worse) {
    def msgStatus = "Build status worsened (${currentBuild.previousBuild.currentResult} -> ${currentBuild.currentResult})";
    def msgBuildTime = "Build time: ${currentBuild.duration / 1000}";

    slackSend(
      color: colors[currentBuild.currentResult],
      channel: 'Jenkins',
      message: "${msgStatus}\n${msgBranch}\n${msgUrl}\n${msgChanges}"
    )
  }
  else if (currentBuild.currentResult == 'UNSTABLE' && currentBuild.previousBuild.currentResult == 'UNSTABLE') {
    def msgStatus = "Build still unstable";
    def msgBuildTime = "Build time: ${currentBuild.duration / 1000}";

    slackSend(
      color: colors[currentBuild.currentResult],
      channel: 'Jenkins',
      message: "${msgStatus}\n${msgBranch}\n${msgUrl}\n${msgChanges}"
    )
  }
}
// if (!currentBuild.resultIsBetterOrEqualTo(currentBuild.previousBuild.currentResult)) {
//   mail([
//     to: 'alexander.bock@liu.se',
//     from: 'jenkins@dev.openspaceproject.com',
//     subject: "Hello there (after): ${env.JOB_NAME}",
//     body:  "not better or equal"
//   ])
// }
// else {
//   mail([
//     to: 'alexander.bock@liu.se',
//     from: 'jenkins@dev.openspaceproject.com',
//     subject: "Hello there (after): ${env.JOB_NAME}",
//     body:  "better or equal"
//   ])
// }





// mail([
//   to: 'alexander.bock@liu.se',
//   from: 'jenkins@dev.openspaceproject.com',
//   subject: "Hello there (after): ${env.JOB_NAME}",
//   body:  changeString(currentBuild)
// ])
