#ifndef _CS_TYPE_H_
#define _CS_TYPE_H_

enum CsType
{
  CS_NONE = 0,
  CS_MASTER = 1,
  CS_SLAVE = 2,
  CS_MASTER_SLAVE = 3
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