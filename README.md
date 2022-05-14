# simple8b-timeseries-compression

C++ implementation of Simple8b compression & decompression algorithms for integer time series. Also includes delta encoding/decoding and zig-zag encoding/decoding routines.

Adapted from https://github.com/lemire/FastPFor (Apache License Version 2.0).

Modified to make methods generic over integer bit-width and support longer arrays (up to 2^64 in length).

Coming soon:
- test cases, example usage, and documentation
- support/examples for invoking via FFI from JavaScript (WebAssembly), Python, and C#
