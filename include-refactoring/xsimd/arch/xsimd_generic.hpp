#ifndef XSIMD_GENERIC_HPP
#define XSIMD_GENERIC_HPP

#include "../types/xsimd_generic_arch.hpp"
#include "./xsimd_constants.hpp"

#include <limits>


namespace xsimd {
  // Forward declaration. Should we put them in a separate file?
  template<class T, class A>
  batch<T, A> abs(batch<T, A> const& self);
  template<class T, class A>
  batch<T, A> bitofsign(batch<T, A> const& self);
  template<class B, class T, class A>
  B bitwise_cast(batch<T, A> const& self);
  template<class T, class A>
  batch<T, A> nearbyint(batch<T, A> const& self);
  template<class T, class A>
  batch<T, A> select(batch_bool<T, A> const&, batch<T, A> const& , batch<T, A> const& );
  template<class T, class A>
  batch<as_float_t<T>, A> to_float(batch<T, A> const& self);
  template<class T, class A>
  batch<as_integer_t<T>, A> to_int(batch<T, A> const& self);
  template<class T, class A>
  batch<T, A> trunc(batch<T, A> const& self);

  namespace kernel {
    using namespace types;
    // bitofsign
    template<class A, class T> batch<T, A> bitofsign(batch<T, A> const& self, requires<generic>) {
      return self & constants::minuszero<batch<T, A>>();
    }

    // clip
    template<class A, class T> batch<T, A> clip(batch<T, A> const& self, batch<T, A> const& lo, batch<T, A> const& hi, requires<generic>) {
      return min(hi, max(self, lo));
    }

    // fdim
    template<class A, class T> batch<T, A> fdim(batch<T, A> const& self, batch<T, A> const& other, requires<generic>) {
      return fmax(batch<T, A>((T)0), self - other);
    }

    // fma
    template<class A, class T> batch<T, A> fma(batch<T, A> const& x, batch<T, A> const& y, batch<T, A> const& z, requires<generic>) {
      return x * y + z;
    }

    // fmod
    template<class A, class T> batch<T, A> fmod(batch<T, A> const& self, batch<T, A> const& other, requires<generic>) {
      return fnma(trunc(self / other), other, self);
    }

    // fms
    template<class A, class T> batch<T, A> fms(batch<T, A> const& x, batch<T, A> const& y, batch<T, A> const& z, requires<generic>) {
      return x * y - z;
    }

    // fnma
    template<class A, class T> batch<T, A> fnma(batch<T, A> const& x, batch<T, A> const& y, batch<T, A> const& z, requires<generic>) {
      return -x * y + z;
    }

    // fnms
    template<class A, class T> batch<T, A> fnms(batch<T, A> const& x, batch<T, A> const& y, batch<T, A> const& z, requires<generic>) {
      return -x * y - z;
    }

    // isinf
    template<class A, class T> batch_bool<T, A> isinf(batch<T, A> const& self, requires<generic>) {
      return abs(self) == batch<T, A>(std::numeric_limits<T>::infinity());
    }

    // isfinite
    template<class A, class T> batch_bool<T, A> isfinite(batch<T, A> const& self, requires<generic>) {
      return (self - self) == batch<T, A>((T)0);
    }

    // nearbyint
    template<class A, class T> batch<T, A> nearbyint(batch<T, A> const& self, requires<generic>) {
      using batch_type = batch<T, A>;
      batch_type s = bitofsign(self);
      batch_type v = self ^ s;
      batch_type t2n = constants::twotonmb<batch_type>();
      batch_type d0 = v + t2n;
      return s ^ select(v < t2n, d0 - t2n, v);
    }

    // nextafter
    namespace detail
    {
        template <class T, class A, bool is_int = std::is_integral<T>::value>
        struct nextafter_kernel
        {
            using batch_type = batch<T, A>;

            static inline batch_type next(batch_type const& b) noexcept
            {
                return b;
            }

            static inline batch_type prev(batch_type const& b) noexcept
            {
                return b;
            }
        };

        template <class T, class A>
        struct bitwise_cast_batch;

        template <class A>
        struct bitwise_cast_batch<float, A>
        {
            using type = batch<int32_t, A>;
        };

        template <class A>
        struct bitwise_cast_batch<double, A>
        {
            using type = batch<int64_t, A>;
        };

        template <class T, class A>
        struct nextafter_kernel<T, A, false>
        {
            using batch_type = batch<T, A>;
            using int_batch = typename bitwise_cast_batch<T, A>::type;
            using int_type = typename int_batch::value_type;

            static inline batch_type next(const batch_type& b) noexcept
            {
                batch_type n = bitwise_cast<batch_type>(bitwise_cast<int_batch>(b) + int_type(1));
                return select(b == constants::infinity<batch_type>(), b, n);
            }

            static inline batch_type prev(const batch_type& b) noexcept
            {
                batch_type p = bitwise_cast<batch_type>(bitwise_cast<int_batch>(b) - int_type(1));
                return select(b == constants::minusinfinity<batch_type>(), b, p);
            }
        };
    }
    template<class A, class T> batch<T, A> nextafter(batch<T, A> const& from, batch<T, A> const& to, requires<generic>) {
      using kernel = detail::nextafter_kernel<T, A>;
      return select(from == to, from,
                    select(to > from, kernel::next(from), kernel::prev(from)));
    }

    // remainder
    template<class A, class T> batch<T, A> remainder(batch<T, A> const& self, batch<T, A> const& other, requires<generic>) {
      return fnma(nearbyint(self / other), other, self);
    }

    // trunc
    template<class A, class T> batch<T, A> trunc(batch<T, A> const& self, requires<generic>) {
      return select(abs(self) < constants::maxflint<batch<T, A>>(), to_float(to_int(self)), self);
    }


  }

}

#endif
