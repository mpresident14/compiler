# compiler
Simple compiler for my simple made-up language. WIP.


## Notes (to self, mostly):
For parser generator, any objects passed as arguments to a constructor are subsequently destroyed, so they should be copied or moved. The constructor assumes ownership of any pointers.
