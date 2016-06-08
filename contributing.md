#Contributing#

Thank you for your interest in contributing to Libaudioverse.  The following tells you how to submit bugs, make feature requests, and provide code and documentation changes.

Please read this file in its entirety before submitting code.
If you don't, it simply wastes time on both sides.

There is a slight amount of documentation on adding nodes in this document.
Documenting this properly is planned.

##An Overview of Submitting Code##

If you wish to submit a code contribution to this project, please do the following.
Each of these steps is explained further below.

- Familiarize yourself with the Copyright Assignment Agreement.  You will need to agree to it before I accept your pull requests.  You don't need to worry about this if you're only opening issues.  It is only required before I can accept changes to the actual repository or in other very special circumstancces.

- Submit an issue.  If I'm not going to accept your contribution, it is best to find out now.  If you are going to work on fixing the described bug or adding the described feature, be sure to indicate that this is the case.

- Wait on an indication that I will accept your changes.

- Fork this repository and make your changes.  They should be based off the `development` branch.

- Optionally rebase your work on top of `development`.  This saves me from having to deal with large merge conflicts for changes that you have been working on for a while.  If you don't know how to do this, it's best not to.

- Submit a pull request against the `development` branch.

- If you haven't signed the CAA, sign it now following the directions in this document.

- Add a comment to the pull request indicating that you have signed the CAA.  This part of the process is likely to be automated in future, but isn't for the time being.

The rest of this document discusses the above in detail.

##Signing The Copyright Assignment Agreement##

Note: If you do not like the CAA, I am open to hearing your feedback.  Also be aware that this process will be streamlined in future.  Automating it is doable, but was complicated enough that I decided to postpone it until some point after Libaudioverse 0.9.

TODO: describe the CLA.

##Submitting an Issue##


If you wish to submit code or documentation, the first step is submitting an issue.
Submitting an issue in no way implies that you are responsible for doing whatever needs doing, so feel free to submit even if you don't intend to code.

There are two categories of issues: bugs and features.

###Bugs###

If you've found something that doesn't work as it should, it's a bug.
If you are reporting a bug, please include the following information.

The following items are not optional:

- A description of what you're trying to do and what happened instead.

- Which version of Libaudioverse you're using, from which language, and on which platform.  Please include whether this is a 32-bit or 64-bit version of Libaudioverse.

- A minimal test case.  This should be something that someone can download and run with Libaudioverse.  It should have no other dependencies and should be trivially runnable.  For example `python test.py` should be sufficient to show the bug for a bug reported by a Python user.  Do not provide binaries unless requested explicitly, as they are both mostly useless and come with trust issues.

The following optional items can be extremely helpful in some cases.
If one of them is explicitly needed, it will be requested.
If you can get them without much work, though, providing them can accelerate fixing the reported bug.

- A stack trace of Libaudioverse's C++ code.

- A core dump of your application running against a debug build of Libaudioverse.

- A recording of any eronious audio.

- If your project is open source, a pointer to the repository containing it.

###Features###

While bugs are mostly mechanical in their reporting format, feature requests are much more freeform.
None of the following is a requirement for having a feature request accepted.
This section aims to lay out what kind of discussion to aim for, no more.

The first thing to note is that I am much more likely to accept feature requests if you indicate that you are willing to code them.
Doing the work is a great way to get your suggested feature into the project.

Unlike bugs, features involve much more discussion.
This project is large and complicated, so adding new features requires a good deal of consideration.

The value of the feature must be established.
Features with more value will be given higher priority.
The easiest way to demonstrate value is to provide examples of use cases in which the feature is valuable.
I will not add features that have no value.
Removing a feature after the fact is difficult, and someone has to maintain it for as long as it exists.

Local features are preferred over global ones.
For instance, a global setting is usually (but not always) a bad design.
Per-server settings or even per-node settings are much better.
The reason for this is that the scope of features should be limited to code that knows about them.
If projects wanting to use new features will have to change code in places unrelated to the code using the feature, then it needs to be a feature with a lot of value.

Breaking compatibility with existing code is still allowed as this is currently a pre-1.0 project.
But it shouldn't be done lightly.
Features which break compatibility require the most justification of all, including explaining why other designs don't work.

Knowing that Libaudioverse needs to be able to do something it can't and being able to articulate why are the only requirements to ask for a feature.
You don't need to know how to design it.
Working out the design of the feature can happen organically as part of the discussion of whether to add it.

##Coding##

Note: If you are not willing to sign the CAA, stop.  I will not accept code from users who  are not willing to signed the CAA.

To submit code, you should fork this repository and create a branch for the changes youa re making.
Putting your code on the development branch will make part of the pull request proces harder.
If you can't come up with a good branch name, I suggest `issue-xxx` where `xxx` is the issue's number.

This project has no style guide, so try to keep your code looking like code around it.
Note that the C++ portion of this project uses tabs, but Python uses 4 spaces.
It is important that you match indentation types, though reindenting for consistency is on the to-do list.

For languages with an official style such as Python's PEP8, we follow that style.
This mostly pertains to bindings, but some Python helper scripts exist.

Your code should not bring in dependencies without a good reason, especially if adding them to this repository and integrating their building into CMake is not possible.
Libaudioverse already depends on a lot of stuff that it can't vendor.
Every new non-vendored dependency must be added to all CI configurations and makes building on Windows harder.

If you add new functions, you need to add them to `metadata/functions.y`.  The rest of the file serves as an example of how to do it.
new constants need to go in `metadata/enumerations.y` and must never be defined outside enums.
If you add a significant feature, you need to also add something to the manual found in `docs/`.

###Adding a Node###

The process for adding a new node is not yet thoroughly documented.
As nodes are 99% of the library's features, I've provided the following list to get you started writing one.
You need the following to have a working node that adds itself to all bindings:

- An implementation in `/include/libaudioverse/implementations/`.  For example, `biquad.hpp` contains the class `BiquadFilter` which is used by the `BiquadNode`.

- A constant in `libaudioverse.h` of the form `Lav_OBJTYPE_NAME_NODE`.

- A file `metadata/nodes/Lav_OBJTYPE_NAME_NODE.y` describing the node.  See the other files in this directory for examples.

- A file `include/nodes/my.hpp` containing:

  -The class `MyNode`, which must inherit from `Node` in `include/libaudioverse/private/node.hpp`.

  - A function `createMyNode` returning `std::shared_ptr<MyNode>`.

- For a node `my_node`, a file `src/libaudioverse/nodes/my.cpp` containing:

  - A function that creates it of the form `LavError Lav_PUBLIC_FUNCTION Lav_createMyNode(LavHandle serverHandle, other arguments here, LavHandle* destination)`.  This should call a function of the form `createMyNode`.  The name of the first and last argument is important and must match that provided here.

  - the implementation of `createMyNode`.

  - The implementation of the `MyNode` class.  This class must override `process` for 99% of cases.

Finally, add `Lav_createMyNode` to `libaudioverse.h` at the end.
Build the project.
If all goes well, you now have a working node.


##Submitting a Pull Request##

Note: Creating an issue first and finding out if I will accept the proposed changes is a very good idea before actually coding a new feature.
I am likely to accept bug fixes without prior discussion.

Assuming that you've created an issue and that we decided that your changes would be accepted, your pull request need only point readers at that issue.
Otherwise, you're going to need to justify your changes, including explaining *exactly* what they do.

Your pull request must meet two requirements before I will devote significant time to it:

- You must have signed the CAA.  At the moment, you must leave a comment on the pull request indicating that this is the case.

- The pull request must pass CI or have a comment explaining the good reason that it doesn't.  "It was too hard" or the like is not a good reason.  "We need to add the following two tools to the CI process, and only you can do that" is a good reason.

If you meet these requirements and have not yet created an issue, then we will have to discuss the changes.
In this case, all of the discussion of good issues pertains to the pull request.
In this context, bugs require very little discussion: if I'm confident that you're not introducing a new bug somewhere else, I'll just accept it.
Features will require much more.

Assuming that we already had said discussion in an issue, however, I will review your changes.
As long as I find no problems, I'll accept it without further discussion.