#include "CTP.h"

namespace PR
{
    namespace CTP
    {
        namespace Tests
        {
            typedef Divisor<81> TestDivisor;
            static_assert(TestDivisor::sqrt_number == 10,
                "[INTERNAL_TEST] Divisor class returns invalid Square Root Number");
            static_assert(TestDivisor::length == 5,
                "[INTERNAL_TEST] Divisor class returns invalid length");
            static_assert(TestDivisor::values[0] == 1,
                "[INTERNAL_TEST] Divisor class value expected to be 1");
            static_assert(TestDivisor::values[1] == 3,
                "[INTERNAL_TEST] Divisor class value expected to be 3");
            static_assert(TestDivisor::values[2] == 9,
                "[INTERNAL_TEST] Divisor class value expected to be 9");
            static_assert(TestDivisor::values[3] == 27,
                "[INTERNAL_TEST] Divisor class value expected to be 27");
            static_assert(TestDivisor::values[4] == 81,
                "[INTERNAL_TEST] Divisor class value expected to be 81");

            static_assert(Divisor<100000>::values[0] == 1,
                "[INTERNAL_TEST] Divisor class returns value");
            static_assert(Divisor<100000>::length == sizeof(Divisor<100000>::values)/sizeof(int),
                "[INTERNAL_TEST] Divisor class returns invalid length");

        }
    }
}