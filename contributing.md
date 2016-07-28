#Contributing

Note: This document is a draft. Parts of this process need legal review before I can accept contributions.

Thank you for your interest in contributing to Libaudioverse.  Contributions are always welcome.

please read this file in its entirety before coding.  If you don't, you may be wasting time.

If you do not yet have a GitHub account, you should create one now.
If you intend to code, you should also install Git.
Providing assistance with these tasks is beyond the scope of this document.

##Issues

All changes to Libaudioverse which are not trivial bug fixes must begin as an issue, opened against GitHub.
Please do not use issues to ask for help with using Libaudioverse.
Instead, [use the Google Group](https://groups.google.com/a/camlorn.net/d/forum/libaudioverse).

If your issue is a bug, please be sure to indicate which version of Libaudioverse you are using, which platform you are using it on, and from which language you are using it.

If you are requesting a feature, be aware that why is much more important than how.
The best thing you can do to make a case for a feature is explain what can be done with it.
Including use cases is extremely helpful.
Proposing an initial design is not necessary.
if you don't, we can work it out as part of the discussion.

##Pull Requests

to be accepted and merged, a pull request must meet the following requirements:

- You must have signed the CAA and you must indicate that you have done so in every PR you submit. I will eventually automate this process.  See the next section for instructions on signing the CAA, including what to do if you are doing work on behalf of an employer.

- Every commit in your PR must be identified with a GitHub user.  If any of them are not you, they must also sign the CAA.

- A pull request must be made against the development branch.

- It must pass CI.  If this is impractical, you need to give a good reason why.

- If your pull request has previous discussion, it should link the related issue or thread.

There are two ways to get me to close a pull request immediately:

- If you or someone else who owns the work indicates that you are unwilling to sign the CAA.  I do not accept code from people who have not done so.

- if you are attempting to add a feature without previous discussion.

Once you open a pull request and it meets all the requirements, I will review the changes, pointing out any problems I find.
Once anything that comes up is resolved, I will merge it and your code will be in Libaudioverse.

Libaudioverse does not currently have a style guide.
If you are creating a binding, you should follow the default style of the language in question.
For anything else, try to match the code around you.
In terms of style, the most important thing is consistency.

##The CAA

Libaudioverse requires that all contributors submitting code or documentation changes sign a copyright assignment agreement (CAA).
This  agreement gives me the power to use your contribution under any license, provided that I also make it available under the license you used when giving it to me.
Note that the proceeding summary is not legally binding in any way.
The only authoritative documents are the agreements themselves, either
[the Individual Copyright Assignment Agreement (ICAA)](http://camlorn.github.io/libaudioverse/icla.pdf)
or the
[Entity Copyright Assignment Agreement (ECAA)](http://camlorn.github.io/libaudioverse/ecaa.pdf).

If you are an individual and are  not doing work on behalf of an employer, signing the CAA is done electronically through [this Google Form](https://docs.google.com/forms/d/1wqbJROb7SRhjdvtvNVuURsARdRGNzc_1uY2rd1abei0/viewform).
You will be signing the ICAA.
I only require you to do so once, though you will need to indicate that you have signed on all PRs you submit.

If you are doing work on behalf of an employer, you must indicate that this is the case on the PR and get your employer to sign the ECAA for you.
The easiest way to do this is to have someone with the power to enter into legal agreements on behalf of your employer download and fill out the ECAA, e-mailing me a copy of the completed document at camlorn@camlorn.net.
They should also include your GitHub username.

Please be aware that being payed by your employer to work on Libaudioverse is not the only case in which the ECAA may be required.
Using company resources, doing work on company time, or being employed by someone to work on software similar to Libaudioverse in any way are also examples of cases where your employer may be able to claim ownership of copyright.
if you have any doubt whatsoever as to whether or not this is the case, please raise your concerns on the PR or contact me at camlorn@camlorn.net beforehand.

Finally, I am willing to accept signed CAAs by mail, but you will need to e-mail me beforehand to work out the details.