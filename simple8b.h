#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#if defined(_WIN32)
#define EXPORT __declspec(dllexport) /* compile as a Win32 DLL (C#) */
#elif defined(__EMSCRIPTEN__)
#define EXPORT EMSCRIPTEN_KEEPALIVE /* compile as WebAssembly (JavaScript) */
#else
#define EXPORT /* compile as a shared library (eg for Python) */
#endif

extern "C" /* prevent compiler from mangling function names */
{
    EXPORT uint64_t Simple8bEncode<>(uint64_t *input, uint64_t inputLength, uint64_t *output);
    EXPORT uint64_t Simple8bDecode<>(uint64_t *input, uint64_t outputLength, uint64_t *output);

    EXPORT void DeltaEncode(int64_t *input, uint64_t length);
    EXPORT void DeltaDecode(int64_t *input, uint64_t length);

    EXPORT void ZigZagEncode(int64_t *input, uint64_t length);
    EXPORT void ZigZagDecode(int64_t *input, uint64_t length);
}