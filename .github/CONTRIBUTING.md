# How to Contribute

1. File a bug at https://github.com/JBMod/jbmod/issues (if there
   isn't one already).  If your patch is going to be large, you should start a
   discussion on GitHub first.

   Leave a comment to let us know that you are working on a PR for the issue.
   We'll assign the issue to you.

2. All contributors must certify the Developer Certificate of Origin by
   signing off each of their commits, on a per-commit basis.  (See below.)

4. Follow the normal process of [forking][] the project, and setup a new
   branch to work in.  It's important that each group of changes be done in
   separate branches in order to ensure that a pull request only includes the
   commits related to that bug or feature.

5. Add an entry to the [AUTHORS][] with your name and email if you wish.
   Adding contributors to the AUTHORS file is optional and has no implications
   for copyright ownership.

7. Do your best to have [well-formed commit messages][] for each change.
   This provides consistency throughout the project, and ensures that commit
   messages are able to be formatted properly by various git tools.

8. Finally, push the commits to your fork and submit a [pull request][].

## Before you begin

### Certifying the Developer Certificate of Origin (DCO)

The [DCO][] is a per-commit sign-off made by a contributor stating that they agree
to the terms published at https://developercertificate.org/ for that particular
contribution.

The [DCO][] is a legally binding statement asserting that you are the creator of
your contribution, or that you otherwise have the authority to distribute the
contribution, and that you are intentionally making the contribution available
under the license(s) associated with the project.

Each commit must include a DCO which looks like this:

`Signed-off-by: Joe Smith <joe.smith@email.com>`

Git makes it easy to add this line to your commit messages. Make sure the
`user.name` and `user.email` are set in your git configs. Use `-s` or
`--signoff` to add the Signed-off-by line to the end of the commit message.

Pseudonymous or anonymous contributions are accepted, but you must be
reachable at the email provided in the Signed-off-by line.

DCO sign-offs represent the source of truth as to copyright ownership for each
contribution, unlike the [AUTHORS][] file typically lists major contributors.  
You retain the copyright to your contribution; this simply gives us permission
to use and redistribute your contributions as part of the project.

### Review our Community Guidelines

All contributors pledge to follow our [Code of Conduct][], adapted from the
Contributor Covenant.

## Contribution process

### Code Reviews

All submissions, including submissions by project members, require review. We
use [GitHub pull requests](https://docs.github.com/articles/about-pull-requests)
for this purpose.

[Developer Certificate of Origin]: /.github/DCO.md
[DCO]: /.github/DCO.md
[AUTHORS]: /AUTHORS
[Code of Conduct]: /.github/CODE_OF_CONDUCT.md 
[forking]: https://help.github.com/articles/fork-a-repo
[well-formed commit messages]: http://tbaggery.com/2008/04/19/a-note-about-git-commit-messages.html
[pull request]: https://help.github.com/articles/creating-a-pull-request
