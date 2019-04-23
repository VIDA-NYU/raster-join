%MappedType NPArray
{
%TypeHeaderCode
#include <interface/NPArray.hpp>
#include <iostream>
#define NPY_NO_DEPRECATED_API NPY_1_7_API_VERSION
#include <numpy/arrayobject.h>
%End

%ConvertFromTypeCode
    import_array();
    int typenum;
    switch(sipCpp->type) {
        case NP_Int:
            typenum = NPY_INT32;
            break;

        case NP_UInt:
            typenum = NPY_UINT32;
            break;

        case NP_Float:
            typenum = NPY_FLOAT32;
            break;

        default:
            // "should not come here";
            return 0;
            break;        
    }

    npy_intp dims = sipCpp->size;
    PyObject *arr = PyArray_SimpleNewFromData(1,&dims,typenum,sipCpp->data);
    if (!arr)
        return 0;

    return arr;
%End

%ConvertToTypeCode
    // no idea why this function is called twice with the first time being an error!!!!
    if (!sipIsErr) {
        PyErr_Clear();
                return (
#if PY_MAJOR_VERSION < 3
                !PyString_Check(sipPy)
#endif
                !PyUnicode_Check(sipPy));
    }


    import_array();

    int typenum = PyArray_TYPE((PyArrayObject *)sipPy);
    PyArrayObject *hostmem = (PyArrayObject *)PyArray_FROM_OTF(sipPy,typenum,NPY_ARRAY_C_CONTIGUOUS | NPY_ARRAY_ALIGNED | NPY_ARRAY_ENSUREARRAY);
    
    if(hostmem == NULL){
        Py_XDECREF(hostmem);
    } else {
        // Get a pointer to the data as C-types
        NPArray *arr = new NPArray();
        arr->data = (void*)PyArray_DATA(hostmem);
        arr->size = PyArray_SHAPE(hostmem)[0];
        switch(typenum) {
            case NPY_FLOAT32:
                arr->type = NP_Float;
                break;

            case NPY_INT32:
                arr->type = NP_Int;
                break;

            case NPY_UINT32:
                arr->type = NP_UInt;
                break;

            default:
                // should not come here
                Py_XDECREF(hostmem);
                arr->size = 0;
                break;       
        }
        *sipCppPtr = arr;
    }

    return sipGetState(sipTransferObj);
%End
};
