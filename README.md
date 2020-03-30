# compiler
Simple compiler for my simple made-up language. WIP.


## Notes (to self, mostly):
For parser generator, any objects passed as arguments to a constructor are subsequently (shallowly) destroyed, so they should be copied or moved. Since
the destruction is shallow, the constructor assumes ownership of any pointers.
