$commit_msg = $args[0]
if(($null -eq $commit_msg) -or ("" -eq $commit_msg)) {
    Write-Error "push needs a commit message. Aborting..."
    return
}

Write-Output "Pushing all local changes to git..."

goto root
git add .
git commit -m "$commit_msg"
git push