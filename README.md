# llvm-pass-skeleton

A completely useless LLVM pass.
It's for LLVM 9.

Build:

    $ cd llvm-pass-skeleton
    $ mkdir build
    $ cd build
    $ cmake ..
    $ make
    $ cd ..

Run:

    $ clang -S -emit-llvm foo.c
    $ opt -load build/skeleton/libSkeletonPass.* -skeleton -S foo.ll % to generate trace

Run branch selection with skull:

    $ clang -c rtlib.c % to generate object file for the runtime lib
    $ clang -S -emit-llvm foo.c
    $ opt -load build/skeleton/libSkeletonPass.* -skull -S foo.ll > foo-mod.ll % to select trace 
    $ clang -c foo-mod.ll % generate object file for the modified code
    $ clang foo-mod.o rtlib.o % create executable with runtime lib
