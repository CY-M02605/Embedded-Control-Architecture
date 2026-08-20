# Git_OPERATION_MANUAL

This document decribes some operations on git bash command

if you want to rewrite commit message from Git online page, use those commands below:
1. the commit message you want to rewrite is the last one you modified
    `git commit --amend -m "new commit message"`
    `git push --force-with-lease`
2. the commit message is not the last one
    `git rebase -i HEAD~3` (according to position of the commit message you want rewrite, if its order is larger than 3, you can use other number to apply to it). After that, you will see `pick xxxxxxx old commit message`, change the `pick` to reword and paste your new commit message on it. Reserve and exit. Input your new commit message again, don't forget to move the `#` in it. At last, use `git log --oneline -5` (for confirmation) and `git push --force-with-lease`.

if you want to delete commit message from Git online page, use those commands below:
    Change the `pick` to `drop`. Don't forget use `git log --oneline -5` (for confirmation) and `git push --force-with-lease`.

if you want to give up your modification on some files, execute those commands below:
1. already be pushed:
    git restore --source=HEAD~1 -- mis-editored_file's_name
    git add mis-editored_file's_name
    git commit -m "fix: revert accidental file change"
    git push
2. without pushing:
    git status
    git restore mis-editored_file's_name
    git add the_file_you_want_to_push
    git commit -m "update commit"
    git push

if you want to drawback an "add" command, use the command below:
    git restore --staged "your addded content"
Do not forget to type "git status" for checking the newest status of your repository after drawbacking.

if you want to delete a local branch, please use
    git branch -d "sub-branch"
    or
    git branch -D "sub-branch" --- This is forced deletion

if you want to delete a origin branch on repository, please use
    git push origin --delete "sub-branch"

if you want to check the submition difference between main and new branch, please use
    git diff main.."sub-branch"

if you want create a new branch, please use
    git switch -c "sub-branch name"

if you want merge your sub branch to main, please use
    git switch "your sub-branch"
    git merge main