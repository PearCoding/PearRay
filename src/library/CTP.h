#pragma once

#include "Config.h"

#include <type_traits>

// Compile-Time Programming
namespace PR
{
    namespace CTP
    {
        /*
         Unfortunately some constexpr functions are available only with c++14,
         but we set the baseline to c++11.
        */
        template<typename T>
        constexpr T t_min(T v1, T v2) { return !(v2 < v1) ? v1 : v2; }

        template<typename T>
        constexpr T t_max(T v1, T v2) { return (v1 < v2) ? v2 : v1; }

        template<typename T>
        constexpr T t_clamp(T v, T min, T max) { return t_min(max, t_max(min, v)); }

        // sqrt using binary search method
        template<typename T>
        class t_sqrt
        {
            static_assert(std::is_integral<T>::value, "T must be an integral type");

            static constexpr T _helper(T v, T low, T high)
            {
                return (low == high ?
                    low :
                    ((v / (low + high + 1) < (low + high + 1) / 4) ?
                        _helper(v, low, (low + high + 1)/2 - 1) :
                        _helper(v, (low + high + 1)/2, high))
                );
            }

        public:
            static constexpr T value(T v)
            { 
                return v > 0 ? _helper(v, 0, v / 2 + 1) : (T)0;
            }

            t_sqrt() = delete;
            t_sqrt(const t_sqrt&) = delete;
            t_sqrt(t_sqrt&&) = delete;
        };

        // Not optimized for high N, as it will destroy the stack...
        // Use trampoline code if higher support is needed
        template <int N>
        class PR_LIB Divisor
        {
            static_assert(N >= 1, "N must be at least 1");

        // First initialization
        public:
            static constexpr auto number = N;
            static constexpr auto sqrt_number = t_sqrt<int>::value(number) + 1;
            
        // Calculate length
        private:
            static constexpr int calc_length(int i) {
                return ((i == 0) ? 
                    0 :
                    calc_length(i-1) + ((number % i == 0) ?
                        ((number / i == i) ? 1 : 2) :
                        0
                        )
                    );
            }

        public:
            static constexpr auto length = calc_length(sqrt_number);

        // Fill array
        private:
            template<int M, int... Rest>
            struct Impl
            {
                typedef typename std::conditional<(number % M) == 0,
                    typename std::conditional<M == (number / M),
                        Impl<M - 1, M, Rest...>,
                        Impl<M - 1, M, Rest..., number / M>
                        >::type,
                    Impl<M - 1, Rest...>
                    >::type T;

                static constexpr auto& values = T::values;
            };

            template<int... Rest>
            struct Impl<0, Rest...>
            {
                static constexpr int values[] = {Rest...};
            };
            
        public:
            static constexpr auto&& values = Impl<sqrt_number>::values;

        // Utils
        public:
            Divisor() = delete;
            Divisor(const Divisor&) = delete;
            Divisor(Divisor&&) = delete;
        };

        template<int N>
        template<int... Rest>
        constexpr int Divisor<N>::Impl<0, Rest...>::values[];
    }
}