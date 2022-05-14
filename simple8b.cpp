/* 
    Adapted from https://github.com/lemire/FastPFor (Apache License Version 2.0)

    Implements Simple8b integer compression/decompression as described in original paper:
        Vo Ngoc Anh, Alistair Moffat: Index compression using 64-bit words
        Softw., Pract. Exper. 40(2): 131-147 (2010)

    Notable changes:
        - C++ templates used to make methods generic over integer bit-width
        - support longer arrays via 64 bit length arguments
*/

const uint8_t SIMPLE8B_SELECTOR_BITS = 4; // number of bits used by Simple8b algorithm to indicate packing scheme

template <typename T>
static void WriteBits(uint64_t *out, const T value, const uint32_t numBits)
{
    *out = (*out << numBits) | value;
}

template <uint64_t numIntegers, uint32_t numBitsPerInt, typename T>
static bool TryPackFast(const T *n)
{
    if (numBitsPerInt >= 32)
        return true;
    for (uint64_t i = 0; i < numIntegers; i++)
    {
        if (n[i] >= (1ULL << numBitsPerInt))
            return false;
    }
    return true;
}

template <uint64_t numIntegers, uint32_t numBitsPerInt, typename T>
static bool TryPackCareful(const T *n, uint64_t maxIntegers)
{
    if (numBitsPerInt >= 32)
        return true;
    const uint64_t minv = (maxIntegers < numIntegers) ? maxIntegers : numIntegers;
    for (uint64_t i = 0; i < minv; i++)
    {
        if (n[i] >= (1ULL << numBitsPerInt))
            return false;
    }
    return true;
}

template <uint32_t numIntegers, uint32_t numBitsPerInt, typename T>
static void UnpackFast(T *&out, const uint64_t *&in)
{
    const uint64_t mask = (1ULL << numBitsPerInt) - 1;
    if (numBitsPerInt < 32)
    {
        for (uint32_t k = 0; k < numIntegers; ++k)
        {
            *(out++) = static_cast<T>(in[0] >> (64 - SIMPLE8B_SELECTOR_BITS - numBitsPerInt - k * numBitsPerInt)) & mask;
        }
    }
    else
    {
        for (uint32_t k = 0; k < numIntegers; ++k)
        {
            *(out++) = static_cast<T>(in[0] >> (64 - SIMPLE8B_SELECTOR_BITS - numBitsPerInt - k * numBitsPerInt)) & mask;
        }
    }
    ++in;
}

template <uint32_t numBitsPerInt, typename T>
static void UnpackCareful(uint32_t numIntegers, T *&out, const uint64_t *&in)
{
    const uint64_t mask = (1ULL << numBitsPerInt) - 1;
    if (numBitsPerInt < 32)
    {
        for (uint32_t k = 0; k < numIntegers; ++k)
        {
            *(out++) = static_cast<T>(
                           in[0] >> (64 - SIMPLE8B_SELECTOR_BITS - numBitsPerInt - k * numBitsPerInt)) &
                       mask;
        }
    }
    else
    {
        for (uint32_t k = 0; k < numIntegers; ++k)
        {
            *(out++) = static_cast<T>(
                           in[0] >> (64 - SIMPLE8B_SELECTOR_BITS - numBitsPerInt - k * numBitsPerInt)) &
                       mask;
        }
    }
    ++in;
}

static uint32_t GetSelectorNum(const uint64_t *const in)
{
    return static_cast<uint32_t>((*in) >> (64 - SIMPLE8B_SELECTOR_BITS));
}

template <typename T>
uint64_t Simple8bEncode(T *input, uint64_t inputLength, uint64_t *out)
{
    uint32_t NumberOfValuesCoded = 0;
    const uint64_t *const initout = out;
    size_t ValuesRemaining(inputLength);

    while (ValuesRemaining >= 240)
    {
        if (TryPackFast<120, 0>(input))
        {
            if (TryPackFast<120, 0>(input + 120))
            {
                NumberOfValuesCoded = 240;
                out[0] = 0;
                input += NumberOfValuesCoded;
            }
            else
            {
                NumberOfValuesCoded = 120;
                out[0] = 1ULL << (64 - SIMPLE8B_SELECTOR_BITS);
                input += NumberOfValuesCoded;
            }
        }
        else if (TryPackFast<60, 1>(input))
        {
            out[0] = 2;
            NumberOfValuesCoded = 60;
            for (uint32_t i = 0; i < 60; i++)
            {
                WriteBits(out, *input++, 1);
            }
            out[0] <<= 64 - SIMPLE8B_SELECTOR_BITS - 1 * 60;
        }
        else if (TryPackFast<30, 2>(input))
        {
            out[0] = 3;
            NumberOfValuesCoded = 30;
            for (uint32_t i = 0; i < 30; i++)
                WriteBits(out, *input++, 2);
            out[0] <<= 64 - SIMPLE8B_SELECTOR_BITS - 2 * 30;
        }
        else if (TryPackFast<20, 3>(input))
        {
            out[0] = 4;
            NumberOfValuesCoded = 20;
            for (uint32_t i = 0; i < 20; i++)
                WriteBits(out, *input++, 3);
            out[0] <<= 64 - SIMPLE8B_SELECTOR_BITS - 3 * 20;
        }
        else if (TryPackFast<15, 4>(input))
        {
            out[0] = 5;
            NumberOfValuesCoded = 15;
            for (uint32_t i = 0; i < 15; i++)
                WriteBits(out, *input++, 4);
            out[0] <<= 64 - SIMPLE8B_SELECTOR_BITS - 4 * 15;
        }
        else if (TryPackFast<12, 5>(input))
        {
            out[0] = 6;
            NumberOfValuesCoded = 12;
            for (uint32_t i = 0; i < NumberOfValuesCoded; i++)
                WriteBits(out, *input++, 5);
            out[0] <<= 64 - SIMPLE8B_SELECTOR_BITS - 5 * 12;
        }
        else if (TryPackFast<10, 6>(input))
        {
            out[0] = 7;
            NumberOfValuesCoded = 10;
            for (uint32_t i = 0; i < NumberOfValuesCoded; i++)
                WriteBits(out, *input++, 6);
            out[0] <<= 64 - SIMPLE8B_SELECTOR_BITS - 6 * 10;
        }
        else if (TryPackFast<8, 7>(input))
        {
            out[0] = 8;
            NumberOfValuesCoded = 8;
            for (uint32_t i = 0; i < NumberOfValuesCoded; i++)
                WriteBits(out, *input++, 7);
            out[0] <<= 64 - SIMPLE8B_SELECTOR_BITS - 7 * 8;
        }
        else if (TryPackFast<7, 8>(input))
        {
            out[0] = 9;
            NumberOfValuesCoded = 7;
            for (uint32_t i = 0; i < NumberOfValuesCoded; i++)
                WriteBits(out, *input++, 8);
            out[0] <<= 64 - SIMPLE8B_SELECTOR_BITS - 8 * 7;
        }
        else if (TryPackFast<6, 10>(input))
        {
            out[0] = 10;
            NumberOfValuesCoded = 6;
            for (uint32_t i = 0; i < NumberOfValuesCoded; i++)
                WriteBits(out, *input++, 10);
            out[0] <<= 64 - SIMPLE8B_SELECTOR_BITS - 10 * 6;
        }
        else if (TryPackFast<5, 12>(input))
        {
            out[0] = 11;
            NumberOfValuesCoded = 5;
            for (uint32_t i = 0; i < NumberOfValuesCoded; i++)
                WriteBits(out, *input++, 12);
            out[0] <<= 64 - SIMPLE8B_SELECTOR_BITS - 12 * NumberOfValuesCoded;
        }
        else if (TryPackFast<4, 15>(input))
        {
            out[0] = 12;
            NumberOfValuesCoded = 4;
            for (uint32_t i = 0; i < NumberOfValuesCoded; i++)
                WriteBits(out, *input++, 15);
            out[0] <<= 64 - SIMPLE8B_SELECTOR_BITS - 15 * 4;
        }
        else if (TryPackFast<3, 20>(input))
        {
            out[0] = 13;
            NumberOfValuesCoded = 3;
            for (uint32_t i = 0; i < NumberOfValuesCoded; i++)
                WriteBits(out, *input++, 20);
            out[0] <<= 64 - SIMPLE8B_SELECTOR_BITS - 20 * 3;
        }
        else if (TryPackFast<2, 30>(input))
        {
            out[0] = 14;
            NumberOfValuesCoded = 2;
            for (uint32_t i = 0; i < NumberOfValuesCoded; i++)
                WriteBits(out, *input++, 30);
            out[0] <<= 64 - SIMPLE8B_SELECTOR_BITS - 30 * 2;
        }
        else if (TryPackFast<1, 60>(input))
        {
            out[0] = 15;
            NumberOfValuesCoded = 1;
            for (uint32_t i = 0; i < NumberOfValuesCoded; i++)
                WriteBits(out, *input++, 60);
            out[0] <<= 64 - SIMPLE8B_SELECTOR_BITS - 60 * 1;
        }
        else
        {
            // throw std::logic_error("shouldn't happen");
        }
        ++out;

        ValuesRemaining -= NumberOfValuesCoded;
    }
    while (ValuesRemaining > 0)
    {
        if (TryPackCareful<240, 0>(input, ValuesRemaining))
        {
            NumberOfValuesCoded = (ValuesRemaining < 240)
                                      ? static_cast<uint32_t>(ValuesRemaining)
                                      : 240;
            out[0] = 0;
            input += NumberOfValuesCoded;
        }
        else if (TryPackCareful<120, 0>(input, ValuesRemaining))
        {
            NumberOfValuesCoded = (ValuesRemaining < 120)
                                      ? static_cast<uint32_t>(ValuesRemaining)
                                      : 120;
            out[0] = 1ULL << (64 - SIMPLE8B_SELECTOR_BITS);
            input += NumberOfValuesCoded;
        }
        else if (TryPackCareful<60, 1>(input, ValuesRemaining))
        {
            out[0] = 2;
            NumberOfValuesCoded =
                (ValuesRemaining < 60) ? static_cast<uint32_t>(ValuesRemaining) : 60;
            for (uint32_t i = 0; i < NumberOfValuesCoded; i++)
            {
                WriteBits(out, *input++, 1);
            }
            out[0] <<= 64 - SIMPLE8B_SELECTOR_BITS - 1 * NumberOfValuesCoded;
        }
        else if (TryPackCareful<30, 2>(input, ValuesRemaining))
        {
            out[0] = 3;
            NumberOfValuesCoded =
                (ValuesRemaining < 30) ? static_cast<uint32_t>(ValuesRemaining) : 30;
            for (uint32_t i = 0; i < NumberOfValuesCoded; i++)
                WriteBits(out, *input++, 2);
            out[0] <<= 64 - SIMPLE8B_SELECTOR_BITS - 2 * NumberOfValuesCoded;
        }
        else if (TryPackCareful<20, 3>(input, ValuesRemaining))
        {
            out[0] = 4;
            NumberOfValuesCoded =
                (ValuesRemaining < 20) ? static_cast<uint32_t>(ValuesRemaining) : 20;
            for (uint32_t i = 0; i < NumberOfValuesCoded; i++)
                WriteBits(out, *input++, 3);
            out[0] <<= 64 - SIMPLE8B_SELECTOR_BITS - 3 * NumberOfValuesCoded;
        }
        else if (TryPackCareful<15, 4>(input, ValuesRemaining))
        {
            out[0] = 5;
            NumberOfValuesCoded =
                (ValuesRemaining < 15) ? static_cast<uint32_t>(ValuesRemaining) : 15;
            for (uint32_t i = 0; i < NumberOfValuesCoded; i++)
                WriteBits(out, *input++, 4);
            out[0] <<= 64 - SIMPLE8B_SELECTOR_BITS - 4 * NumberOfValuesCoded;
        }
        else if (TryPackCareful<12, 5>(input, ValuesRemaining))
        {
            out[0] = 6;
            NumberOfValuesCoded =
                (ValuesRemaining < 12) ? static_cast<uint32_t>(ValuesRemaining) : 12;
            for (uint32_t i = 0; i < NumberOfValuesCoded; i++)
                WriteBits(out, *input++, 5);
            out[0] <<= 64 - SIMPLE8B_SELECTOR_BITS - 5 * NumberOfValuesCoded;
        }
        else if (TryPackCareful<10, 6>(input, ValuesRemaining))
        {
            out[0] = 7;
            NumberOfValuesCoded =
                (ValuesRemaining < 10) ? static_cast<uint32_t>(ValuesRemaining) : 10;
            for (uint32_t i = 0; i < NumberOfValuesCoded; i++)
                WriteBits(out, *input++, 6);
            out[0] <<= 64 - SIMPLE8B_SELECTOR_BITS - 6 * NumberOfValuesCoded;
        }
        else if (TryPackCareful<8, 7>(input, ValuesRemaining))
        {
            out[0] = 8;
            NumberOfValuesCoded =
                (ValuesRemaining < 8) ? static_cast<uint32_t>(ValuesRemaining) : 8;
            for (uint32_t i = 0; i < NumberOfValuesCoded; i++)
                WriteBits(out, *input++, 7);
            out[0] <<= 64 - SIMPLE8B_SELECTOR_BITS - 7 * NumberOfValuesCoded;
        }
        else if (TryPackCareful<7, 8>(input, ValuesRemaining))
        {
            out[0] = 9;
            NumberOfValuesCoded =
                (ValuesRemaining < 7) ? static_cast<uint32_t>(ValuesRemaining) : 7;
            for (uint32_t i = 0; i < NumberOfValuesCoded; i++)
                WriteBits(out, *input++, 8);
            out[0] <<= 64 - SIMPLE8B_SELECTOR_BITS - 8 * NumberOfValuesCoded;
        }
        else if (TryPackCareful<6, 10>(input, ValuesRemaining))
        {
            out[0] = 10;
            NumberOfValuesCoded =
                (ValuesRemaining < 6) ? static_cast<uint32_t>(ValuesRemaining) : 6;
            for (uint32_t i = 0; i < NumberOfValuesCoded; i++)
                WriteBits(out, *input++, 10);
            out[0] <<= 64 - SIMPLE8B_SELECTOR_BITS - 10 * NumberOfValuesCoded;
        }
        else if (TryPackCareful<5, 12>(input, ValuesRemaining))
        {
            out[0] = 11;
            NumberOfValuesCoded =
                (ValuesRemaining < 5) ? static_cast<uint32_t>(ValuesRemaining) : 5;
            for (uint32_t i = 0; i < NumberOfValuesCoded; i++)
                WriteBits(out, *input++, 12);
            out[0] <<= 64 - SIMPLE8B_SELECTOR_BITS - 12 * NumberOfValuesCoded;
        }
        else if (TryPackCareful<4, 15>(input, ValuesRemaining))
        {
            out[0] = 12;
            NumberOfValuesCoded =
                (ValuesRemaining < 4) ? static_cast<uint32_t>(ValuesRemaining) : 4;
            for (uint32_t i = 0; i < NumberOfValuesCoded; i++)
                WriteBits(out, *input++, 15);
            out[0] <<= 64 - SIMPLE8B_SELECTOR_BITS - 15 * NumberOfValuesCoded;
        }
        else if (TryPackCareful<3, 20>(input, ValuesRemaining))
        {
            out[0] = 13;
            NumberOfValuesCoded =
                (ValuesRemaining < 3) ? static_cast<uint32_t>(ValuesRemaining) : 3;
            for (uint32_t i = 0; i < NumberOfValuesCoded; i++)
                WriteBits(out, *input++, 20);
            out[0] <<= 64 - SIMPLE8B_SELECTOR_BITS - 20 * NumberOfValuesCoded;
        }
        else if (TryPackCareful<2, 30>(input, ValuesRemaining))
        {
            out[0] = 14;
            NumberOfValuesCoded =
                (ValuesRemaining < 2) ? static_cast<uint32_t>(ValuesRemaining) : 2;
            for (uint32_t i = 0; i < NumberOfValuesCoded; i++)
                WriteBits(out, *input++, 30);
            out[0] <<= 64 - SIMPLE8B_SELECTOR_BITS - 30 * NumberOfValuesCoded;
        }
        else if (TryPackCareful<1, 60>(input, ValuesRemaining))
        {
            out[0] = 15;
            NumberOfValuesCoded = (ValuesRemaining < 1) ? ValuesRemaining : 1;
            for (uint32_t i = 0; i < NumberOfValuesCoded; i++)
                WriteBits(out, *input++, 60);
            out[0] <<= 64 - SIMPLE8B_SELECTOR_BITS - 60 * NumberOfValuesCoded;
        }
        else
        {
            // throw std::logic_error("shouldn't happen");
        }

        ++out;

        ValuesRemaining -= NumberOfValuesCoded;
    }

    return (out)-initout;
}

template <typename T>
const uint64_t Simple8bDecode(uint64_t *input, uint64_t uncompressedLength, T *out)
{
    const uint64_t *in = input;
    const T *const end = out + uncompressedLength;
    const T *const initout = out;

    while (end > out + 240)
    {
        switch (GetSelectorNum(in))
        {
        case 0:
            UnpackFast<240, 0>(out, in);
            break;
        case 1:
            UnpackFast<120, 0>(out, in);
            break;
        case 2:
            UnpackFast<60, 1>(out, in);
            break;
        case 3:
            UnpackFast<30, 2>(out, in);
            break;
        case 4:
            UnpackFast<20, 3>(out, in);
            break;
        case 5:
            UnpackFast<15, 4>(out, in);
            break;
        case 6:
            UnpackFast<12, 5>(out, in);
            break;
        case 7:
            UnpackFast<10, 6>(out, in);
            break;
        case 8:
            UnpackFast<8, 7>(out, in);
            break;
        case 9:
            UnpackFast<7, 8>(out, in);
            break;
        case 10:
            UnpackFast<6, 10>(out, in);
            break;
        case 11:
            UnpackFast<5, 12>(out, in);
            break;
        case 12:
            UnpackFast<4, 15>(out, in);
            break;
        case 13:
            UnpackFast<3, 20>(out, in);
            break;
        case 14:
            UnpackFast<2, 30>(out, in);
            break;
        case 15:
            UnpackFast<1, 60>(out, in);
            break;
        default:
            break;
        }
    }
    while (end > out)
    {
        switch (GetSelectorNum(in))
        {
        case 0:
            UnpackCareful<0>(std::min<uint32_t>(static_cast<uint32_t>(end - out), 240), out, in);
            break;
        case 1:
            UnpackCareful<0>(std::min<uint32_t>(static_cast<uint32_t>(end - out), 120), out, in);
            break;
        case 2:
            UnpackCareful<1>(std::min<uint32_t>(static_cast<uint32_t>(end - out), 60), out, in);
            break;
        case 3:
            UnpackCareful<2>(std::min<uint32_t>(static_cast<uint32_t>(end - out), 30), out, in);
            break;
        case 4:
            UnpackCareful<3>(std::min<uint32_t>(static_cast<uint32_t>(end - out), 20), out, in);
            break;
        case 5:
            UnpackCareful<4>(std::min<uint32_t>(static_cast<uint32_t>(end - out), 15), out, in);
            break;
        case 6:
            UnpackCareful<5>(std::min<uint32_t>(static_cast<uint32_t>(end - out), 12), out, in);
            break;
        case 7:
            UnpackCareful<6>(std::min<uint32_t>(static_cast<uint32_t>(end - out), 10), out, in);
            break;
        case 8:
            UnpackCareful<7>(std::min<uint32_t>(static_cast<uint32_t>(end - out), 8), out, in);
            break;
        case 9:
            UnpackCareful<8>(std::min<uint32_t>(static_cast<uint32_t>(end - out), 7), out, in);
            break;
        case 10:
            UnpackCareful<10>(std::min<uint32_t>(static_cast<uint32_t>(end - out), 6), out, in);
            break;
        case 11:
            UnpackCareful<12>(std::min<uint32_t>(static_cast<uint32_t>(end - out), 5), out, in);
            break;
        case 12:
            UnpackCareful<15>(std::min<uint32_t>(static_cast<uint32_t>(end - out), 4), out, in);
            break;
        case 13:
            UnpackCareful<20>(std::min<uint32_t>(static_cast<uint32_t>(end - out), 3), out, in);
            break;
        case 14:
            UnpackCareful<30>(std::min<uint32_t>(static_cast<uint32_t>(end - out), 2), out, in);
            break;
        case 15:
            UnpackCareful<60>(1, out, in);
            break;
        default:
            break;
        }
    }

    // ASSERT(out < end + 240, out - end);
    return out - initout;
}

template <typename T>
void DeltaEncode(T *input, uint64_t length)
{
    for (uint64_t i = length - 1; i > 0; i--)
    {
        input[i] = input[i] - input[i - 1];
    }
}

template <typename T>
void DeltaDecode(T *input, uint64_t length)
{
    for (uint64_t i = 1; i < length; i++)
    {
        input[i] = input[i] + input[i - 1];
    }
}

template <typename T>
void ZigZagEncode(T *input, uint64_t length)
{
    T shift = (sizeof(T) * 8) - 1; // sizeof * 8 = # of bits
    for (uint64_t i = 0; i < length; i++)
    {
        input[i] = (input[i] << 1LL) ^ (input[i] >> shift);
    }
    return;
}

template <typename T>
void ZigZagDecode(T *input, uint64_t length)
{
    for (uint64_t i = 0; i < length; i++)
    {
        input[i] = (input[i] >> 1LL) ^ -(input[i] & 1LL);
    }
    return;
}