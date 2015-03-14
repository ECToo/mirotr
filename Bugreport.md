# Introduction #
Any complex piece of software will contain bugs and OTR is no exception. Testing before a new release is made allows us to identify and fix all of the obvious bugs, however more obscure ones will inevitably make it into the final release. Reporting bugs is one of the best things that OTR users can do to help the project improve the software, but the developers only have a chance to fix bugs if they can reproduce the bugs themselves. This can be difficult and time consuming for more obscure ones. It's important to realise that a good bug report which easily allows a developer to reproduce the bug is more likely to get the developer's attention and the bug fixed. This page describes how to make a good bug report.

## Before reporting a bug ##
**Update to the latest release or development build.**Disable ALL non-core plugins (the ones that didn't come with Miranda IM).
**Check that it really is a bug. If you need help, see our support options.**Search the issues for an existing bug report. The grid provides a nice overview.
**Select "Feature Request" template above the summary field if:
- you hope to see a new feature in future releases.
- you propose an improvement to something that works (graphics, text, layout...).**

## Creating a test database ##
The OTR developers do not have access to a user's computer or database and for data privacy reasons a user is normally unwilling to send a copy of their database to a developer. Bugs that cause data corruption usually mean that the symptoms of the bug are only visible in the user's database. To work around this problem, please crate a test database and use it with minimal plugins. If you can reproduce the bug in the test database and you record the steps taken to producing the bug then there is a good chance that a developer can do the same. Here is how to create yourself a test database.
**Download Miranda IM UNICODE release (as you know ANSI is no longer supported)** Download the latest OTR release
**Put the OTR.dll and the OTR.pdb in your 'Plugins' folder** Start Miranda IM and create a new database

## Reproducing the bug ##
Bugs come in many different shapes and sizes. Some bugs just cause unexpected behaviour. Others are more serious and can cause OTR to crash. Identifying the source of the bug is the hard part because the problem you see might just be a symptom of the bug and not the actual bug itself. These types of bugs often don't show any signs when the corruption is occurring. You only notice the symptoms of the bug when another part of OTR tries to use the corrupted data. Here are some tips to help you reproduce the bug.

**Login in to your account and the protocols** Enable the Netlog feature of Miranda IM (if you use Jabber, please provde us a XML-Log)
**If OTR Crash, please provide us a Crashlog** YOU MUST post a valid VersionInfo! Because, we need Information, what Plugins you are exactly use.


Here a some Links, how to make an VersionsInfo, Crash Report & a Network Log:
See http://wiki.miranda-im.org/Version_information
Attach a Network Log:  http://wiki.miranda-im.org/Network_log
Attach a Crash Report: http://wiki.miranda-im.org/Crash_report

Attach a file with a Dr Watson log if you are experiencing crashes and the Crash Dumper plugin is not able to catch it:
- To enable Dr Watson, Start Menu->Run-> "drwtsn32 -i".
- Dr Watson crash logs can be found in c:\Documents and Settings\All Users\Application Data\Microsoft\Dr Watson\.


Important!:
If you ignore this, we can't provide any support. Plase respect this! We have not endless time and fancy.



## After reporting an issue ##
**Monitor the issues you report, and provide feedback and additional information if requested.**Post a note confirming when your issue has been resolved.
