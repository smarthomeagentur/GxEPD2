#ifndef _CS_TYPE_H_
#define _CS_TYPE_H_

enum CsType {
    NONE = 0,
    MASTER = 1,
    SLAVE = 2,
    MASTER_SLAVE = 3
};

inline CsType operator|(CsType a, CsType b)
{
    return static_cast<CsType>(static_cast<int>(a) | static_cast<int>(b));
}

inline CsType operator&(CsType a, CsType b)
{
    return static_cast<CsType>(static_cast<int>(a) & static_cast<int>(b));
}

#endif