# mini-shell
Bash-esque mini-shell in C

## Known issues
- Jobs doesn't pipe into wc properly. I suspect that this is related to all_jobs being a global variable but that should be copied over on forking so I'm unsure that this is the real issue.
- Running a shell within a shell doesn't work, and the "exit" command doesn't work.
- The set of paths that are checked to run commands is smaller than desired.
