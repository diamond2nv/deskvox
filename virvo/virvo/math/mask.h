#pragma once

#include "simd/vec.h"
#include "vec4.h"

#include "../vvforceinline.h"

#include "../mem/align.h"


namespace virvo
{

namespace math
{

template < typename T >
class CACHE_ALIGN base_mask;


template < >
class CACHE_ALIGN base_mask< __m128 >
{
public:
  typedef __m128 value_type;
  value_type value;

  VV_FORCE_INLINE base_mask(value_type const& m)
    : value(m)
  {
  }

  VV_FORCE_INLINE base_mask(Vec const& m)
    : value(m)
  {
  }

  VV_FORCE_INLINE operator value_type() const
  {
    return value;
  }

  VV_FORCE_INLINE operator sse_vec() const
  {
    return value;
  }
};


typedef base_mask< __m128 > sse_mask;
typedef sse_mask Mask;


/* Vec */

VV_FORCE_INLINE sse_mask operator<(sse_vec const& u, sse_vec const& v)
{
  return _mm_cmplt_ps(u, v);
}

VV_FORCE_INLINE sse_mask operator>(sse_vec const& u, sse_vec const& v)
{
  return _mm_cmpgt_ps(u, v);
}

VV_FORCE_INLINE sse_mask operator<=(sse_vec const& u, sse_vec const& v)
{
  return _mm_cmple_ps(u, v);
}

VV_FORCE_INLINE sse_mask operator>=(sse_vec const& u, sse_vec const& v)
{
  return _mm_cmpge_ps(u, v);
}

VV_FORCE_INLINE sse_mask operator==(sse_vec const& u, sse_vec const& v)
{
  return _mm_cmpeq_ps(u, v);
}

VV_FORCE_INLINE sse_mask operator!=(sse_vec const& u, sse_vec const& v)
{
  return _mm_cmpneq_ps(u, v);
}

VV_FORCE_INLINE sse_mask operator&&(sse_vec const& u, sse_vec const& v)
{
  return _mm_and_ps(u, v);
}


VV_FORCE_INLINE bool any(sse_mask const& m)
{
  return _mm_movemask_ps(m) != 0;
}

VV_FORCE_INLINE bool all(sse_mask const& m)
{
  return _mm_movemask_ps(m) == 0xF;
}

#if 1
VV_FORCE_INLINE sse_vec if_else(sse_vec const& ifexpr, sse_vec const& elseexpr, sse_mask const& mask)
{
  return _mm_or_ps(_mm_and_ps(mask, ifexpr), _mm_andnot_ps(mask, elseexpr));
}
#endif

VV_FORCE_INLINE sse_vec neg(sse_vec const& v, sse_mask const& mask)
{
  return if_else(-v, 0.0f, mask);
}

VV_FORCE_INLINE sse_vec add(sse_vec const& u, sse_vec const& v, sse_mask const& mask)
{
  return if_else(u + v, 0.0f, mask);
}

VV_FORCE_INLINE float sub(float u, float v, float /* mask */)
{
  return u - v;
}

VV_FORCE_INLINE sse_vec sub(sse_vec const& u, sse_vec const& v, sse_mask const& mask)
{
  return if_else(u - v, 0.0f, mask);
}

VV_FORCE_INLINE sse_vec mul(sse_vec const& u, sse_vec const& v, sse_mask const& mask)
{
  return if_else(u * v, 0.0f, mask);
}

VV_FORCE_INLINE sse_vec div(sse_vec const& u, sse_vec const& v, sse_mask const& mask)
{
  return if_else(u / v, 0.0f, mask);
}

VV_FORCE_INLINE sse_vec lt(sse_vec const& u, sse_vec const& v, sse_mask const& mask)
{
  return if_else(u < v, 0.0f, mask);
}

VV_FORCE_INLINE sse_vec gt(sse_vec const& u, sse_vec const& v, sse_mask const& mask)
{
  return if_else(u > v, 0.0f, mask);
}

VV_FORCE_INLINE sse_vec le(sse_vec const& u, sse_vec const& v, sse_mask const& mask)
{
  return if_else(u <= v, 0.0f, mask);
}

VV_FORCE_INLINE sse_vec ge(sse_vec const& u, sse_vec const& v, sse_mask const& mask)
{
  return if_else(u >= v, 0.0f, mask);
}

VV_FORCE_INLINE sse_vec eq(sse_vec const& u, sse_vec const& v, sse_mask const& mask)
{
  return if_else(u == v, 0.0f, mask);
}

VV_FORCE_INLINE sse_vec neq(sse_vec const& u, sse_vec const& v, sse_mask const& mask)
{
  return if_else(u != v, 0.0f, mask);
}

VV_FORCE_INLINE void store(sse_vec const& v, float dst[4], sse_mask const& mask)
{
  sse_vec tmp = if_else(v, 0.0f, mask);
  store(tmp, dst);
}


/* Vec3 */

template <typename T, typename M>
VV_FORCE_INLINE base_vec3<T> neg(base_vec3<T> const& v, M const& mask)
{
  return base_vec3<T>(neg(v.x, mask), neg(v.y, mask), neg(v.z, mask));
}

template <typename T, typename M>
VV_FORCE_INLINE base_vec3<T> add(base_vec3<T> const& u, base_vec3<T> const& v, M const& mask)
{
  return base_vec3<T>(add(u.x, v.x, mask), add(u.y, v.y, mask), add(u.z, v.z, mask));
}

template <typename T, typename M>
VV_FORCE_INLINE base_vec3<T> sub(base_vec3<T> const& u, base_vec3<T> const& v, M const& mask)
{
  return base_vec3<T>(sub(u.x, v.x, mask), sub(u.y, v.y, mask), sub(u.z, v.z, mask));
}

template <typename T, typename M>
VV_FORCE_INLINE base_vec3<T> mul(base_vec3<T> const& u, base_vec3<T> const& v, M const& mask)
{
  return base_vec3<T>(mul(u.x, v.x, mask), mul(u.y, v.y, mask), mul(u.z, v.z, mask));
}

template <typename T, typename M>
VV_FORCE_INLINE base_vec3<T> div(base_vec3<T> const& u, base_vec3<T> const& v, M const& mask)
{
  return base_vec3<T>(div(u.x, v.x, mask), div(u.y, v.y, mask), div(u.z, v.z, mask));
}

template <typename T, typename M>
VV_FORCE_INLINE base_vec3<T> add(base_vec3<T> const& v, T const& s, M const& mask)
{
  return base_vec3<T>(add(v.x, s, mask), add(v.y, s, mask), add(v.z, s, mask));
}

template <typename T, typename M>
VV_FORCE_INLINE base_vec3<T> sub(base_vec3<T> const& v, T const& s, M const& mask)
{
  return base_vec3<T>(sub(v.x, s, mask), sub(v.y, s, mask), sub(v.z, s, mask));
}

template <typename T, typename M>
VV_FORCE_INLINE base_vec3<T> mul(base_vec3<T> const& v, T const& s, M const& mask)
{
  return base_vec3<T>(mul(v.x, s, mask), mul(v.y, s, mask), mul(v.z, s, mask));
}

template <typename T, typename M>
VV_FORCE_INLINE base_vec3<T> div(base_vec3<T> const& v, T const& s, M const& mask)
{
  return base_vec3<T>(div(v.x, s, mask), div(v.y, s, mask), div(v.z, s, mask));
}

template <typename T, typename M>
VV_FORCE_INLINE base_vec3<T> add(T const& s, base_vec3<T> const& v, M const& mask)
{
  return base_vec3<T>(add(s, v.x, mask), add(s, v.y, mask), add(s, v.z, mask));
}

template <typename T, typename M>
VV_FORCE_INLINE base_vec3<T> sub(T const& s, base_vec3<T> const& v, M const& mask)
{
  return base_vec3<T>(sub(s, v.x, mask), sub(s, v.y, mask), sub(s, v.z, mask));
}

template <typename T, typename M>
VV_FORCE_INLINE base_vec3<T> mul(T const& s, base_vec3<T> const& v, M const& mask)
{
  return base_vec3<T>(mul(s, v.x, mask), mul(s, v.y, mask), sub(s, v.z, mask));
}

template <typename T, typename M>
VV_FORCE_INLINE base_vec3<T> div(T const& s, base_vec3<T> const& v, M const& mask)
{
  return base_vec3<T>(div(s, v.x, mask), div(s, v.y, mask), div(s, v.z, mask));
}


/* Vec4 */

template <typename T, typename M>
VV_FORCE_INLINE base_vec4<T> neg(base_vec4<T> const& v, M const& mask)
{
  return base_vec4<T>(neg(v.x, mask), neg(v.y, mask), neg(v.z, mask), neg(v.w, mask));
}

template <typename T, typename M>
VV_FORCE_INLINE base_vec4<T> add(base_vec4<T> const& u, base_vec4<T> const& v, M const& mask)
{
  return base_vec4<T>(add(u.x, v.x, mask), add(u.y, v.y, mask), add(u.z, v.z, mask), add(u.w, v.w, mask));
}

template <typename T, typename M>
VV_FORCE_INLINE base_vec4<T> sub(base_vec4<T> const& u, base_vec4<T> const& v, M const& mask)
{
  return base_vec4<T>(sub(u.x, v.x, mask), sub(u.y, v.y, mask), sub(u.z, v.z, mask), sub(u.w, v.w, mask));
}

template <typename T, typename M>
VV_FORCE_INLINE base_vec4<T> mul(base_vec4<T> const& u, base_vec4<T> const& v, M const& mask)
{
  return base_vec4<T>(mul(u.x, v.x, mask), mul(u.y, v.y, mask), mul(u.z, v.z, mask), mul(u.w, v.w, mask));
}

template <typename T, typename M>
VV_FORCE_INLINE base_vec4<T> div(base_vec4<T> const& u, base_vec4<T> const& v, M const& mask)
{
  return base_vec4<T>(div(u.x, v.x, mask), div(u.y, v.y, mask), div(u.z, v.z, mask), div(u.w, v.w, mask));
}

template <typename T, typename M>
VV_FORCE_INLINE base_vec4<T> add(base_vec4<T> const& v, T const& s, M const& mask)
{
  return base_vec4<T>(add(v.x, s, mask), add(v.y, s, mask), add(v.z, s, mask), add(v.w, s, mask));
}

template <typename T, typename M>
VV_FORCE_INLINE base_vec4<T> sub(base_vec4<T> const& v, T const& s, M const& mask)
{
  return base_vec4<T>(sub(v.x, s, mask), sub(v.y, s, mask), sub(v.z, s, mask), sub(v.w, s, mask));
}

template <typename T, typename M>
VV_FORCE_INLINE base_vec4<T> mul(base_vec4<T> const& v, T const& s, M const& mask)
{
  return base_vec4<T>(mul(v.x, s, mask), mul(v.y, s, mask), mul(v.z, s, mask), mul(v.w, s, mask));
}

VV_FORCE_INLINE base_vec4< float > mul(base_vec4< float > const& v, float s, float /* mask */)
{
  return v * s;
}

template <typename T, typename M>
VV_FORCE_INLINE base_vec4<T> div(base_vec4<T> const& v, T const& s, M const& mask)
{
  return base_vec4<T>(div(v.x, s, mask), div(v.y, s, mask), div(v.z, s, mask), div(v.w, s, mask));
}

template <typename T, typename M>
VV_FORCE_INLINE base_vec4<T> add(T const& s, base_vec4<T> const& v, M const& mask)
{
  return base_vec4<T>(add(s, v.x, mask), add(s, v.y, mask), add(s, v.z, mask), add(s, v.w, mask));
}

template <typename T, typename M>
VV_FORCE_INLINE base_vec4<T> sub(T const& s, base_vec4<T> const& v, M const& mask)
{
  return base_vec4<T>(sub(s, v.x, mask), sub(s, v.y, mask), sub(s, v.z, mask), sub(s, v.w, mask));
}

template <typename T, typename M>
VV_FORCE_INLINE base_vec4<T> mul(T const& s, base_vec4<T> const& v, M const& mask)
{
  return base_vec4<T>(mul(s, v.x, mask), mul(s, v.y, mask), mul(s, v.z, mask), mul(s, v.w, mask));
}

template <typename T, typename M>
VV_FORCE_INLINE base_vec4<T> div(T const& s, base_vec4<T> const& v, M const& mask)
{
  return base_vec4<T>(div(s, v.x, mask), div(s, v.y, mask), div(s, v.z, mask), div(s, v.w, mask));
}


} // math

} // virvo

