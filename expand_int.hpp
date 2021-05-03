#pragma once
#include <stdint.h>
#include <malloc.h>
#include <utility>
#include <stdexcept>

#define IS_BIG_ENDIAN (!(union { uint16_t u16; unsigned char c; }){ .u16 = 1 }.c)


namespace {
    template <typename T> std::string n2hexstr(T w, size_t hex_len = sizeof(T) << 1) {
        static const char* digits = "0123456789ABCDEF";
        std::string rc(hex_len, '0');
        for (size_t i = 0, j = (hex_len - 1) * 4; i < hex_len; ++i, j -= 4)
            rc[i] = digits[(w >> j) & 0x0f];
        return rc;
    }
}


template<uint64_t byte_expand>
class expand_uint;








template<>
class expand_uint<0> {
#ifdef IS_BIG_ENDIAN
    uint64_t UPPER;
    uint64_t LOWER;
#else
    uint64_t LOWER;
    uint64_t UPPER;
#endif
public:
    const uint64_t& upper() const {
        return UPPER;
    }

    const uint64_t& lower() const {
        return LOWER;
    }
    static constexpr uint64_t expand_bits() {
        return 128;
    }
    static constexpr uint64_t expand_bits_childs() {
        return 64;
    }
    static constexpr uint64_t expand_bits_sub_childs() {
        return 32;
    }

    expand_uint() : UPPER(0), LOWER(0) {};
    expand_uint(const expand_uint& rhs) = default;
    expand_uint(expand_uint&& rhs) = default;

    template <typename T>
    expand_uint(const T& rhs) : UPPER(0), LOWER(rhs)
    {
        if (std::is_signed<T>::value) {
            if (rhs < 0) {
                UPPER = -1;
            }
        }
    }

    template <typename S, typename T>
    expand_uint(const S& upper_rhs, const T& lower_rhs) : UPPER(upper_rhs), LOWER(lower_rhs)
    {}

    expand_uint(const char* str) : UPPER(0), LOWER(0) {
        expand_uint mult(10);
        size_t str_len = strlen(str);
        for (size_t i = 0; i < str_len; i++) {
            *this *= mult;
            *this += str[i] - '0';
        }
    }
    std::string to_ansi_string() {
        std::string res;
        {
            expand_uint tmp = *this;
            std::pair<expand_uint, expand_uint> temp;
            expand_uint dever(10);
            while (true) {
                res += (uint8_t)(tmp % dever) + '0';
                tmp /= dever;
                if (tmp == exint_0)
                    break;
            }
        }
        std::reverse(res.begin(), res.end());
        return res;
    }
    std::string to_ansi_string(uint8_t base) {
        if ((base < 2) || (base > 36)) {
            throw std::invalid_argument("Base must be in the range 2-36");
        }
        std::string out = "";
        if (!(*this)) {
            out = "0";
        }
        else {
            std::pair <expand_uint, expand_uint> qr(*this, exint_0);
            do {
                qr = divmod(qr.first, base);
                out = "0123456789abcdefghijklmnopqrstuvwxyz"[(uint8_t)qr.second] + out;
            } while (qr.first);
        }
        return out;
    }

    expand_uint& operator=(const expand_uint& rhs) = default;
    expand_uint& operator=(expand_uint&& rhs) = default;

    std::pair <expand_uint, expand_uint> divmod(const expand_uint& lhs, const expand_uint& rhs) const {
        // Save some calculations /////////////////////
        if (rhs == exint_0) {
            throw std::domain_error("Error: division or modulus by 0");
        }
        else if (rhs == exint_1) {
            return std::pair <expand_uint, expand_uint>(lhs, exint_0);
        }
        else if (lhs == rhs) {
            return std::pair <expand_uint, expand_uint>(exint_1, exint_0);
        }
        else if ((lhs == exint_0) || (lhs < rhs)) {
            return std::pair <expand_uint, expand_uint>(exint_0, lhs);
        }

        std::pair <expand_uint, expand_uint> qr(exint_0, exint_0);
        for (uint64_t x = lhs.bits(); x > 0; x--) {
            qr.first <<= 1;
            qr.second <<= 1;

            if ((lhs >> (x - 1U)) & exint_1) {
                ++qr.second;
            }

            if (qr.second >= rhs) {
                qr.second -= rhs;
                ++qr.first;
            }
        }
        return qr;
    }
   
    expand_uint& operator++() {
        return *this += exint_1;
    }
    expand_uint operator++(int) {
        expand_uint temp(*this);
        ++* this;
        return temp;
    }
    expand_uint operator/(const expand_uint& rhs) const {
        return divmod(*this, rhs).first;
    }

    expand_uint& operator/=(const expand_uint& rhs) {
        *this = *this / rhs;
        return *this;
    }

    expand_uint operator%(const expand_uint& rhs) const {
        return divmod(*this, rhs).second;
    }

    expand_uint& operator%=(const expand_uint& rhs) {
        *this = *this % rhs;
        return *this;
    }










    static const expand_uint exint_0;
    static const expand_uint exint_1;

    expand_uint operator&(const expand_uint& rhs) const {
        return expand_uint(UPPER & rhs.UPPER, LOWER & rhs.LOWER);
    }

    expand_uint& operator&=(const expand_uint& rhs) {
        UPPER &= rhs.UPPER;
        LOWER &= rhs.LOWER;
        return *this;
    }

    expand_uint operator|(const expand_uint& rhs) const {
        return expand_uint(UPPER | rhs.UPPER, LOWER | rhs.LOWER);
    }

    expand_uint& operator|=(const expand_uint& rhs) {
        UPPER |= rhs.UPPER;
        LOWER |= rhs.LOWER;
        return *this;
    }

    expand_uint operator^(const expand_uint& rhs) const {
        return expand_uint(UPPER ^ rhs.UPPER, LOWER ^ rhs.LOWER);
    }

    expand_uint& operator^=(const expand_uint& rhs) {
        UPPER ^= rhs.UPPER;
        LOWER ^= rhs.LOWER;
        return *this;
    }

    expand_uint operator~() const {
        return expand_uint(~UPPER, ~LOWER);
    }
    uint64_t bits() const {
        uint64_t out = 0;
        if (UPPER) {
            out = 64;
            uint64_t up = UPPER;
            while (up) {
                up >>= 1;
                out++;
            }
        }
        else {
            uint64_t low = LOWER;
            while (low) {
                low >>= 1;
                out++;
            }
        }
        return out;
    }
    expand_uint& operator<<=(uint64_t shift) {
        if (shift >= expand_bits())
            *this = exint_0;

        else if (shift == expand_bits_childs()) {
            UPPER = LOWER;
            LOWER = 0;
        }
        else if (shift == 0);
        else if (shift < expand_bits_childs()) {
            uint64_t tmp = LOWER;
            UPPER = (UPPER <<= shift) + (LOWER >>= (expand_bits_childs() - shift));
            LOWER = tmp;
            LOWER <<= shift;
        }
        else if ((expand_bits() > shift) && (shift > expand_bits_childs())) {
            UPPER = LOWER <<= (shift - expand_bits_childs());
            LOWER = 0;
        }
        else
            *this = exint_0;

        return *this;
    }
    expand_uint operator<<(uint64_t shift) const {
        return expand_uint(*this) <<= shift;
    }

    expand_uint& operator>>=(uint64_t shift) {
        if (shift >= expand_bits())
            *this = exint_0;
        else if (shift == expand_bits_childs()) {
            LOWER = UPPER;
            UPPER = 0;
        }
        else if (shift == 0);
        else if (shift < expand_bits_childs()) {
            uint64_t tmp = UPPER;
            LOWER = (UPPER <<= (expand_bits_childs() - shift)) + (LOWER >>= shift);
            UPPER = tmp;
            UPPER >>= shift;
        }
        else if ((expand_bits() > shift) && (shift > expand_bits_childs())) {
            UPPER >>= (shift - expand_bits_childs());
            LOWER = UPPER;
            UPPER = 0;
        }
        else
            *this = exint_0;

        return *this;
    }
    expand_uint operator>>(uint64_t shift) const {
        return expand_uint(*this) >>= shift;
    }
    bool operator!() const {
        return !(bool)(UPPER | LOWER);
    }

    bool operator&&(const expand_uint& rhs) const {
        return ((bool)*this && rhs);
    }

    bool operator||(const expand_uint& rhs) const {
        return ((bool)*this || rhs);
    }

    bool operator==(const expand_uint& rhs) const {
        return ((UPPER == rhs.UPPER) && (LOWER == rhs.LOWER));
    }

    bool operator!=(const expand_uint& rhs) const {
        return ((UPPER != rhs.UPPER) | (LOWER != rhs.LOWER));
    }

    bool operator>(const expand_uint& rhs) const {
        if (UPPER == rhs.UPPER) {
            return (LOWER > rhs.LOWER);
        }
        return (UPPER > rhs.UPPER);
    }

    bool operator<(const expand_uint& rhs) const {
        if (UPPER == rhs.UPPER) {
            return (LOWER < rhs.LOWER);
        }
        return (UPPER < rhs.UPPER);
    }

    bool operator>=(const expand_uint& rhs) const {
        return ((*this > rhs) | (*this == rhs));
    }

    bool operator<=(const expand_uint& rhs) const {
        return ((*this < rhs) | (*this == rhs));
    }

    expand_uint operator+(const expand_uint& rhs) const {
        return expand_uint(UPPER + rhs.UPPER + ((LOWER + rhs.LOWER) < LOWER), LOWER + rhs.LOWER);
    }

    expand_uint& operator+=(const expand_uint& rhs) {
        UPPER += rhs.UPPER + ((LOWER + rhs.LOWER) < LOWER);
        LOWER += rhs.LOWER;
        return *this;
    }

    expand_uint operator-(const expand_uint& rhs) const {
        return expand_uint(UPPER - rhs.UPPER - ((LOWER - rhs.LOWER) > LOWER), LOWER - rhs.LOWER);
    }

    expand_uint& operator-=(const expand_uint& rhs) {
        UPPER -= rhs.UPPER + ((LOWER - rhs.LOWER) > LOWER);
        LOWER -= rhs.LOWER;
        return *this;
    }

    expand_uint operator*(const expand_uint& rhs) const {
        // split values into 4 32-bit parts
        uint64_t top[4] = { UPPER >> 32, UPPER & 0xffffffff, LOWER >> 32, LOWER & 0xffffffff };
        uint64_t bottom[4] = { rhs.UPPER >> 32, rhs.UPPER & 0xffffffff, rhs.LOWER >> 32, rhs.LOWER & 0xffffffff };
        uint64_t products[4][4];

        // multiply each component of the values
        for (int y = 3; y > -1; y--) {
            for (int x = 3; x > -1; x--) {
                products[3 - x][y] = top[x] * bottom[y];
            }
        }

        // first row
        uint64_t fourth32 = (products[0][3] & 0xffffffff);
        uint64_t third32 = (products[0][2] & 0xffffffff) + (products[0][3] >> 32);
        uint64_t second32 = (products[0][1] & 0xffffffff) + (products[0][2] >> 32);
        uint64_t first32 = (products[0][0] & 0xffffffff) + (products[0][1] >> 32);

        // second row
        third32 += (products[1][3] & 0xffffffff);
        second32 += (products[1][2] & 0xffffffff) + (products[1][3] >> 32);
        first32 += (products[1][1] & 0xffffffff) + (products[1][2] >> 32);

        // third row
        second32 += (products[2][3] & 0xffffffff);
        first32 += (products[2][2] & 0xffffffff) + (products[2][3] >> 32);

        // fourth row
        first32 += (products[3][3] & 0xffffffff);

        // move carry to next digit
        third32 += fourth32 >> 32;
        second32 += third32 >> 32;
        first32 += second32 >> 32;

        // remove carry from current digit
        fourth32 &= 0xffffffff;
        third32 &= 0xffffffff;
        second32 &= 0xffffffff;
        first32 &= 0xffffffff;

        // combine components
        return expand_uint((first32 << 32) | second32, (third32 << 32) | fourth32);
    }

    expand_uint& operator*=(const expand_uint& rhs) {
        *this = *this * rhs;
        return *this;
    }
    explicit operator bool() const {
        return (bool)(UPPER | LOWER);
    }
    explicit operator uint8_t() const {
        return (uint8_t)LOWER;
    }
    explicit operator uint16_t() const {
        return (uint16_t)LOWER;
    }

    explicit operator uint32_t() const {
        return (uint32_t)LOWER;
    }

    explicit operator uint64_t() const {
        return (uint64_t)LOWER;
    }
    void set_to_max() {
        UPPER = 0xffffffffffffffff;
        LOWER = 0xffffffffffffffff;
    }
};
const expand_uint<0> expand_uint<0>::exint_0 = 0;
const expand_uint<0> expand_uint<0>::exint_1 = 1;

template<uint64_t byte_expand>
class expand_uint {
#ifdef IS_BIG_ENDIAN
    expand_uint<byte_expand - 1> UPPER;
    expand_uint<byte_expand - 1> LOWER;
#else
    expand_uint<byte_expand - 1> LOWER;
    expand_uint<byte_expand - 1> UPPER;
#endif
    uint64_t bits() const {
        uint64_t out = 0;
        if (UPPER) {
            out = expand_bits_childs();
            expand_uint<byte_expand - 1> up = UPPER;
            while (up) {
                up >>= 1;
                out++;
            }
        }
        else {
            expand_uint<byte_expand - 1> low = LOWER;
            while (low) {
                low >>= 1;
                out++;
            }
        }
        return out;
    }
    std::pair <expand_uint, expand_uint> divmod(const expand_uint& lhs, const expand_uint& rhs) const {
        // Save some calculations /////////////////////
        if (rhs == exint_0) {
            throw std::domain_error("Error: division or modulus by 0");
        }
        else if (rhs == exint_1) {
            return std::pair <expand_uint, expand_uint>(lhs, exint_0);
        }
        else if (lhs == rhs) {
            return std::pair <expand_uint, expand_uint>(exint_1, exint_0);
        }
        else if ((lhs == exint_0) || (lhs < rhs)) {
            return std::pair <expand_uint, expand_uint>(exint_0, lhs);
        }

        std::pair <expand_uint, expand_uint> qr(exint_0, lhs);
        expand_uint copyd = rhs << (lhs.bits() - rhs.bits());
        expand_uint adder = exint_1 << (lhs.bits() - rhs.bits());
        if (copyd > qr.second) {
            copyd >>= 1;
            adder >>= 1;
        }
        while (qr.second >= rhs) {
            if (qr.second >= copyd) {
                qr.second -= copyd;
                qr.first |= adder;
            }
            copyd >>= 1;
            adder >>= 1;
        }
        return qr;
    }
    template <typename I> std::string n2hexstr(I w, size_t hex_len = sizeof(I) << 1) {
        static const char* digits = "0123456789ABCDEF";
        std::string rc(hex_len, '0');
        for (size_t i = 0, j = (hex_len - 1) * 4; i < hex_len; ++i, j -= 4)
            rc[i] = digits[(w >> j) & 0x0f];
        return rc;
    }
public:
    static const expand_uint<byte_expand> exint_0;
    static const expand_uint<byte_expand> exint_1;
    const expand_uint<byte_expand - 1>& upper() const {
        return UPPER;
    }

    const expand_uint<byte_expand - 1>& lower() const {
        return LOWER;
    }

    constexpr static uint64_t expand_bits() {
        return expand_uint<byte_expand - 1>::expand_bits() * 2;
    }
    constexpr static uint64_t expand_bits_childs() {
        return expand_uint<byte_expand - 1>::expand_bits();
    }
    constexpr static uint64_t expand_bits_sub_childs() {
        return expand_uint<byte_expand - 1>::expand_bits_childs();
    }
    expand_uint() : UPPER(0), LOWER(0) {};
    expand_uint(const expand_uint& rhs) = default;
    expand_uint(expand_uint&& rhs) = default;
    expand_uint(const char* str) : UPPER(0), LOWER(0) {
        expand_uint mult(10);
        size_t str_len = strlen(str);
        for (size_t i = 0; i < str_len; i++) {
            *this *= mult;
            *this += str[i] - '0';
        }
    }
    template <typename T>
    expand_uint(const T& rhs) : UPPER(0), LOWER(rhs)
    {
        if (std::is_signed<T>::value) {
            if (rhs < 0) {
                UPPER = -1;
            }
        }
    }

    template <typename S, typename T>
    expand_uint(const S& upper_rhs, const T& lower_rhs) : LOWER(lower_rhs), UPPER(upper_rhs)
    {}

    std::string to_ansi_string() {
        std::string res;
        {
            std::pair<expand_uint, expand_uint> temp(*this, exint_0);
            expand_uint dever(10);
            while (true) {
                temp = divmod(temp.first, dever);
                res += (uint8_t)(temp.second) + '0';
                if (temp.first == exint_0)
                    break;
            }
        }
        std::reverse(res.begin(), res.end());
        return res;
    }
    std::string to_hex_str() {
        uint64_t* proxy = ((uint64_t*)this);
        std::stringstream stream;
        for (int64_t i = expand_bits() / 64 - 1; i >= 0; --i)
            stream << n2hexstr(proxy[i]);
        return "0x" + stream.str();
    }



    expand_uint& operator=(const expand_uint& rhs) = default;
    expand_uint& operator=(expand_uint&& rhs) = default;

    expand_uint& operator++() {
        return *this += exint_1;
    }
    expand_uint operator++(int) {
        expand_uint temp(*this);
        *this += exint_1;
        return temp;
    }

    expand_uint operator/(const expand_uint& rhs) const {
        return divmod(*this, rhs).first;
    }

    expand_uint operator%(const expand_uint& rhs) const {
        return divmod(*this, rhs).second;
    }

    expand_uint& operator/=(const expand_uint& rhs) {
        *this = divmod(*this, rhs).first;
        return *this;
    }

    expand_uint& operator%=(const expand_uint& rhs) {
        *this = divmod(*this, rhs).second;
        return *this;
    }










    expand_uint operator&(const expand_uint& rhs) const {
        return expand_uint(UPPER & rhs.UPPER, LOWER & rhs.LOWER);
    }

    expand_uint& operator&=(const expand_uint& rhs) {
        UPPER &= rhs.UPPER;
        LOWER &= rhs.LOWER;
        return *this;
    }

    expand_uint operator|(const expand_uint& rhs) const {
        return expand_uint(UPPER | rhs.UPPER, LOWER | rhs.LOWER);
    }

    expand_uint& operator|=(const expand_uint& rhs) {
        UPPER |= rhs.UPPER;
        LOWER |= rhs.LOWER;
        return *this;
    }

    expand_uint operator^(const expand_uint& rhs) const {
        return expand_uint(UPPER ^ rhs.UPPER, LOWER ^ rhs.LOWER);
    }

    expand_uint& operator^=(const expand_uint& rhs) {
        UPPER ^= rhs.UPPER;
        LOWER ^= rhs.LOWER;
        return *this;
    }

    expand_uint operator~() const {
        return expand_uint(~UPPER, ~LOWER);
    }



    expand_uint& operator<<=(uint64_t shift) {
        if (shift >= expand_bits())
            *this = exint_0;

        else if (shift == expand_bits_childs()) {
            UPPER = LOWER;
            LOWER = expand_uint<byte_expand - 1>::exint_0;
        }
        else if (shift == 0);
        else if (shift < expand_bits_childs()) {
            expand_uint<byte_expand - 1> tmp = LOWER;
            UPPER <<= shift;
            UPPER += (tmp >>= (expand_bits_childs() - shift));
            LOWER <<= shift;
        }
        else if ((expand_bits() > shift) && (shift > expand_bits_childs())) {
            UPPER = LOWER <<= (shift - expand_bits_childs());
            LOWER = expand_uint<byte_expand - 1>::exint_0;
        }
        else
            *this = exint_0;

        return *this;
    }
    expand_uint operator<<(uint64_t shift) const {
        return expand_uint(*this) <<= shift;
    }


    expand_uint& operator>>=(uint64_t shift) {
        if (shift >= expand_bits())
            *this = exint_0;
        else if (shift == expand_bits_childs()) {
            LOWER = UPPER;
            UPPER = expand_uint<byte_expand - 1>::exint_0;
        }
        else if (shift == 0);
        else if (shift < expand_bits_childs()) {
            expand_uint<byte_expand - 1> tmp = UPPER;
            LOWER >>= shift;
            LOWER += (tmp <<= (expand_bits_childs() - shift));
            UPPER >>= shift;
        }
        else if ((expand_bits() > shift) && (shift > expand_bits_childs())) {
            UPPER >>= (shift - expand_bits_childs());
            LOWER = UPPER;
            UPPER = expand_uint<byte_expand - 1>::exint_0;
        }
        else
            *this = exint_0;

        return *this;
    }
    expand_uint operator>>(uint64_t shift) const {
        return expand_uint(*this) >>= shift;
    }
    bool operator!() const {
        return !(bool)(UPPER | LOWER);
    }

    bool operator&&(const expand_uint& rhs) const {
        return ((bool)*this && rhs);
    }

    bool operator||(const expand_uint& rhs) const {
        return ((bool)*this || rhs);
    }

    bool operator==(const expand_uint& rhs) const {
        return ((UPPER == rhs.UPPER) && (LOWER == rhs.LOWER));
    }

    bool operator!=(const expand_uint& rhs) const {
        return ((UPPER != rhs.UPPER) | (LOWER != rhs.LOWER));
    }

    bool operator>(const expand_uint& rhs) const {
        if (UPPER == rhs.UPPER) {
            return (LOWER > rhs.LOWER);
        }
        return (UPPER > rhs.UPPER);
    }

    bool operator<(const expand_uint& rhs) const {
        if (UPPER == rhs.UPPER) {
            return (LOWER < rhs.LOWER);
        }
        return (UPPER < rhs.UPPER);
    }

    bool operator>=(const expand_uint& rhs) const {
        return ((*this > rhs) | (*this == rhs));
    }

    bool operator<=(const expand_uint& rhs) const {
        return ((*this < rhs) | (*this == rhs));
    }

    expand_uint operator+(const expand_uint& rhs) const {
        return expand_uint(*this) += rhs;
    }
    expand_uint operator-(const expand_uint& rhs) const {
        return expand_uint(*this) -= rhs;
    }

    expand_uint& operator+=(const expand_uint& rhs) {
        UPPER += rhs.UPPER;
        UPPER += ((LOWER + rhs.LOWER) < LOWER) ? exint_1 : exint_0;
        LOWER += rhs.LOWER;
        return *this;
    }
    expand_uint& operator-=(const expand_uint& rhs) {
        UPPER -= rhs.UPPER;
        UPPER -= ((LOWER - rhs.LOWER) > LOWER) ? exint_1 : exint_0;
        LOWER -= rhs.LOWER;
        return *this;
    }

    expand_uint operator*(const expand_uint& rhs) const {
        // split values into 4 64-bit parts
        expand_uint<byte_expand - 1> products[4][4];
        expand_uint<byte_expand - 1> top[4] = { UPPER.upper(), UPPER.lower(), LOWER.upper(), LOWER.lower() };
        expand_uint<byte_expand - 1> bottom[4] = { rhs.upper().upper(), rhs.upper().lower(), rhs.lower().upper(), rhs.lower().lower() };


        // multiply each component of the values
        for (int y = 3; y > -1; y--) {
            for (int x = 3; x > -1; x--) {
                products[3 - y][x] = top[x] * bottom[y];
            }
        }    // first row
        expand_uint<byte_expand - 1> fourth64 = expand_uint<byte_expand - 1>(products[0][3].lower());
        expand_uint<byte_expand - 1> third64 = expand_uint<byte_expand - 1>(products[0][2].lower()) + expand_uint<byte_expand - 1>(products[0][3].upper());
        expand_uint<byte_expand - 1> second64 = expand_uint<byte_expand - 1>(products[0][1].lower()) + expand_uint<byte_expand - 1>(products[0][2].upper());
        expand_uint<byte_expand - 1> first64 = expand_uint<byte_expand - 1>(products[0][0].lower()) + expand_uint<byte_expand - 1>(products[0][1].upper());

        // second row
        third64 += expand_uint<byte_expand - 1>(products[1][3].lower());
        second64 += expand_uint<byte_expand - 1>(products[1][2].lower()) + expand_uint<byte_expand - 1>(products[1][3].upper());
        first64 += expand_uint<byte_expand - 1>(products[1][1].lower()) + expand_uint<byte_expand - 1>(products[1][2].upper());

        // third row
        second64 += expand_uint<byte_expand - 1>(products[2][3].lower());
        first64 += expand_uint<byte_expand - 1>(products[2][2].lower()) + expand_uint<byte_expand - 1>(products[2][3].upper());

        // fourth row
        first64 += expand_uint<byte_expand - 1>(products[3][3].lower());





        // combines the values, taking care of carry over
        return expand_uint(first64 << expand_bits_sub_childs(), exint_0) +
            expand_uint(third64.upper(), third64 << expand_bits_sub_childs()) +
            expand_uint(second64, exint_0) +
            expand_uint(fourth64);
    }


    expand_uint& operator*=(const expand_uint& rhs) {
        *this = *this * rhs;
        return *this;
    }
    explicit operator bool() const {
        return (bool)(UPPER | LOWER);
    }
    explicit operator uint8_t() const {
        return (uint8_t)LOWER;
    }
    explicit operator uint16_t() const {
        return (uint16_t)LOWER;
    }

    explicit operator uint32_t() const {
        return (uint32_t)LOWER;
    }

    explicit operator uint64_t() const {
        return (uint64_t)LOWER;
    }
    void set_to_max() {
        UPPER.set_to_max();
        LOWER.set_to_max();
    }
};
template<uint64_t byte_expand>
const expand_uint<byte_expand> expand_uint<byte_expand>::exint_0 = 0;
template<uint64_t byte_expand>
const expand_uint<byte_expand> expand_uint<byte_expand>::exint_1 = 1;

typedef expand_uint<0> uint128_ext;
typedef expand_uint<1> uint256_ext;
typedef expand_uint<2> uint512_ext;
typedef expand_uint<3> uint1024_ext;
typedef expand_uint<4> uint2048_ext;
typedef expand_uint<5> uint4096_ext;
typedef expand_uint<6> uint8192_ext;
typedef expand_uint<7> uint16384_ext;
typedef expand_uint<8> uint32768_ext;

typedef expand_uint<0> uint16_exb;
typedef expand_uint<1> uint32_exb;
typedef expand_uint<2> uint64_exb;
typedef expand_uint<3> uint128_exb;
typedef expand_uint<4> uint256_exb;
typedef expand_uint<5> uint512_exb;
typedef expand_uint<6> uint1024_exb;
typedef expand_uint<7> uint2048_exb;
typedef expand_uint<8> uint4096_exb;





template<uint64_t byte_expand>
class expand_int {

    expand_int& switch_my_siqn() {
        val.unsigned_int = ~val.unsigned_int;
        val.unsigned_int += expand_uint<byte_expand>::exint_1;
        return *this;
    }
    expand_int switch_my_siqn() const {
        return expand_int(*this).switch_my_siqn();
    }

    expand_int& switch_to_unsiqn() {
        if (val.is_minus) 
            switch_my_siqn();
        return *this;
    }
    expand_int switch_to_unsiqn() const {
        if (val.is_minus)
            return expand_int(*this).switch_my_siqn();
        else return *this;
    }

    expand_int& switch_to_siqn() {
        if (!val.is_minus)
            switch_my_siqn();
        return *this;
    }
    expand_int switch_to_siqn() const {
        if (!val.is_minus)
            return expand_int(*this).switch_my_siqn();
        else return *this;
    }

    union to_constructor{
#ifdef IS_BIG_ENDIAN
        bool is_minus : 1;
#else
        struct s {
            bool unused : sizeof(expand_uint<byte_expand>) * 8 - 1;
            bool _is_minus : 1;

            s() {

            }
            s(bool val) {
                _is_minus = val;
            }
            operator bool() {
                return _is_minus;
            }
        } is_minus;
#endif


        expand_uint<byte_expand> unsigned_int;
        to_constructor() {}
    } val;
    static expand_int get_min() {
        expand_int tmp = 0;
        tmp.val.is_minus = 1;
        return tmp;
    }
    static expand_int get_max() {
        expand_int tmp;
        tmp.val.unsigned_int.set_to_max();
        tmp.val.is_minus = 0;
        return tmp;
    }
public:
    static expand_int min;
    static expand_int max;


    expand_int() {
        val.unsigned_int = 0; 
    }
    expand_int(const expand_int& rhs) = default;
    expand_int(expand_int && rhs) = default;
    expand_int(const char* str) {
        bool set_minus = 0;
        if (*str++ == '-')
            set_minus = 1;
        if (!set_minus)
            str--;
        val.unsigned_int = expand_uint<byte_expand>(str);
        if (set_minus) {
            switch_to_siqn();
            val.is_minus = 1;
        }
    }
    template <typename T>
    expand_int(const T& rhs) 
    {
        val.unsigned_int = rhs;
    }
    std::string to_ansi_string() {
        if (val.is_minus)
            return 
                '-' + expand_int(*this).switch_to_unsiqn().
                                  val.unsigned_int.to_ansi_string();
        else 
            return val.unsigned_int.to_ansi_string();
    }
    std::string to_hex_str() {
        return val.unsigned_int.to_ansi_string();
    }

    expand_int& operator=(const expand_int & rhs) = default;
    expand_int& operator=(expand_int && rhs) = default;

    expand_int& operator++() {
        if (val.is_minus) {
            switch_my_siqn().val.unsigned_int += expand_uint<byte_expand>::exint_1;
            switch_my_siqn();
        }
        else 
            val.unsigned_int++;
        return *this;
    }
    expand_int operator++(int) {
        expand_int temp(*this);
        ++*this;
        return temp;
    }

    expand_int operator/(const expand_int & rhs) const {
        return expand_int(*this) /= rhs;
    }

    expand_int operator%(const expand_int & rhs) const {
        return expand_int(*this) %= rhs;
    }

    expand_int& operator/=(const expand_int & rhs) {
        if (val.is_minus && rhs.val.is_minus)
            switch_my_siqn().val.unsigned_int /= rhs.switch_my_siqn().val.unsigned_int;
        else if (val.is_minus || rhs.val.is_minus) {
            switch_to_unsiqn().val.unsigned_int /= rhs.switch_to_unsiqn().val.unsigned_int;
            switch_my_siqn();
        }
        else
            val.unsigned_int /= rhs.val.unsigned_int;
        return *this;
    }

    expand_int& operator%=(const expand_int & rhs) {
        if (val.is_minus && rhs.val.is_minus)
            switch_my_siqn().val.unsigned_int %= rhs.switch_my_siqn().val.unsigned_int;
        else if (val.is_minus || rhs.val.is_minus) {
            switch_to_unsiqn().val.unsigned_int %= rhs.switch_to_unsiqn().val.unsigned_int;
            switch_my_siqn();
        }
        else
            val.unsigned_int %= rhs.val.unsigned_int;
        return *this;
    }










    expand_int operator&(const expand_int & rhs) const {
        return expand_int(*this) &= rhs;
    }

    expand_int& operator&=(const expand_int & rhs) {
        val.unsigned_int &= rhs.val.unsigned_int;
        return *this;
    }

    expand_int operator|(const expand_int & rhs) const {
        return expand_int(*this) |= rhs;
    }

    expand_int& operator|=(const expand_int & rhs) {
        val.unsigned_int |= rhs.val.unsigned_int;
        return *this;
    }

    expand_int operator^(const expand_int & rhs) const {
        return expand_int(*this) ^= rhs;
    }

    expand_int& operator^=(const expand_int & rhs) {
        val.unsigned_int ^= rhs.val.unsigned_int;
        return *this;
    }

    expand_int operator~() const {
        expand_int tmp(*this);
        ~tmp.val.unsigned_int;
        return tmp;
    }



    expand_int& operator<<=(uint64_t shift) {
        val.unsigned_int <<= shift;
        return *this;
    }
    expand_int operator<<(uint64_t shift) const {
        return expand_int(*this) <<= shift;
    }


    expand_int& operator>>=(uint64_t shift) {
        val.unsigned_int >>= shift;
        return *this;
    }
    expand_int operator>>(uint64_t shift) const {
        return expand_int(*this) >>= shift;
    }
    bool operator!() const {
        return !val.unsigned_int;
    }

    bool operator&&(const expand_int & rhs) const {
        return ((bool)*this && rhs);
    }

    bool operator||(const expand_int & rhs) const {
        return ((bool)*this || rhs);
    }

    bool operator==(const expand_int & rhs) const {
        return val.unsigned_int == rhs.val.unsigned_int;
    }

    bool operator!=(const expand_int & rhs) const {
        return val.unsigned_int != rhs.val.unsigned_int;
    }

    bool operator>(const expand_int & rhs) const {
        if (val.is_minus && !rhs.val.is_minus)
            return false;
        if (!val.is_minus && rhs.val.is_minus)
            return true;
        return expand_int(*this).switch_to_siqn().val.unsigned_int > rhs.switch_to_siqn().val.unsigned_int;
    }

    bool operator<(const expand_int & rhs) const {
        if (val.is_minus && !rhs.val.is_minus)
            return true;
        if (!val.is_minus && rhs.val.is_minus)
            return false;
        return expand_int(*this).switch_to_siqn().val.unsigned_int < rhs.switch_to_siqn().val.unsigned_int;
    }

    bool operator>=(const expand_int & rhs) const {
        return ((*this > rhs) | (*this == rhs));
    }

    bool operator<=(const expand_int & rhs) const {
        return ((*this < rhs) | (*this == rhs));
    }

    expand_int operator+(const expand_int & rhs) const {
        return expand_int(*this) += rhs;
    }
    expand_int operator-(const expand_int & rhs) const {
        return expand_int(*this) -= rhs;
    }

    expand_int& operator+=(const expand_int & rhs) {
        if (val.is_minus && rhs.val.is_minus) {
            switch_my_siqn().val.unsigned_int += rhs.switch_my_siqn().val.unsigned_int;
            switch_to_siqn();
        }
        else if (val.is_minus) {
            if (switch_my_siqn().val.unsigned_int < rhs.val.unsigned_int) {
                val.unsigned_int = rhs.val.unsigned_int - val.unsigned_int;
                switch_to_unsiqn();
            }
            else {
                val.unsigned_int -= rhs.val.unsigned_int;
                switch_to_siqn();
            }
        }
        else if (rhs.val.is_minus) {
            val.unsigned_int -= rhs.switch_my_siqn().val.unsigned_int;
        }
        else 
            val.unsigned_int += rhs.val.unsigned_int;
        return *this;
    }
    expand_int& operator-=(const expand_int & rhs) {
        if (val.is_minus && rhs.val.is_minus) {
            switch_my_siqn().val.unsigned_int -= rhs.switch_my_siqn().val.unsigned_int;
            switch_my_siqn();
        }
        else if (val.is_minus) {
            switch_my_siqn().val.unsigned_int -= rhs.val.unsigned_int;
            if(val.unsigned_int)
                switch_to_unsiqn();
        }
        else if (rhs.val.is_minus) {
            if (switch_my_siqn().val.unsigned_int < rhs.val.unsigned_int) {
                val.unsigned_int = rhs.val.unsigned_int - val.unsigned_int;
                switch_to_siqn();
            }
            else
                val.unsigned_int -= rhs.val.unsigned_int;
        }
        else
            val.unsigned_int -= rhs.val.unsigned_int;
        return *this;
    }

    expand_int operator*(const expand_int & rhs) const {
        return expand_int(*this) *= rhs;
    }


    expand_int& operator*=(const expand_int & rhs) {
        if (val.is_minus && rhs.val.is_minus)
            switch_my_siqn().val.unsigned_int *= rhs.switch_my_siqn().val.unsigned_int;
        else if (val.is_minus || rhs.val.is_minus) {
            switch_to_unsiqn().val.unsigned_int *= rhs.switch_to_unsiqn().val.unsigned_int;
            switch_to_siqn();
        }
        else
            val.unsigned_int *= rhs.val.unsigned_int;
        return *this;
    }
    explicit operator bool() const {
        return (bool)val.unsigned_int;
    }
    explicit operator uint8_t() const {
        return (uint8_t)val.unsigned_int;
    }
    explicit operator uint16_t() const {
        return (uint16_t)val.unsigned_int;
    }

    explicit operator uint32_t() const {
        return (uint32_t)val.unsigned_int;
    }

    explicit operator uint64_t() const {
        return (uint64_t)val.unsigned_int;
    }
};

template<uint64_t byte_expand>
expand_int<byte_expand> expand_int<byte_expand>::min = expand_int<byte_expand>::get_min();

template<uint64_t byte_expand>
expand_int<byte_expand> expand_int<byte_expand>::max = expand_int<byte_expand>::get_max();


typedef expand_int<0> int128_ext;
typedef expand_int<1> int256_ext;
typedef expand_int<2> int512_ext;
typedef expand_int<3> int1024_ext;
typedef expand_int<4> int2048_ext;
typedef expand_int<5> int4096_ext;
typedef expand_int<6> int8192_ext;
typedef expand_int<7> int16384_ext;
typedef expand_int<8> int32768_ext;

typedef expand_int<0> int16_exb;
typedef expand_int<1> int32_exb;
typedef expand_int<2> int64_exb;
typedef expand_int<3> int128_exb;
typedef expand_int<4> int256_exb;
typedef expand_int<5> int512_exb;
typedef expand_int<6> int1024_exb;
typedef expand_int<7> int2048_exb;
typedef expand_int<8> int4096_exb;

//TO-DO expand_real<>