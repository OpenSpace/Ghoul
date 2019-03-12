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
parallel linux: {
  node('linux') {
    stage('linux/SCM') {
      deleteDir()
      checkoutGit()
    }
    stage('linux/Build') {
      cmakeBuild([
        installation: 'InSearchPath',
        generator: 'Unix Makefiles',
        buildDir: 'build',
        steps: [[args: '-- -j4', withCmake: true]]
      ])
    }
    stage('linux/test') {
      runTests('build/GhoulTest')
    }
  } // node('linux')
},
windows: {
  node('windows') {
    // We specify the workspace directory manually to reduce the path length and thus try to avoid MSB3491 on Visual Studio
    ws("${env.JENKINS_BASE}/G/${env.BRANCH_NAME}/${env.BUILD_ID}") {
      stage('windows/SCM') {
        deleteDir()
        checkoutGit()
      }
      stage('windows/Build') {
        cmakeBuild([
          installation: 'InSearchPath',
          generator: 'Visual Studio 15 2017 Win64',
          buildDir: 'build',
          steps: [[args: '-- /nologo /verbosity:minimal /m:4', withCmake: true]]
        ])
      }
      stage('windows/test') {
        // Currently, the unit tests are failing on Windows
        // runTests('build\\Debug\\GhoulTest')
      }
    }
  } // node('windows')
},
osx: {
  node('osx') {
    stage('osx/SCM') {
      deleteDir()
      checkoutGit()
    }
    stage('osx/Build') {
      cmakeBuild([
        installation: 'InSearchPath',
        generator: 'Xcode',
        buildDir: 'build',
        steps: [[args: '-- -parallelizeTargets -jobs 4', withCmake: true]]
      ])
    }
    stage('osx/test') {
      // Currently, the unit tests are crashing on OS X
      // runTests('build/Debug/GhoulTest')
    }
  } // node('osx')
}


@NonCPS
def getChangeString(build) {
    def MAX_MSG_LEN = 100
    def changeString = ""
    build.rawBuild.changeSets.each {entries -> 
        entries.each { entry -> 
            changeString += "${new Date(entry.timestamp).format("yyyy-MM-dd HH:mm:ss")} "
            changeString += "[${entry.commitId.take(8)}] ${entry.author}: ${entry.msg.take(MAX_MSG_LEN)}\n"
        }
    }
    return changeString ?: " - No new changes"
}

mail([
  to: 'alexander.bock@liu.se',
  from: 'jenkins@dev.openspaceproject.com',
  subject: "Hello there (after): ${env.JOB_NAME}",
  body:  "BRANCH_NAME:${env.BRANCH_NAME}\nCHANGE_ID:${env.CHANGE_ID}\nCHANGE_URL:${env.CHANGE_URL}\nCHANGE_TITLE:${env.CHANGE_TITLE}\nCHANGE_AUTHOR:${env.CHANGE_AUTHOR}\nCHANGE_AUTHOR_DISPLAY_NAME:${env.CHANGE_AUTHOR_DISPLAY_NAME}\nCHANGE_AUTHOR_EMAIL:${env.CHANGE_AUTHOR_EMAIL}\nCHANGE_TARGET:${env.CHANGE_TARGET}\nBUILD_NUMBER:${env.BUILD_NUMBER}\nBUILD_ID:${env.BUILD_ID}\nBUILD_DISPLAY_NAME:${env.BUILD_DISPLAY_NAME}\nJOB_NAME:${env.JOB_NAME}\nJOB_BASE_NAME:${env.JOB_BASE_NAME}\nBUILD_TAG:${env.BUILD_TAG}\nEXECUTOR_NUMBER:${env.EXECUTOR_NUMBER}\nNODE_NAME:${env.NODE_NAME}\nNODE_LABELS:${env.NODE_LABELS}\nWORKSPACE:${env.WORKSPACE}\nJENKINS_HOME:${env.JENKINS_HOME}\nJENKINS_URL:${env.JENKINS_URL}\nBUILD_URL:${env.BUILD_URL}\nJOB_URL:${env.JOB_URL}GIT_COMMIT:${env.GIT_COMMIT}\nGIT_PREVIOUS_COMMIT:${env.GIT_PREVIOUS_COMMIT}\nGIT_PREVIOUS_SUCCESSFUL_COMMIT:${env.GIT_PREVIOUS_SUCCESSFUL_COMMIT}\nGIT_BRANCH:${env.GIT_BRANCH}\nGIT_LOCAL_BRANCH:${env.GIT_LOCAL_BRANCH}\nGIT_URL:${env.GIT_URL}\nGIT_COMMITTER_NAME:${env.GIT_COMMITTER_NAME}\nGIT_AUTHOR_NAME:${env.GIT_AUTHOR_NAME}\nGIT_COMMITTER_EMAIL:${env.GIT_COMMITTER_EMAIL}\nGIT_AUTHOR_EMAIL:${env.GIT_AUTHOR_EMAIL}\n\n" + getChangeString(build)
])
