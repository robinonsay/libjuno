#ifndef JUNO_MATH_VEC_H
#define JUNO_MATH_VEC_H

#include <cstddef>
#include "juno/module.h"

template<typename T, const size_t N, const size_t M>
struct JUNO_MATH_MAT_API_T;


#define JUNO_MATH_MAT_API_TEMPLATE(T, N, M) JUNO_MATH_MAT_API_T<T, N, M>
/**
    A N-Dimensional vector in a Frame
*/
template<typename T, const size_t N, const size_t M>
struct JUNO_MATH_MAT_T JUNO_MODULE_ROOT(JUNO_MATH_MAT_API_TEMPLATE(T, N, M),
    /// @brief  The Data for the vector
    T tData[N][M];
);

template<typename T, const size_t N>
using JUNO_MATH_FRAME_T = JUNO_MATH_MAT_T<T, N, 3>;

#define JUNO_MATH_MAT_TEMPLATE(T, N, M) JUNO_MATH_MAT_T<T, N, M>
#define JUNO_MATH_FRAME_TEMPLATE(T, N) JUNO_MATH_FRAME_T<T, N>
/**
    A N-Dimensional vector in a Frame
*/
template<typename T, const size_t N, const JUNO_MATH_FRAME_T<T,N> F>
struct JUNO_MATH_VEC_T JUNO_MODULE_DERIVE(JUNO_MATH_MAT_TEMPLATE(T, N, 1),
    using VEC_T = JUNO_MATH_MAT_TEMPLATE(T, N, 1);
    const JUNO_MATH_FRAME_TEMPLATE(T, N) tFrame{F};
    /// @brief  The Data for the vector
    VEC_T tData;
);


template<typename T, const size_t N, const size_t M>
struct JUNO_MATH_MAT_API_T
{
    JUNO_STATUS_T (*Get)(size_t iN, size_t iM, T& tRet);
    JUNO_STATUS_T (*Equals)(JUNO_MATH_MAT_T <T, N, M> &tMat1, JUNO_MATH_MAT_T <T, N, M> &tMat2, bool &bRet);
    JUNO_STATUS_T (*Test)(JUNO_MATH_MAT_T <T, N, M> &tMat1, JUNO_MATH_MAT_T <T, N, M> &tMat2, bool &bRet);
};

#endif
