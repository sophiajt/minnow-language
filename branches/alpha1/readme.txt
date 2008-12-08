Minnow Programming Language 
Pre-release 0.xx

http://minnow-lang.org
http://code.google.com/p/minnow-language

See license.txt for license details

---
REQUIREMENTS:

  * Boost 1.35

NOTES:

Please note this is a pre-release, which currently means "expect few things to work".  Minnow is under rapid development, so expect its state to change quickly from day to day until it comes to build a release for public consumption.

The Minnow programming language is split into two parts:

  * 'minnow': a Minnow-to-C++ translator that outputs code which links to aquarium
  * 'aquarium': a library that handles the nuts and bolts of messaging, creating, rebalancing, isolating, and destroying actors

The translator can attempt to use a C++ compiler on your system, if it's available.  To change the commandline it uses see src/minnow/main.cpp

