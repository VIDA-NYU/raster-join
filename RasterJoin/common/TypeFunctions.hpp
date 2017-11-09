#ifndef TYPEFUNCTIONS_HPP
#define TYPEFUNCTIONS_HPP

#include <stdint.h>

inline uint32_t float2uint(float f) {
    uint32_t t(*((uint32_t*)&f));
    return t ^ ((-(t >> 31)) | 0x80000000);
}

inline float uint2float(uint32_t f) {
    uint32_t u = f ^ (((f >> 31) - 1) | 0x80000000);
    return *((float*) &u);
}

inline uint64_t double2uint(double f) {
    uint64_t t(*((uint64_t*) &f));
    return t ^ ((-(t >> 63ULL)) | 0x8000000000000000ULL);
}

inline double uint2double(uint64_t f) {
    uint64_t u = f ^ (((f >> 63ULL) - 1ULL) | 0x8000000000000000ULL);
    return *((double*) &u);
}

inline int64_t uint2long(uint64_t f) {
    uint64_t u = (f ^ 0x8000000000000000ULL);
    return *((int64_t*) &u);
}

inline uint64_t long2uint(int64_t f) {
    uint64_t t(*((uint64_t*) &f));
    return (t ^ 0x8000000000000000ULL);
}


#endif // TYPEFUNCTIONS_HPP
