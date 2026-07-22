# Git_OPERATION_MANUAL

This document decribes some operations on git bash command

if you want to delete Commits record(s) from Git online page, use those commands below:
1. Record is the most recent
    git reset --hard HEAD~2
    git push --force-with-lease
2. Record is before the most recent
    git log --oneline
    git rebase -i xxxxxxx (xxxxxxx is a commit record number for just one record before the you want to delete)
    delete your target, save and quit.
    git push --force-with-lease
    if there is a conflict code in it, execute these commands below sequenently:
    git checkout --theirs README.md
    git add README.md
    git rebase --continue
    save and quit
    git log --oneline -5 (for confirmation)
    git push --force-with-lease

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