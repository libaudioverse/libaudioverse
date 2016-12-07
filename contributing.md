# Contributing

Thank you for your interest in contributing to Libaudioverse.  Contributions are always welcome.

Our goal is to accept your contribution with a minimum of wasted time on both sides.  To that end, please read this file in its entirety before coding.  The purpose of the following is to lay out our standards so that you can meet them as quickly as possible, as opposed to spending significant time on something only to be told that your changes are rejected.

## Guidelines for Bug Reports

When submitting a bug report, please include at least the following information: the platform you are on, the language you are accessing Libaudioverse from, whether you are using 32 bit or 64 bit Libaudioverse, and steps to reproduce the problem.
If possible, please also include an executable test case.  The easier it is to run a program that demonstrates the bug, the faster it can be addressed.
Audio recordings can be helpful, but are not always required.  If getting a recording is difficult, wait for it to be requested.

## Guidelines for Code Contributions

before contributing code, please note the following:

-  All contributions must be made against the development branch and submitted  as GitHub pull requests.

- I may refuse your pull request.  If what you want to do will take a significant amount of time, ask first.  Also see below for guidelines on proposing new features.

-  Style is important.  This is a multi-language project without specific style guidelines, but trying to match the style of surrounding code is essential.

- Do not contribute bindings at this time.  The bindings generator is due for a complete rewrite to make it simpler and actually usable; coding against it will make this process more complicated.

-  Code in this repository must be maintained by people who are not you.  To that end, we will review all pull requests with an eye to any potential problems.  Fixing such problems falls on you; your pull request will not be merged until you have.

- Responsibility for rebasing on top of upstream changes and fixing merge conflicts falls on you; only conflict-free pull requests will be accepted

- Pull requests will only be accepted if they pass continuous integration.

- Finally, by submitting a pull request, you agree that your  contribution is compatible with and may be used under Libaudioverse's licensing terms, specifically that such code can be used under either or both the Gnu General Public License V3 or later and/or the Mozilla Public License 2.0.

## Guidelines for New Features

It is all too easy for projects such as this to grow out of control, gaining a million features that no one uses.  To that end, adding new functionality should start with opening an issue proposing a design for the feature.  You are strongly encouraged not to begin coding the feature until receiving approval that the feature will be accepted.

The following are the criteria we consider when adding features to Libaudioverse.  Showing how your proposal meets them is a good way to argue for it:

- The functionality must not currently be possible.  Libaudioverse does not want to offer 5 ways to do everything.

- The feature must not be an undue maintenance burden.  As  above, code in this repository is code we are committing to maintain with or without you.

- Someone must be willing to code it.  Asking for and getting approval for a feature is only the beginning.  The fastest way to get the feature is to be willing to write it yourself.

- The functionality must be useful, in the sense that there must be consensus that people would use it if it existed.

- Finally, the functionality must not make future plans impossible.

It is anticipated that features will start with a design, be refigned into a final form, and then implemented.  If you implement a feature and deviate significantly from the original design (especially without justification), your pull request will not be merged until you either bring it in line with the design or provide a justification.  While this project does not have an RFC process, you should think along those lines.  We may adopt something more formal in future.

If you are implementing new functionality and it is going to take a significant amount of time, you are encouraged to open a pull request whose title starts with `[WIP]`.  This will allow anyone interested to observe your progress and potentially catch problems as they arise.