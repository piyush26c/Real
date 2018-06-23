#ifndef BOOST_REAL_REAL_HELPERS_HPP
#define BOOST_REAL_REAL_HELPERS_HPP

#include <list>

#include <real/range.hpp>
#include <real/range_helper.hpp>

namespace boost {
    namespace real {
        namespace helper {

            int add_vectors(
                    std::vector<int> &lhs,
                    int lhs_integers,
                    std::vector<int> &rhs,
                    int rhs_integers,
                    std::vector<int> &result
            ) {
                int carry = 0;
                int digit;

                boost::real::helper::align_numbers(lhs, lhs_integers, rhs, rhs_integers);

                auto lhs_it = lhs.crbegin();
                auto rhs_it = rhs.crbegin();

                while(lhs_it != lhs.crend() and rhs_it != rhs.crend()) {

                    digit = carry + *lhs_it + *rhs_it;

                    if (digit > 9) {
                        carry = 1;
                        digit -= 10;
                    } else {
                        carry = 0;
                    }

                    result.insert(result.begin(), digit);
                    ++lhs_it;
                    ++rhs_it;
                }

                if (carry == 1) {
                    result.insert(result.begin(), 1);
                    return rhs_integers + 1;
                }

                return rhs_integers;
            }

            /*
             * Pre-condition: lhs >= rhs
             */
            int subtract_vectors(
                    std::vector<int> &lhs,
                    int lhs_integers,
                    std::vector<int> &rhs,
                    int rhs_integers,
                    std::vector<int> &result
            ) {
                int borrow = 0;

                boost::real::helper::align_numbers(lhs, lhs_integers, rhs, rhs_integers);

                auto lhs_it = lhs.rbegin();
                auto rhs_it = rhs.rbegin();

                while(lhs_it != lhs.rend() and rhs_it != rhs.rend()) {

                    if (*lhs_it < borrow) {
                        *lhs_it += (10 - borrow); // Borrow is always 0 or 1, then it is never greater than 10
                    } else {
                        *lhs_it -= borrow;
                        borrow = 0;
                    }

                    if (*lhs_it < *rhs_it) {
                        *lhs_it += 10;
                        borrow++;
                    }

                    result.insert(result.begin(), *lhs_it - *rhs_it);
                    ++lhs_it;
                    ++rhs_it;
                }


                // Remove possible 0 prefix if more significant digits were canceled.
                while (result.front() == 0 && lhs_integers > 1) {
                    result.erase(result.begin());
                    --lhs_integers;
                }

                return lhs_integers;
            }

            //TODO: make lhs and rhs const
            void add_bounds(boost::real::Bound& lhs, boost::real::Bound& rhs, boost::real::Bound& result) {
                if (lhs.positive == rhs.positive) {
                    result.integer_part = add_vectors(lhs.digits, lhs.integer_part, rhs.digits, rhs.integer_part, result.digits);
                    result.positive = lhs.positive;
                } else if (vector_is_lower(rhs.digits, lhs.digits)) {
                    result.integer_part = subtract_vectors(lhs.digits, lhs.integer_part, rhs.digits, rhs.integer_part, result.digits);
                    result.positive = lhs.positive;
                } else {
                    result.integer_part = subtract_vectors(rhs.digits, rhs.integer_part, lhs.digits, lhs.integer_part, result.digits);
                    result.positive = rhs.positive;
                }
            }

            void subtract_bounds(boost::real::Bound& lhs, boost::real::Bound& rhs, boost::real::Bound& result) {
                if (lhs.positive != rhs.positive) {
                    result.integer_part = add_vectors(lhs, lhs.integer_part, rhs, rhs.integer_part, result);
                    result.positive = lhs.positive;
                } else {
                    if (vector_is_lower(rhs, lhs)) {
                        result.integer_part = subtract_vectors(lhs, lhs.integer_part, rhs, rhs.integer_part, result);
                        result.positive = lhs.positive;
                    } else {
                        result.integer_part = subtract_vectors(rhs, rhs.integer_part, lhs, lhs.integer_part, result);
                        result.positive = !lhs.positive;
                    }
                }
            }
        }
    }
}

#endif //BOOST_REAL_REAL_HELPERS_HPP