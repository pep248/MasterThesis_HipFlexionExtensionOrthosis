#ifndef DEF_LIB_VEC4_H
#define DEF_LIB_VEC4_H

#include "vecn.h"
#include "utils.h"

/**
 * @defgroup Vec4 Vec4
 * @brief Generic 4-dimensions vector.
 * @ingroup Lib
 * @{
 */

/**
 * @brief Generic 4-dimensions vector.
 * @tparam T vector elements type.
 */
template<typename T>
class Vec4
{
public:
    /**
     * @brief Constructs a Vec4 with all components set to zero.
     */
    Vec4()
    {
        a = 0;
        b = 0;
        c = 0;
        d = 0;
    }

    /**
     * @brief Constructs a Vec4 with all components set to the desired values.
     * @param a first vector component.
     * @param b second vector component.
     * @param c third vector component.
     * @param d fourth vector component.
     */
    Vec4(T a, T b, T c, T d)
    {
        this->a = a;
        this->b = b;
        this->c = c;
        this->d = d;
    }

    /**
     * @brief Constructs a Vec4 from a VecN.
     * @param o VecN to initialize from.
     */
    Vec4(VecN<T,4> &o)
    {
        *this = o;
    }

    /**
     * @brief Constructs a Vec4 from a string.
     * @param expression the string to interpret. It should include four numbers
     * separated with commas or spaces. They can be surrounded with parenthesis.
     */
    Vec4(std::string expression)
    {
        fromString(expression);
    }

    /**
     * @brief Copy constructor.
     */
    Vec4(const Vec4 &o)
    {
        a = o.a;
        b = o.b;
        c = o.c;
        d = o.d;
    }

    /**
     * @brief Access a vector component from its index.
     * @param index component index [0-3].
     * @return A reference to the selected vector component.
     */
    T& operator[](int index)
    {
        switch(index)
        {
        case 0:
            return a;
        case 1:
            return b;
        case 2:
            return c;
        case 3:
            return d;
        default:
            throw std::runtime_error("Vec4[]: bad index.");
        }
    }

    /**
     * @brief Assigns o to this vector.
     * @param o the vector that this vector should be equal to.
     */
    void operator=(const Vec4 &o)
    {
        a = o.a;
        b = o.b;
        c = o.c;
        d = o.d;
    }

    /**
     * @brief Assigns a Vec4 from a string.
     * @param str the string to interpret. It should include four numbers
     * separated with commas or spaces. They can be surrounded with parenthesis.
     */
    void operator=(std::string str)
    {
        fromString(str);
    }

    /**
     * @brief Assigns a VecN of size 4 to this vector.
     * @param o the vector that this vector should be equal to.
     */
    void operator=(const VecN<T,4> &o)
    {
        a = o[0];
        b = o[1];
        c = o[2];
        d = o[3];
    }

    /**
     * @brief Compute the sum of this vector with another one.
     * @param o the other vector to sum.
     * @return A vector that is the sum of this vector and o.
     */
    Vec4 operator+(const Vec4 &o) const
    {
        return Vec4(a + o.a, b + o.b, c + o.c, d + o.d);
    }

    /**
     * @brief Compute the difference of this vector with another one (this - o).
     * @param o the vector to subtract to this vector.
     * @return A vector that is the difference of this vector and o.
     */
    Vec4 operator-(const Vec4 &o) const
    {
        return Vec4(a - o.a, b - o.b, c - o.c, d - o.d);
    }

    /**
     * @brief Adds a given vector to this vector.
     * @param o the vector to add to this vector.
     */
    void operator+=(const Vec4 &o)
    {
        a += o.a;
        b += o.b;
        c += o.c;
        d += o.d;
    }

    /**
     * @brief Compute the product of this vector with a scalar.
     * @param s the scalar to multiply this vector.
     * @return A vector that is the product of this vector and s.
     */
    Vec4 operator*(const T s) const
    {
        return Vec4(a * s, b * s, c * s, d * s);
    }

    /**
     * @brief Multiply the current vector by a scalar.
     * @param s the scalar number to multiply this vector.
     */
    void operator*=(const T s)
    {
        a *= s;
        b *= s;
        c *= s;
        d *= s;
    }

    /**
     * @brief Compute the division of this vector by a scalar.
     * @param s the scalar to divide this vector.
     * @return A vector that is the result of this vector divided by s.
     */
    Vec4 operator/(const T s) const
    {
        return Vec4(a / s, b / s, c / s, d / s);
    }

    /**
     * @brief Compute the element-wise product of this vector with another one.
     * @param o the other vector for the element-wise product.
     */
    Vec4 operator*(const Vec4 &o) const
    {
        return Vec4(a * o.a, b * o.b, c * o.c, d * o.d);
    }

    /**
     * @brief Multiply element-wise the current vector by another one.
     * @param o the other vector for the element-wise product.
     */
    void operator*=(const Vec4 &o)
    {
        a *= o.a;
        b *= o.b;
        c *= o.c;
        d *= o.d;
    }

    /**
     * @brief Compute the element-wise division of this vector with another one.
     * @param o the other vector for the element-wise division.
     */
    Vec4 operator/(const Vec4 &o) const
    {
        return Vec4(a / o.a, b / o.b, c / o.c, d / o.d);
    }

    /**
     * @brief Divide element-wise the current vector by another one.
     * @param o the other vector for the element-wise division.
     */
    void operator/=(const Vec4 &o)
    {
        a /= o.a;
        b /= o.b;
        c /= o.c;
        d /= o.d;
    }

    /**
     * @brief Checks if two vectors are equal.
     * @param o the vector to compare to this vector.
     * @return true the vectors have the same components, false otherwise.
     * @note For Vec4f and Vec4d, the exact floating-point comparison is used.
     */
    bool operator==(const Vec4 &o) const
    {
        return (a == o.a && b == o.b && c == o.c && d == o.d);
    }

    /**
     * @brief Checks if two vectors are different.
     * @param o the vector to compare to this vector.
     * @return false the vectors have the same components, true otherwise.
     * @note For Vec4f and Vec4d, the exact floating-point comparison is used.
     */
    bool operator!=(const Vec4 &o) const
    {
        return !((*this) == o);
    }

    /**
     * @brief Compute the sum of the vector components.
     * @return the sum of the vector components.
     */
    T sum() const
    {
        return a + b + c + d;
    }

    /**
     * @brief Computes the norm (or length) of the vector.
     * @return The norm of the vector.
     */
    T length() const
    {
        return (T) sqrt(a*a + b*b + c*c + d*d);
    }

    /**
     * @brief Normalizes the vector, such that its norm is 1.
     * @return The previous norm of the vector.
     */
    T normalize()
    {
        T length = length();

        if(length != 0)
        {
            a = a / length;
            b = b / length;
            c = c / length;
            d = d / length;
        }

        return length;
    }
    
    /**
     * @brief Sets all components of this vector to zero.
     */
    void nullify()
    {
        a = 0;
        b = 0;
        c = 0;
        d = 0;
    }

    /**
     * @brief Set all components with specified values.
     * @param a first vector component.
     * @param b second vector component.
     * @param c third vector component.
     * @param d fourth vector component.
     */
    void setValue(T a, T b, T c, T d)
    {
        this->a = a;
        this->b = b;
        this->c = c;
        this->d = d;
    }

    /**
     * @brief Assigns a Vec4 from a string.
     * @param str the string to interpret. It should include four numbers
     * separated with commas or spaces. They can be surrounded with parenthesis.
     */
    void fromString(std::string str)
    {
        std::istringstream separator;

        str.erase(str.find("("), 1);
        str.erase(str.find(")"));

        while(str.find(",") != std::string::npos)
            str.replace(str.find(","), 1, " ");

        separator.str(str);
        separator >> a >> b >> c >> d;
    }

    T a, ///< First vector component. Can be directly accessed.
      b, ///< Second vector component. Can be directly accessed.
      c, ///< Third vector component. Can be directly accessed.
      d; ///< Fourth vector component. Can be directly accessed.
};

/**
 * @brief Assigns a Vec4 from a stream.
 * Four numbers will be pulled out of stream, and assigned to the components of
 * the vector.
 * @param stream the stream to read.
 * @param vec the vector to affect.
 */
template <typename T>
std::istream& operator>>(std::istream& stream, Vec4<T> &vec)
{
    if(!(stream >> vec.a) || !(stream >> vec.b) ||
       !(stream >> vec.c) || !(stream >> vec.d))
    {
        throw std::runtime_error("Vec4: not enough values in the stream.");
    }

    return stream;
}

/**
 * @brief Prints a vector to a stream.
 * @param stream the stream to print to.
 * @param vec the vector to print, in the format "a b c d".
 */
template <typename T>
std::ostream& operator<<(std::ostream &stream, const Vec4<T> &vec)
{
    stream << vec.a << " " << vec.b << " " << vec.c << " " << vec.d;
    return stream;
}

typedef Vec4<int> Vec4i; ///< Four-dimensional vector of ints.
typedef Vec4<unsigned int> Vec4u; ///< Four-dimensional vector of unsigned ints.
typedef Vec4<float> Vec4f; ///< Four-dimensional vector of floats.
typedef Vec4<double> Vec4d; ///< Four-dimensional vector of doubles.

/**
 * @}
 */

#endif
