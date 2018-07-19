#ifndef BOOST_REAL_REAL_HELPERS_HPP
#define BOOST_REAL_REAL_HELPERS_HPP

#include <list>

#include <real/interval.hpp>
#include <real/range_helper.hpp>

namespace boost {
    namespace real {
        namespace helper {

            boost::real::boundary abs(const boost::real::boundary& b) {
                boost::real::boundary result = b;
                result.positive = true;
                return result;
            }

            int add_vectors(const std::vector<int> &lhs,
                            int lhs_exponent,
                            const std::vector<int> &rhs,
                            int rhs_exponent,
                            std::vector<int> &result) {
                int carry = 0;

                int lower_digit = std::max((int)lhs.size() - lhs_exponent, (int)rhs.size() - rhs_exponent);
                int upper_digit = std::max(lhs_exponent, rhs_exponent);

                for (int i = lower_digit; i > -upper_digit; i--) {

                    int lhs_digit = 0

                    if (0 <= lhs_exponent + i && lhs_exponent + i < (int)lhs.size()) {
                        lhs_digit = lhs[lhs_exponent + i];
                    }

                    int rhs_digit = 0;
                    if (0 <= rhs_exponent + i && rhs_exponent + i < (int)rhs.size()) {
                        rhs_digit = rhs[rhs_exponent + i];
                    }

                    int digit = carry + lhs_digit + rhs_digit;

                    if (digit > 9) {
                        carry = 1;
                        digit -= 10;
                    } else {
                        carry = 0;
                    }

                    result.insert(result.begin(), digit);
                }

                if (carry == 1) {
                    result.insert(result.begin(), 1);
                    upper_digit++;
                }

                while (result.front() == 0) {
                    result.erase(result.begin());
                    upper_digit--;
                }

                while (result.back() == 0) {
                    result.pop_back();
                }

                return upper_digit;
            }

            /*
             * Pre-condition: lhs >= rhs
             */
            int subtract_vectors(const std::vector<int> &lhs,
                                 int lhs_integers,
                                 const std::vector<int> &rhs,
                                 int rhs_integers,
                                 std::vector<int> &result) {
                int borrow = 0;

                std::vector<int> aligned_lhs = lhs;
                std::vector<int> aligned_rhs = rhs;

                boost::real::helper::align_numbers(aligned_lhs, lhs_integers, aligned_rhs, rhs_integers);

                auto lhs_it = aligned_lhs.rbegin();
                auto rhs_it = aligned_rhs.rbegin();

                while(lhs_it != aligned_lhs.rend() and rhs_it != aligned_rhs.rend()) {

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

            void add_boundaries(const boost::real::boundary &lhs,
                                const boost::real::boundary &rhs,
                                boost::real::boundary &result) {
                if (lhs.positive == rhs.positive) {
                    result.exponent = add_vectors(lhs.digits,
                                                  lhs.exponent,
                                                  rhs.digits,
                                                  rhs.exponent,
                                                  result.digits);
                    result.positive = lhs.positive;
                } else if (vector_is_lower(rhs.digits, lhs.digits)) {
                    result.exponent = subtract_vectors(lhs.digits,
                                                       lhs.exponent,
                                                       rhs.digits,
                                                       rhs.exponent,
                                                       result.digits);
                    result.positive = lhs.positive;
                } else {
                    result.exponent = subtract_vectors(rhs.digits,
                                                       rhs.exponent,
                                                       lhs.digits,
                                                       lhs.exponent,
                                                       result.digits);
                    result.positive = rhs.positive;
                }
            }

            void subtract_boundaries(const boost::real::boundary &lhs,
                                     const boost::real::boundary &rhs,
                                     boost::real::boundary &result) {
                if (lhs.positive != rhs.positive) {
                    result.exponent = add_vectors(lhs.digits,
                                                      lhs.exponent,
                                                      rhs.digits,
                                                      rhs.exponent,
                                                      result.digits);
                    result.positive = lhs.positive;
                } else {

                    if (abs(rhs) < abs(lhs)) {
                        result.exponent = subtract_vectors(lhs.digits,
                                                               lhs.exponent,
                                                               rhs.digits,
                                                               rhs.exponent,
                                                               result.digits);
                        result.positive = lhs.positive;
                    } else {
                        result.exponent = subtract_vectors(rhs.digits,
                                                               rhs.exponent,
                                                               lhs.digits,
                                                               lhs.exponent,
                                                               result.digits);
                        result.positive = !lhs.positive;
                    }
                }
            }

            // Multiplies str1 and str2, and prints result.
            int multiply_vectors(
                    const std::vector<int>& lhs,
                    int lhs_exponent,
                    const std::vector<int>& rhs,
                    int rhs_exponent,
                    std::vector<int>& result
            ) {

                // will keep the result number in vector
                // in reverse order
                size_t new_size = lhs.size() + rhs.size();

                if (!result.empty()) result.clear();
                for (int i = 0; i < (int)new_size; i++) result.push_back(0);
                // TODO: Check why the assign method crashes.
                //result.assign(new_size, 0);

                // Below two indexes are used to find positions
                // in result.
                auto i_n1 = (int) result.size() - 1;

                // Go from right to left in lhs
                for (int i = (int)lhs.size()-1; i>=0; i--) {
                    int carry = 0;

                    // To shift position to left after every
                    // multiplication of a digit in rhs
                    int i_n2 = 0;

                    // Go from right to left in rhs
                    for (int j = (int)rhs.size()-1; j>=0; j--) {

                        // Multiply current digit of second number with current digit of first number
                        // and add result to previously stored result at current position.
                        int sum = lhs[i]*rhs[j] + result[i_n1 - i_n2] + carry;

                        // Carry for next iteration
                        carry = sum / 10;

                        // Store result
                        result[i_n1 - i_n2] = sum % 10;

                        i_n2++;
                    }

                    // store carry in next cell
                    if (carry > 0) {
                        result[i_n1 - i_n2] += carry;
                    }

                    // To shift position to left after every
                    // multiplication of a digit in lhs.
                    i_n1--;
                }

                int fractional_part = ((int)lhs.size() - lhs_exponent) + ((int)rhs.size() - rhs_exponent);

                // ignore 0s from the most right of the integer part
                auto it = result.begin();
                while ((int)result.size() > fractional_part + 1 && *it == 0) {
                    result.erase(it);
                }

                if (result.empty()) {
                    result.assign(1, 0);
                    return 1;
                }

                return (int)result.size() - fractional_part;
            }


            void multiply_boundaries(const boost::real::boundary &lhs,
                                     const boost::real::boundary &rhs,
                                     boost::real::boundary &result) {

                result.positive = lhs.positive == rhs.positive;
                result.exponent = multiply_vectors(lhs.digits,
                                                       lhs.exponent,
                                                       rhs.digits,
                                                       rhs.exponent,
                                                       result.digits);
            }

            int divide_vectors(
                    const std::vector<int>& dividend,
                    int dividend_exponent,
                    const std::vector<int>& divisor,
                    int divisor_exponent,
                    std::vector<int>& cotient
            ) {

                // NOTE: All the alignment must be replaced by a logical alignment
                // Align decimal parts
                int dividend_decimal_part = (int)dividend.size() - dividend_exponent;
                int divisor_decimal_part = (int)divisor.size() - divisor_exponent;

                std::vector<int> aligned_dividend = dividend;
                std::vector<int> aligned_divisor = divisor;


                while (dividend_decimal_part < divisor_decimal_part) {
                    aligned_dividend.push_back(0);
                    dividend_decimal_part++;
                }

                while (dividend_decimal_part > divisor_decimal_part) {
                    aligned_divisor.push_back(0);
                    divisor_decimal_part++;
                }

                // NOTE: when logical alignment will be implemented numbers have no extra zeros and this is not necessary
                while(aligned_dividend.front() == 0) {
                    aligned_dividend.erase(aligned_dividend.begin());
                }

                while(aligned_divisor.front() == 0) {
                    aligned_divisor.erase(aligned_divisor.begin());
                }

                std::vector<int> current_dividend(
                        aligned_dividend.begin(),
                        aligned_dividend.begin() + aligned_divisor.size()
                );
                auto next_digit = aligned_dividend.begin() + aligned_divisor.size();
                std::vector<int> residual = aligned_dividend;

                // TODO: This loop end criteria generate a whole division, a precision stop criteria
                // TODO: must be implemented for numbers like 1/3 that are periodic numbers to allow
                // TODO: calculate floating point result with some desired precision
                while ((residual.size() > 1 || residual.front() != 0) && next_digit != aligned_dividend.end()) {

                    // Obtain the smaller part of the dividend that is greater than the divisor
                    while (aligned_divisor.size() > current_dividend.size()) {
                        current_dividend.push_back(*next_digit);
                        ++next_digit;
                    }

                    if (aligned_divisor.size() == current_dividend.size() && boost::real::helper::vector_is_lower(current_dividend, aligned_divisor)) {
                        current_dividend.push_back(*next_digit);
                        ++next_digit;
                    }

                    // Obtaining the greater digit by which the divisor can be multiplied and still be lower than the dividend
                    // TODO: when using a higher base, this search could be done using binary search to improve performance
                    std::vector<int> closest;
                    int digit = 0;
                    do {
                        digit++;
                        std::vector<int> multiplier = {digit};
                        multiply_vectors(aligned_divisor, (int)aligned_divisor.size(), multiplier, 1, closest);

                    } while(
                            closest.size() < current_dividend.size() ||
                            (closest.size() == current_dividend.size() && !boost::real::helper::vector_is_lower(current_dividend, closest)) // closes <= current_dividend
                    );

                    // i should be in [1, 10] and i - 1 in [0, 9]
                    // The found digit is the next digit in the cotient result
                    cotient.push_back(digit-1);

                    // Update the residual for the next iteration where more digits of the dividend will be considered
                    std::vector<int> multiplier = {digit-1};
                    multiply_vectors(aligned_divisor, (int)aligned_divisor.size(), multiplier, 1, closest);
                    residual.clear();
                    subtract_vectors(current_dividend, (int)current_dividend.size(), closest, (int)closest.size(), residual);
                    current_dividend = residual;
                }

                // TODO: once the stop criteria is improved, the integer part is not the whole number
                return (int)cotient.size();
            }
        }
    }
}

#endif //BOOST_REAL_REAL_HELPERS_HPP