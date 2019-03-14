// Returns a list of the commit messages that led to this build being triggered
// All messages are in the format:
// <date> [Author (optionally: @author for slac)] (commit id): commit message
// NonCPS is required as the changeSets of the current build are not serializable
@NonCPS
def changeString(build) {
  def authors = load("${JENKINS_HOME}/slack_users.groovy").slackAuthors;

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

return this;
