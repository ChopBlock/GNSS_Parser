#pragma once

#define GP_NAMESPACE_BEGIN namespace gp{ 
#define GP_NAMESPACE_END }

#define GP_BUILD_SHARED_LIB
#define GP_EXPORT
#define GP_ERROR -1

#if defined(GP_BUILD_SHARED_LIB)
    #if defined(_MSC_VER) && defined(GP_EXPORT)
        #define GP_LIB  __declspec(dllexport)
        #else
        #define GP_LIB  __declspec(dllimport)
    #endif
    #else
        #define GP_LIB
#endif

