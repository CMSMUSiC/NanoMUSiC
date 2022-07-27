# Contributing to TAPAS 
## Git Workflow
* Check out the development branch: `git checkout master`
* Create a feature branch for your changes: `git checkout -b my-feature-branch`
* Implement and test your changes
* Add changes to the staging area: `git add -p .`
* Create a commit object containing the changes since the last commit: `git commit`, write a message (see below)
* Check that everything is added: `git status` should show a clean state, use a `.gitignore` file to avoid clutter
* Merge other's changes into your branch: `git fetch` and `git merge master`
* Push your branch to the remote repository: `git push -u origin my-feature-branch`
* Visit the Gitlab project page and follow the instructions to post a merge request (next section)

## Posting a Merge Request in GitLab
* Ensure that your code compiles / runs
* Extensively test your feature
* Follow the coding conventions (especially: remove commented-out code, check spelling)
* Write great commit messages (see below)
* Refer to related Jira-Issues in the commit body, e.g. "Closes MARII-123"
* Squash multiple commits into one (using `git rebase -i`, [see here](http://makandracards.com/makandra/527-squash-several-git-commits-into-a-single-commit))
* Explain testing (from step 2) in the commit message
* After posting: Watch and respond to comments and changes
* Avoid accepting your own merge requests. You will be solely responsible for everything you brake otherwise !

## Accepting a Merge Request
* Check-out the branch and run the tests described in the message (avoids "works on my machine" effects)
* Review the code quality of the changes
* Post comments if you have questions regarding specific code sections
* If you fix something yourself: also leave a comment for the original author
* Accept the merge request!
* Delete the source branch

## Creating Commit Messages
### Required documentation:
* Use the body to explain what and why vs. how. 
* One line commit messages are in general only allowed if only one line of code is changed. 
* If you change more code your commit message should at least contain: the motivation for the changes, the idea behind your changes and possible resources you used (e.g. new recomendations by CMS twiki pages, papers, hypernews).
  
  example: 
  
  avoid: "adjusted parameter XY using setXY"
  
  instead: "Handling of "something" changed, adjusted parameter XY as recommended in http://twiki.cern.ch/.../someTopic/"
         
### Style Recommendations:
* Separate subject from body with a blank line
* Limit the subject line to 50 characters
* Capitalize the subject line or start e.g. with the name of the tool you change.
* Do not end the subject line with a period
* Use the imperative mood in the subject line
* Wrap the body at 72 characters