//  (C) Copyright Nick Thompson 2019.
//  Use, modification and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_MATH_TOOLS_CONDITION_NUMBERS_HPP
#define BOOST_MATH_TOOLS_CONDITION_NUMBERS_HPP
#include <cmath>
#include <boost/math/differentiation/finite_difference.hpp>

namespace boost::math::tools {

template<class Real, bool kahan=true>
class summation_condition_number {
public:
    summation_condition_number(Real const x = 0)
    {
        using std::abs;
        m_l1 = abs(x);
        m_sum = x;
        m_c = 0;
    }

    void operator+=(Real const & x)
    {
        using std::abs;
        // No need to Kahan the l1 calc; it's well conditioned:
        m_l1 += abs(x);
        if constexpr(kahan)
        {
            Real y = x - m_c;
            Real t = m_sum + y;
            m_c = (t-m_sum) -y;
            m_sum = t;
        }
        else
        {
            m_sum += x;
        }
    }

    inline void operator-=(Real const & x)
    {
        this->operator+=(-x);
    }

    // Is operator*= relevant? Presumably everything gets rescaled,
    // (m_sum -> k*m_sum, m_l1->k*m_l1, m_c->k*m_c),
    // but is this sensible? More important is it useful?

    [[nodiscard]] Real operator()() const
    {
        using std::abs;
        if (m_sum == Real(0) && m_l1 != Real(0))
        {
            return std::numeric_limits<Real>::infinity();
        }
        return m_l1/abs(m_sum);
    }

    [[nodiscard]] Real sum() const
    {
        // Higham, 1993, "The Accuracy of Floating Point Summation":
        // "In [17] and [18], Kahan describes a variation of compensated summation in which the final sum is also corrected
        // thus s=s+e is appended to the algorithm above)."
        return m_sum + m_c;
    }

    [[nodiscard]] Real l1_norm() const
    {
        return m_l1;
    }

private:
    Real m_l1;
    Real m_sum;
    Real m_c = 0;
};

template<class F, class Real>
auto condition_number(F const & f, Real const & x)
{
    using std::abs;
    // Looking forward to using autodifferentiation here!
    Real fp = boost::math::differentiation::finite_difference_derivative(f, x);
    Real fx = f(x);
    if (fx == 0) {
        if (x==0 || fp==0)
        {
            return std::numeric_limits<Real>::quiet_NaN();
        }
        return std::numeric_limits<Real>::infinity();
    }
    return abs(x*fp/fx);
}

template<class F, class Real>
auto mollified_condition_number(F const & f, Real const & x)
{
    using std::abs;
    Real fp = boost::math::differentiation::finite_difference_derivative(f, x);
    Real absfx = abs(f(x));

    if (absfx < 1)
    {
        absfx = 1;
    }

    Real absx = abs(x);
    if(absx < 1)
    {
        absx = 1;
    }

    return absx*abs(fp)/absfx;
}



}
#endif
