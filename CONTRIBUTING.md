OVBS Studio Contribution Guidelines
============

OVBS Studio accepts contributions via public **pull requests**.
This repository is a fork with its own direction, so the current tree and
docs are the source of truth for how we work.

Anyone with the necessary permissions can open a pull request. Review and
acceptance decisions are made by the project maintainers based on what best
fits OVBS Studio's goals.

These guidelines are here to make contributions easier to review and help
preserve maintainability, but they are not a manifesto. Use good judgment,
keep changes focused, and work with maintainers when direction shifts.

## General Contribution Guidelines

The canonical language used by the project is “American English”, which mainly
applies to source code and non-translated material. Examples include:

* Commit messages
* Names of constants, variables, and types
* Source code comments
* File names

If a pull request template is shown for issues or PRs, fill it out as best you
can. If a section does not apply, note that clearly rather than leaving it
blank.

Authors of contributions to OVBS Studio are expected to abide by the project
Code of Conduct available at ./COC.rst.

## Commit Authoring Guidelines

We recommend the “50/72” standard for commit messages:

```
prefix: This is a commit title using 50 characters

This is a commit description line that is allowed to use up to 72 chars
```

Use prefixes based on the area of the repository affected. If you are unsure,
review recent history and follow the conventions already present in this repo.

### Commit Messages

Write commit messages that explain both what changed and why it changed.
Good commit text makes review faster and keeps context available in the
repository history.

Many of these ideas match the guidance in
https://chris.beams.io/git-commit.

### Commit Content Authoring Guidelines

#### Code Style

All code contributions should follow the style guidelines and formatter
configuration used by this repository.
See ./CODESTYLE.md and the repository's current formatting tools for details.

The most important rule is consistency: match the existing style in the files
you are changing.

To help with formatting, this repository includes config and helper scripts for
common languages and build tooling.

#### Commit Contents

A commit should represent a logical unit of change that makes sense on its
own. If a change depends on another fix, consider splitting it into separate
commits or PRs.

* Small fixes and formatting changes may be grouped if they are clearly related.
* Larger changes should be separated into clean, reviewable units.
* If a change would leave the build broken or functionality impaired, it should
  either be split into smaller commits or clearly explained in the PR.

> During review, maintainers may ask for follow-up changes. That is normal.
> When the review is complete, you may squash or rebase your commits to present
> a cleaner history.

#### Co-Authorship

If you build on someone else's work and the original author should be
credited, add a Co-authored-by line to the commit message.

```
module: Improve feature implementation

Additional context about the change

Co-authored-by: Name <email@example.com>
```

# AI/Machine Learning Policy

OVBS Studio welcomes AI-assisted development, creative tooling, and vibe coding.
We encourage contributors to experiment with AI systems, generative workflows,
and creative automation so long as the final code is clean, reviewed, and fully
functional.

AI tools are a part of the workflow here, not a replacement for human
judgment. Contributions still need to be tested, maintainable, and compatible
with the project license.

If an AI suggestion is used, it should be verified and adapted to the
repository’s style and quality expectations.

This project does not ban AI usage on principle. What matters is whether the
work is solid, respects the license, and helps move OVBS Studio forward.
