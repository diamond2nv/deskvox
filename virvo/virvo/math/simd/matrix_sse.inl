#pragma once

#include "vec.h"
#include "../vec3.h"
#include "../vec4.h"

#include <ostream>

namespace virvo
{
namespace math
{


template < >
class CACHE_ALIGN Matrix< sse_vec >
{
public:

  typedef sse_vec row_type;

  inline Matrix< sse_vec >()
  {
  }

  inline Matrix< sse_vec >(virvo::Matrix const& m)
  {
    CACHE_ALIGN float row1[4];
    CACHE_ALIGN float row2[4];
    CACHE_ALIGN float row3[4];
    CACHE_ALIGN float row4[4];

    m.getRow(0, &row1[0], &row1[1], &row1[2], &row1[3]);
    m.getRow(1, &row2[0], &row2[1], &row2[2], &row2[3]);
    m.getRow(2, &row3[0], &row3[1], &row3[2], &row3[3]);
    m.getRow(3, &row4[0], &row4[1], &row4[2], &row4[3]);

    rows[0] = row1;
    rows[1] = row2;
    rows[2] = row3;
    rows[3] = row4;
  }

  inline operator virvo::Matrix() const
  {
    CACHE_ALIGN float row1[4];
    CACHE_ALIGN float row2[4];
    CACHE_ALIGN float row3[4];
    CACHE_ALIGN float row4[4];

    store(rows[0], &row1[0]);
    store(rows[1], &row2[0]);
    store(rows[2], &row3[0]);
    store(rows[3], &row4[0]);

    virvo::Matrix m;
    m.setRow(0, row1[0], row1[1], row1[2], row1[3]);
    m.setRow(1, row2[0], row2[1], row2[2], row2[3]);
    m.setRow(2, row3[0], row3[1], row3[2], row3[3]);
    m.setRow(3, row4[0], row4[1], row4[2], row4[3]);
    return m;
  }

  inline sse_vec row(size_t i) const
  {
    assert(i < 4);
    return rows[i];
  }

  inline void setRow(size_t i, sse_vec const& v)
  {
    assert(i < 4);
    rows[i] = v;
  }

  void identity()
  {
    rows[0] = sse_vec(1.0f, 0.0f, 0.0f, 0.0f);
    rows[1] = sse_vec(0.0f, 1.0f, 0.0f, 0.0f);
    rows[2] = sse_vec(0.0f, 0.0f, 1.0f, 0.0f);
    rows[3] = sse_vec(0.0f, 0.0f, 0.0f, 1.0f);
  }

  void transpose()
  {
    sse_vec tmp1 = _mm_unpacklo_ps(rows[0], rows[1]);
    sse_vec tmp2 = _mm_unpacklo_ps(rows[2], rows[3]);
    sse_vec tmp3 = _mm_unpackhi_ps(rows[0], rows[1]);
    sse_vec tmp4 = _mm_unpackhi_ps(rows[2], rows[3]);

    rows[0] = _mm_movelh_ps(tmp1, tmp2);
    rows[1] = _mm_movehl_ps(tmp2, tmp1);
    rows[2] = _mm_movelh_ps(tmp3, tmp4);
    rows[3] = _mm_movehl_ps(tmp4, tmp3);
  }

  void invert()
  {
    sse_vec cofactors[6];

    sse_vec tmpa = shuffle<3, 3, 3, 3>(rows[3], rows[2]);
    sse_vec tmpb = shuffle<2, 2, 2, 2>(rows[3], rows[2]);
    sse_vec tmp0 = shuffle<2, 2, 2, 2>(rows[2], rows[1]);
    sse_vec tmp1 = shuffle<0, 0, 0, 2>(tmpa, tmpa);
    sse_vec tmp2 = shuffle<0, 0, 0, 2>(tmpb, tmpb);
    sse_vec tmp3 = shuffle<3, 3, 3, 3>(rows[2], rows[1]);
    cofactors[0] = tmp0 * tmp1 - tmp2 * tmp3;

    tmpa = shuffle<3, 3, 3, 3>(rows[3], rows[2]);
    tmpb = shuffle<1, 1, 1, 1>(rows[3], rows[2]);
    tmp0 = shuffle<1, 1, 1, 1>(rows[2], rows[1]);
    tmp1 = shuffle<0, 0, 0, 2>(tmpa, tmpa);
    tmp2 = shuffle<0, 0, 0, 2>(tmpb, tmpb);
    tmp3 = shuffle<3, 3, 3, 3>(rows[2], rows[1]);
    cofactors[1] = tmp0 * tmp1 - tmp2 * tmp3;

    tmpa = shuffle<2, 2, 2, 2>(rows[3], rows[2]);
    tmpb = shuffle<1, 1, 1, 1>(rows[3], rows[2]);
    tmp0 = shuffle<1, 1, 1, 1>(rows[2], rows[1]);
    tmp1 = shuffle<0, 0, 0, 2>(tmpa, tmpa);
    tmp2 = shuffle<0, 0, 0, 2>(tmpb, tmpb);
    tmp3 = shuffle<2, 2, 2, 2>(rows[2], rows[1]);
    cofactors[2] = tmp0 * tmp1 - tmp2 * tmp3;

    tmpa = shuffle<3, 3, 3, 3>(rows[3], rows[2]);
    tmpb = shuffle<0, 0, 0, 0>(rows[3], rows[2]);
    tmp0 = shuffle<0, 0, 0, 0>(rows[2], rows[1]);
    tmp1 = shuffle<0, 0, 0, 2>(tmpa, tmpa);
    tmp2 = shuffle<0, 0, 0, 2>(tmpb, tmpb);
    tmp3 = shuffle<3, 3, 3, 3>(rows[2], rows[1]);
    cofactors[3] = tmp0 * tmp1 - tmp2 * tmp3;

    tmpa = shuffle<2, 2, 2, 2>(rows[3], rows[2]);
    tmpb = shuffle<0, 0, 0, 0>(rows[3], rows[2]);
    tmp0 = shuffle<0, 0, 0, 0>(rows[2], rows[1]);
    tmp1 = shuffle<0, 0, 0, 2>(tmpa, tmpa);
    tmp2 = shuffle<0, 0, 0, 2>(tmpb, tmpb);
    tmp3 = shuffle<2, 2, 2, 2>(rows[2], rows[1]);
    cofactors[4] = tmp0 * tmp1 - tmp2 * tmp3;

    tmpa = shuffle<1, 1, 1, 1>(rows[3], rows[2]);
    tmpb = shuffle<0, 0, 0, 0>(rows[3], rows[2]);
    tmp0 = shuffle<0, 0, 0, 0>(rows[2], rows[1]);
    tmp1 = shuffle<0, 0, 0, 2>(tmpa, tmpa);
    tmp2 = shuffle<0, 0, 0, 2>(tmpb, tmpb);
    tmp3 = shuffle<1, 1, 1, 1>(rows[2], rows[1]);
    cofactors[5] = tmp0 * tmp1 - tmp2 * tmp3;

    static sse_vec const& pmpm = sse_vec(1.0f, -1.0f, 1.0f, -1.0f);
    static sse_vec const& mpmp = sse_vec(-1.0f, 1.0f, -1.0f, 1.0f);

    sse_vec r01 = shuffle<0, 0, 0, 0>(rows[1], rows[0]);
    sse_vec v0 = shuffle<0, 2, 2, 2>(r01, r01);
    sse_vec r10 = shuffle<1, 1, 1, 1>(rows[1], rows[0]);
    sse_vec v1 = shuffle<0, 2, 2, 2>(r10, r10);
    r01 = shuffle<2, 2, 2, 2>(rows[1], rows[0]);
    sse_vec v2 = shuffle<0, 2, 2, 2>(r01, r01);
    r10 = shuffle<3, 3, 3, 3>(rows[1], rows[0]);
    sse_vec v3 = shuffle<0, 2, 2, 2>(r10, r10);

    sse_vec inv0 = mpmp * ((v1 * cofactors[0] - v2 * cofactors[1]) + v3 * cofactors[3]);
    sse_vec inv1 = pmpm * ((v0 * cofactors[0] - v2 * cofactors[3]) + v3 * cofactors[4]);
    sse_vec inv2 = mpmp * ((v0 * cofactors[1] - v1 * cofactors[3]) + v3 * cofactors[5]);
    sse_vec inv3 = pmpm * ((v0 * cofactors[2] - v1 * cofactors[4]) + v2 * cofactors[5]);
    sse_vec r = shuffle<0, 2, 0, 2>(shuffle<0, 0, 0, 0>(inv0, inv1), shuffle<0, 0, 0, 0>(inv2, inv3));

    sse_vec det = dot(rows[0], r);
    sse_vec rcp = fast::rcp<1>(det);

    rows[0] = inv0 * rcp;
    rows[1] = inv1 * rcp;
    rows[2] = inv2 * rcp;
    rows[3] = inv3 * rcp;
  }
private:
  row_type rows[4];
};

inline Matrix< sse_vec > operator*(Matrix< sse_vec > const& m, Matrix< sse_vec > const& n)
{
  Matrix< sse_vec > result;
  for (size_t i = 0; i < 4; ++i)
  {
    sse_vec row = shuffle<0, 0, 0, 0>(m.row(i)) * n.row(0);
    row += shuffle<1, 1, 1, 1>(m.row(i)) * n.row(1);
    row += shuffle<2, 2, 2, 2>(m.row(i)) * n.row(2);
    row += shuffle<3, 3, 3, 3>(m.row(i)) * n.row(3);
    result.setRow(i, row);
  }
  return result;
}

inline base_vec4< sse_vec > operator*(Matrix< sse_vec > const& m, base_vec4< sse_vec > const& v)
{
  Matrix< sse_vec > tmp;
  tmp.setRow(0, v.x);
  tmp.setRow(1, v.y);
  tmp.setRow(2, v.z);
  tmp.setRow(3, v.w);
  Matrix< sse_vec > res = m * tmp;
  return base_vec4< sse_vec >(res.row(0), res.row(1), res.row(2), res.row(3));
}


inline base_vec3< sse_vec > operator*(Matrix< sse_vec > const& m, base_vec3< sse_vec > const& v)
{
  base_vec4< sse_vec > tmp(v[0], v[1], v[2], 1);
  base_vec4< sse_vec > res = m * tmp;
  return base_vec3< sse_vec >(res[0] / res[3], res[1] / res[3], res[2] / res[3]);
}


template < typename T >
inline std::ostream& operator<<(std::ostream& out, Matrix< T > const& m)
{
  out << m.row(0) << "\n";
  out << m.row(1) << "\n";
  out << m.row(2) << "\n";
  out << m.row(3) << "\n";
  return out;
}

} // math
} // virvo
