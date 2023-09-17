This repo tests the MSVC STL in a Windows x64 kernel, with the libc++ test suite.
This is for research purposes for wg21 freestanding papers.

At time of writing, this project has lots of hard coded paths to SDKs and toolchains.  You may need to tweak these.

Building:
`ninja asm` will build all the tests, and verify that the generated binaries aren't using floating point in unexpected ways.

Test machine setup:
1. Turn off secure boot in your bios / UEFI.
1. Enable test signing by running `bcdedit /set TESTSIGNING ON` as an administrator
1. Run the provided `genCert.bat` to generate a code signing certificate and install it on your local machine.

Testing:
`ninja check` will run the tests on the current machine.  You'll need admin privileges for this.