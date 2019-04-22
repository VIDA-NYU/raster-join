#ifndef NPARRAY_HPP
#define NPARRAY_HPP

#include <stddef.h>

enum NpType {
    NP_Int,
    NP_UInt,
    NP_Float,
    NP_Undefined
};

struct NPArray {

    NPArray(): data(0), size(0), type(NP_Undefined) {}
    NPArray(const NPArray &old) {
        data = old.data;
        size = old.size;
        type = old.type;
    }
    NPArray(const NPArray *old) {
        data = old->data;
        size = old->size;
        type = old->type;
    }
    void *data;
    size_t size;
    NpType type;
};

#endif // NPARRAY_HPP
