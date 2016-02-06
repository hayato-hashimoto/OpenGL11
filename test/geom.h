/*
 * The MIT License (MIT)
 * 
 * Copyright (c) 2015-2016 Hayato Hashimoto <hayato.hashimoto@gmail.com>
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifndef GEOM_H
#define GEOM_H
#include <iostream>
#include <array>
#include <complex>
#include <cmath>

// armadillo matrix support
#ifdef USE_ARMADILLO
#include <armadillo>
#endif
namespace geom {
  /* field T */
  template <typename T>
  class quaternion {
    public:
    /* q  = w + xi + yj + zk */
    T w, x, y, z;
    inline quaternion<T> (const T& _w, const T& _x, const T& _y, const T& _z) : w(_w), x(_x), y(_y), z(_z) {};
    /* x ∈ R ↦ x + 0i + 0j + 0k ∈ H*/
    inline quaternion<T> (const T& _w) : w(_w), x(0), y(0), z(0) {};
    /* x + yi ∈ C ↦ x + yi + 0j + 0k ∈ H */ 
    inline quaternion<T> (const std::complex<T> a) : w(std::real(a)), x(std::imag(a)), y(0), z(0) {};
    inline quaternion<T> (const std::array<T, 3> vec) : w(0), x(vec[0]), y(vec[1]), z(vec[2]) {};
    /* (*this) * q2 */
    inline quaternion<T> operator *(const quaternion<T>& q2) const {
      return quaternion<T>(
          w * q2.w - x * q2.x - y * q2.y - z * q2.z,
          w * q2.x + x * q2.w + y * q2.z - z * q2.y,
          w * q2.y - x * q2.z + y * q2.w + z * q2.x,
          w * q2.z + x * q2.y - y * q2.x + z * q2.w
      );
    };
    inline quaternion<T> operator *(const T& s) const {
      return quaternion<T>(s * w, s * x, s * y, s * z);
    };
    inline quaternion<T> operator +(const quaternion<T>& q2) const {
      return quaternion<T>(
          w + q2.w,
          x + q2.x,
          y + q2.y,
          z + q2.z);
    };
    inline quaternion<T> operator -(const quaternion<T>& q2) const {
      return quaternion<T>(
          w - q2.w,
          x - q2.x,
          y - q2.y,
          z - q2.z);
    };
    inline quaternion<T> operator -() const {
      return quaternion<T>(-w, -x, -y, -z);
    };
    /* conjugation (use prefix star instead of postfix star) */
    inline quaternion<T> operator *() const {
      return quaternion<T>(w, -x, -y, -z);
    };
    inline quaternion<T> operator /(const T& s) const {
      return (1 / s) * (*this);
    };
    inline quaternion<T> operator /(const quaternion<T>& q) const {
      // dot(q, q) = |q|^2
      return (*this) * (*q) / dot(q, q);
    };
#ifdef USE_ARMADILLO
    inline operator typename arma::Col<T>::template fixed<4>() const {
      typename arma::Col<T>::template fixed<4> ret;
      ret[0] = w, ret[1] = x, ret[2] = y, ret[3] = z;
      return ret;
    };
#endif
  };
  template <typename T>
    inline quaternion<T> operator *(const T& s, const quaternion<T>& q) {
      return q * s;
    };
  template <typename T>
    inline quaternion<T> operator /(const T& s, const quaternion<T>& q) {
      return s * (*q) / dot(q, q);
    };

  /* write to text */
  template <typename T>
    std::ostream& operator <<(std::ostream& os, const quaternion<T>& q) {
      os << q.w << "+" << q.x << "i+" << q.y << "j+" << q.z << "k";
      return os;
    };
  
  /* 4D dot product */
  template <typename T>
  inline T dot(const quaternion<T>& q1, const quaternion<T>& q2) {
    return q1.w * q2.w + q1.x * q2.x + q1.y * q2.y + q1.z * q2.z;
  }

  /* 3D dot product of vectors (x, y, z) */
  template <typename T>
  inline T vdot(const quaternion<T>& q1, const quaternion<T>& q2) {
    return q1.x * q2.x + q1.y * q2.y + q1.z * q2.z;
  }

  /* 3D cross product of vectors (x, y, z) */
  template <typename T>
  inline quaternion<T> vcross(const quaternion<T>& q1, const quaternion<T>& q2) {
    return quaternion<T>
      (0,
       q1.y * q2.z - q1.z * q2.y,
       q1.z * q2.x - q1.x * q2.z,
       q1.x * q2.y - q1.y * q2.x);
  }
  
  /*
   * see: https://code.google.com/p/kri/wiki/Quaternions
   * faster q*v*(*q) under the assertion (abs(q) == 1 && v.w == 0)
   */
  template <typename T>
  inline quaternion<T> qrot(const quaternion<T>& q, const quaternion<T>& v) {
    return v + ((T) 2) * vcross(q, vcross(q, v) + q.w * v);
  }

  template <typename T>
    inline T abs(const quaternion<T>& q) {
      return sqrt(q.w * q.w + q.x * q.x + q.y * q.y + q.z * q.z);
    }

  template <typename T>
    inline quaternion<T> normalize(const quaternion<T> &q) {
      return q / abs(q);
    }

  template <typename T>
  inline quaternion<T> exp(const quaternion<T>& q) {
    return std::exp(q.w) * exp_pure(q);
  }

  // e with a pure quaternion exponent, evaluating to an unit quaternion
  // q.w is ignored as if being zero
  template <typename T>
  inline quaternion<T> exp_pure(const quaternion<T>& q) {
    quaternion<T> n(0, q.x, q.y, q.z);
    T alpha = abs(n);
    if (alpha == 0) {
      // e^0 == 1
      return quaternion<T>(1);
#ifdef QUATERNION_FIXED_POINT
     } else if (alpha > 1 || alpha < -1) {
       T s = sin(alpha);
       return quaternion<T>(cos(alpha), s * (q.x / alpha), s * (q.y / alpha), s * (q.z / alpha));
#endif
    } else {
      T s = sin(alpha) / alpha;
      return quaternion<T>(cos(alpha), s * q.x, s * q.y, s * q.z);
    }
  }

  // logarithm of an unit quaternion
  template <typename T>
    inline quaternion<T> log_unit(const quaternion<T>& q) {
      // ASSERT(abs(q) == 1);
      // pure component n
      quaternion<T> n(0, q.x, q.y, q.z);
      T size_n = abs(n);
      // use atan2 so that α can take value in range -π < α <= π
      T alpha = atan2(size_n, q.w);
      if (size_n == 0) {
        // log 1 == 0
        return quaternion<T>(0);
      } else {
        return alpha * n / size_n;
      }
    }

  template <typename T>
    inline quaternion<T> log(const quaternion<T>& q) {
      T s = abs(q);
      return quaternion<T>(std::log(s)) + log_unit(q/s);
    }

  /*
   * rotation & translate & scaling
   *
   * Rotation and scaling factors are separated in a quaternion and a scalar for the following reasons:
   *
   *  - Some interpolation methods such as "nlerp" involve (re-)normalization of quaternions.
   *  - Simple scaling should not cost four multiplications.
   */

  template <typename T>
  class transform {
    public:
      quaternion<T> pos, rot;
      T scale;
      transform(const quaternion<T>& _pos, const quaternion<T>& _rot, const T& _scale) : pos(_pos), rot(_rot), scale(_scale) { };
      // do nothing
      transform() : pos((T)0), rot((T)1), scale((T)1) {}
      /* perform transformation on given position/orientation of rigid body */
      inline quaternion<T> operator *(const quaternion<T>& q) {
        return qrot(rot, scale * q) + pos;
      };
      /* t1 * (t2 * q) == (t1 * t2) * q */
      inline transform<T> operator *(const transform<T>& t2) {
        return transform<T>((*this) * t2.pos, rot * t2.rot, scale * t2.scale);
      };
      /* linear interpolation */
      inline transform<T> operator *(const T& s) {
        return transform<T>(s * pos, s * rot, s * scale);
      };
      inline transform<T> operator +(const transform<T>& t2) {
        return transform<T>(pos + t2.pos, rot + t2.rot, scale + t2.scale);
      };
      inline transform<T> operator -(const transform<T>& t2) {
        return transform<T>(pos - t2.pos, rot - t2.rot, scale - t2.scale);
      };
      inline transform<T> operator -() {
        return transform<T>(-pos, -rot, -scale);
      };
#ifdef USE_ARMADILLO
      inline operator typename arma::Mat<T>::template fixed<4, 4> () {
        typename arma::Mat<T>::template fixed<4, 4> m;
        m(0, 0) = scale * (1 - 2 * (rot.y * rot.y + rot.z * rot.z));
        m(1, 0) = scale * (2 * (rot.x * rot.y - rot.z * rot.w));
        m(2, 0) = scale * (2 * (rot.x * rot.z + rot.y * rot.w));
        m(3, 0) = 0;

        m(0, 1) = scale * (2 * (rot.y * rot.x + rot.z * rot.w));
        m(1, 1) = scale * (1 - 2 * (rot.z * rot.z + rot.x * rot.x));
        m(2, 1) = scale * (2 * (rot.y * rot.z - rot.x * rot.w));
        m(3, 1) = 0;

        m(0, 2) = scale * (2 * (rot.z * rot.x - rot.y * rot.w));
        m(1, 2) = scale * (2 * (rot.z * rot.y + rot.x * rot.w));
        m(2, 2) = scale * (1 - 2 * (rot.x * rot.x + rot.y * rot.y));
        m(3, 2) = 0;

        m(0, 3) = pos.x;
        m(1, 3) = pos.y;
        m(2, 3) = pos.z;
        m(3, 3) = 1;
        return m;
      };
#endif
  };
  
  template <typename T>
  inline quaternion<T> operator *(const T& s, const transform<T>& t) {
    return t * s;
  }

  // q is a position
  template <typename T>
  inline quaternion<T> operator /(const quaternion<T>& q, const transform<T>& t) {
    return qrot(*(t.rot), (q - t.pos) / t.scale);
  }
  
  template <typename T>
  inline transform<T> operator /(const transform<T>& t2, const transform<T>& t1) {
    return transform<T>(t2.pos/t1, (*t1.rot) * t2.rot, t2.w / t1.w);
  }
  
  template <typename T>
  inline transform<T> scale(const T& s) {
    return transform<T>(quaternion<T>(0), quaternion<T>(1), s);
  }

  template <typename T>
  inline transform<T> translate(const T& x, const T& y, const T& z) {
    return transform<T>(quaternion<T>(0, x, y, z), quaternion<T>(1), 1);
  }
  
  template <typename T>
  inline transform<T> translate(const std::array<T, 3>& xyz) {
    return translate(xyz[0], xyz[1], xyz[2]);
  }
  
  template <typename T>
  inline transform<T> translate(const std::vector<T>& xyz) {
  // ASSERT_GE(xyz.size(), 3);
    return translate(xyz[0], xyz[1], xyz[2]);
  }
  
  template <typename T>
  inline transform<T> rotateX(const T& rad) {
    return transform<T>(quaternion<T>(0), quaternion<T>(cos(rad / 2), sin(rad / 2), 0, 0), 1); 
  }
  template <typename T>
  inline transform<T> rotateY(const T& rad) {
    return transform<T>(quaternion<T>(0), quaternion<T>(cos(rad / 2), 0, sin(rad / 2), 0), 1); 
  }
  template <typename T>
  inline transform<T> rotateZ(const T& rad) {
    return transform<T>(quaternion<T>(0), quaternion<T>(cos(rad / 2), 0, 0, sin(rad / 2)), 1); 
  }
  
  /* rotation by euler axis */
  template <typename T>
  inline transform<T> rotate(const T& rad, const T& ex, const T& ey, const T& ez) {
  // ASSERT_EQ(abs(quaternion(0, ex, ey, ez)), 1)
    return transform<T>(quaternion<T>(0), quaternion<T>(cos(rad / 2), sin(rad / 2) * ex, sin(rad / 2) * ey, sin(rad / 2) * ez), 1);
  }
 
  /* rotation by euler angle */
  template <typename T>
  inline transform<T> rotate(const T& roll, const T& pitch, const T& yaw) {
    quaternion<T> qroll(cos(roll/2), sin(roll/2), 0, 0), qpitch(cos(pitch/2), 0, sin(pitch/2), 0), qyaw(cos(yaw/2), 0, 0, sin(yaw/2));
    return transform<T>(quaternion<T>(0), qyaw * qpitch * qroll, 1);
  }

  /* specify axis by latlng coordinates with the pole (0, 0, 1) and the prime meridian (0, 0, 1) - (1, 0, 0) - (0, 0, -1) */
  template <typename T>
  inline transform<T> rotateLatLng(const T& rad, const T& latitude, const T& longitude) {
    return rotate(rad, cos(latitude) * cos(longitude), cos(latitude) * sin(longitude), sin(latitude));
  }

  /* slerp */
  template <typename T>
  class unit_slerp_interpolater {
    quaternion<T> q, d;
    public:
      unit_slerp_interpolater<T>(quaternion<T> q1, quaternion<T> q2) : q(q1), d(log_unit(q2 * (*q1))) {};
      inline quaternion<T> operator ()(const T& t) const {
        // \forall q \in H
        //   log (q^t) = t log q
        //   exp (log q) = q
        //
        //  s = slerp(q1, q2)(t)
        //  s = (q2 q1^(-1))^t q1  (|q1| = |q2| = 1)
        //  s q1* = (q2 q1*)^t
        //  log (s q1*) = t d      (d = log (q2 q1*))
        //  s q1* = exp (t d)
        //  s = exp (t d) q1
        //
        return exp_pure(t * d) * q;
      };
  };

  /* normalized lerp */
  template <typename T>
  class nlerp_interpolater {
    quaternion<T> q, d;
    public:
    nlerp_interpolater<T>(quaternion<T> q1, quaternion<T> q2) : q(q1), d(q2/q1) {};
    inline quaternion<T> operator ()(const T& t) {
      return normalize(q + d * t);
    };
  };
 
  typedef transform<float> ftransform;
  typedef quaternion<float> fquaternion;
  typedef transform<double> dtransform;
  typedef quaternion<double> dquaternion;
};
#endif
