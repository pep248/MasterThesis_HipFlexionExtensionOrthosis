#ifndef DEF_LIB_VEC2_H
#define DEF_LIB_VEC2_H

#include "vecn.h"
#include "utils.h"

/**
 * @defgroup Vec2 Vec2
 * @brief Generic 2-dimensions vector.
 * @ingroup Lib
 * @{
 */

/**
 * @brief Generic 2-dimensions vector.
 * @tparam T vector elements type.
 */
template<typename T>
class Vec2
{
public:
    /**
     * @brief Constructs a Vec2 with all components set to zero.
     */
    Vec2()
    {
        x = 0;
        y = 0;
    }

    /**
     * @brief Constructs a Vec2 with all components set to the desired values.
     * @param x first vector component.
     * @param y second vector component.
     */
    Vec2(T x, T y)
    {
        this->x = x;
        this->y = y;
    }

    /**
     * @brief Constructs a Vec2 from a VecN.
     * @param o VecN to initialize from.
     */
    Vec2(VecN<T,2> &o)
    {
        *this = o;
    }

    /**
     * @brief Constructs a Vec2 from a string.
     * @param expression the string to interpret. It should include four numbers
     * separated with commas or spaces. They can be surrounded with parenthesis.
     */
    Vec2(std::string expression)
    {
        fromString(expression);
    }

    /**
     * @brief Copy constructor.
     */
    Vec2(const Vec2 &o)
    {
        x = o.x;
        y = o.y;
    }


    /**
     * @brief Assigns o to this vector.
     * @param o the vector that this vector should be equal to.
     */
    void operator=(const Vec2 &o)
    {
        x = o.x;
        y = o.y;
    }

    /**
     * @brief Assigns a VecN of size 2 to this vector.
     * @param o the vector that this vector should be equal to.
     */
    void operator=(const VecN<T,2> &o)
    {
        x = o[0];
        y = o[1];
    }

    /**
     * @brief Assigns a Vec2 from a string.
     * @param str the string to interpret. It should include four numbers
     * separated with commas or spaces. They can be surrounded with parenthesis.
     */
    void operator=(std::string str)
    {
        fromString(str);
    }

    /**
     * @brief Compute the sum of this vector with another one.
     * @param o the other vector to sum.
     * @return A vector that is the sum of this vector and o.
     */
    Vec2 operator+(const Vec2 &o) const
    {
        return Vec2(x + o.x, y + o.y);
    }

    /**
     * @brief Compute the difference of this vector with another one (this - o).
     * @param o the vector to subtract to this vector.
     * @return A vector that is the difference of this vector and o.
     */
    Vec2 operator-(const Vec2 &o) const
    {
        return Vec2(x - o.x, y - o.y);
    }

    /**
     * @brief Adds a given vector to this vector.
     * @param o the vector to add to this vector.
     */
    void operator+=(const Vec2 &o)
    {
        x += o.x;
        y += o.y;
    }

    /**
     * @brief Compute the product of this vector with a scalar.
     * @param s the scalar to multiply this vector.
     * @return A vector that is the product of this vector and s.
     */
    Vec2 operator*(const T s) const
    {
        return Vec2(x * s, y * s);
    }

    /**
     * @brief Multiply the current vector by a scalar.
     * @param s the scalar number to multiply this vector.
     */
    void operator*=(const T s)
    {
        x *= s;
        y *= s;
    }

    /**
     * @brief Compute the scaled down vector with a scalar.
     * @param s the scalar to divide this vector.
     * @return The scaled down vector.
     */
    Vec2 operator/(const T s) const
    {
        return Vec2(x / s, y / s);
    }

    /**
     * @brief Scale down the current vector by a scalar.
     * @param s the scalar number to scale down this vector.
     */
    void operator/=(const T s)
    {
        x /= s;
        y /= s;
    }

    /**
     * @brief Compute the element-wise product of this vector with another one.
     * @param o the other vector for the element-wise product.
     */
    Vec2 operator*(const Vec2 &o) const
    {
        return Vec2(x * o.x, y * o.y);
    }

    /**
     * @brief Multiply element-wise the current vector by another one.
     * @param o the other vector for the element-wise product.
     */
    void operator*=(const Vec2 &o)
    {
        x *= o.x;
        y *= o.y;
    }

    /**
     * @brief Compute the element-wise division of this vector with another one.
     * @param o the other vector for the element-wise division.
     */
    Vec2 operator/(const Vec2 &o) const
    {
        return Vec2(x / o.x, y / o.y);
    }

    /**
     * @brief Divide element-wise the current vector by another one.
     * @param o the other vector for the element-wise division.
     */
    void operator/=(const Vec2 &o)
    {
        x /= o.x;
        y /= o.y;
    }

    /**
     * @brief Computes the dot of this vector with another one.
     * @param o the other vector to compute the dot product with.
     * @return the dot product of this vector and o.
     */
    float dot(const Vec2<T> &o) const
    {
        return x*o.x + y*o.y;
    }

    /**
     * @brief Checks if two vectors are equal.
     * @param o the vector to compare to this vector.
     * @return true the vectors have the same components, false otherwise.
     * @note For Vec2f and Vec2d, the exact floating-point comparison is used.
     */
    bool operator==(const Vec2 &o) const
    {
        return (x == o.x && y == o.y);
    }

    /**
     * @brief Checks if two vectors are different.
     * @param o the vector to compare to this vector.
     * @return false the vectors have the same components, true otherwise.
     * @note For Vec2f and Vec2d, the exact floating-point comparison is used.
     */
    bool operator!=(const Vec2 &o) const
    {
        return !((*this) == o);
    }

    /**
     * @brief Compute the sum of the vector components.
     * @return the sum of the vector components.
     */
    T sum() const
    {
        return x + y;
    }

    /**
     * @brief Computes the norm (or length) of the vector.
     * @return The norm of the vector.
     */
    T length() const
    {
        return (T) sqrt(x*x + y*y);
    }

    /**
     * @brief Normalizes the vector, such that its norm is 1.
     * @return The previous norm of the vector.
     */
    T normalize()
    {
        T previousNorm = length();

        if(previousNorm != 0)
        {
            x = x / previousNorm;
            y = y / previousNorm;
        }

        return previousNorm;
    }
    
    /**
     * @brief Sets all components of this vector to zero.
     */
    void nullify()
    {
        x = 0;
        y = 0;
    }

    /**
     * @brief Rotates the vector around the origin.
     * @param angle rotation angle [deg]. A positive angle results in a rotation
     * in the trigonometric positive direction.
     */
    Vec2<T>& rotate(float angle)
    {
        Vec2<T> initial(*this);

        angle = DEG_TO_RAD(angle);

        x = initial.x*cos(angle) - initial.y*sin(angle);
        y = initial.x*sin(angle) + initial.y*cos(angle);

        return *this;
    }

    /**
     * @brief Set all components with specified values.
     * @param x first vector component.
     * @param y second vector component.
     */
    void setValue(T x, T y)
    {
        this->x = x;
        this->y = y;
    }

    /**
     * @brief Assigns a Vec2 from a string.
     * @param str the string to interpret. It should include two numbers
     * separated with a comma or space. They can be surrounded with parenthesis.
     */
    void fromString(std::string str)
    {
        std::istringstream separator;

        str.erase(str.find("("), 1);
        str.erase(str.find(")"));

        while(str.find(",") != std::string::npos)
            str.replace(str.find(","), 1, " ");

        separator.str(str);
        separator >> x >> y;
    }

    T x, ///< First vector component. Can be directly accessed.
      y; ///< Second vector component. Can be directly accessed.
};

/**
 * @brief Assigns a Vec2 from a stream.
 * Two numbers will be pulled out of stream, and assigned to the components of
 * the vector.
 * @param stream the stream to read.
 * @param vec the vector to affect.
 * @throw Throws a runtime_exception if there are not enough values in the
 * given stream.
 */
template <typename T>
std::istream& operator>>(std::istream& stream, Vec2<T> &vec)
{
    if(!(stream >> vec.x) || !(stream >> vec.y))
        throw std::runtime_error("Vec2: not enough values in the stream.");

    return stream;
}

/**
 * @brief Prints a vector to a stream.
 * @param stream the stream to print to.
 * @param vec the vector to print, in the format "x y".
 */
template <typename T>
std::ostream& operator<<(std::ostream &stream, const Vec2<T> &vec)
{
    stream << vec.x << " " << vec.y;
    return stream;
}

/**
 * @brief Inverts the given vector.
 * @param The vector to invert.
 * @return The inverted vector.
 */
template <typename T>
Vec2<T> operator-(const Vec2<T> &v)
{
    return Vec2<T>(-v.x, -v.y);
}

/**
 * @brief Scales the vector by a scalar.
 * @param s scalar to scale the vector.
 * @param v vector to scale.
 * @return the scaled vector, each component multiplied by the given scalar.
 */
template <typename T>
Vec2<T> operator*(T s, Vec2<T> v)
{
    return Vec2<T>(s*v.x, s*v.y);
}

typedef Vec2<int> Vec2i; ///< Two-dimensional vector of ints.
typedef Vec2<unsigned int> Vec2u; ///< Two-dimensional vector of unsigned ints.
typedef Vec2<float> Vec2f; ///< Two-dimensional vector of floats.
typedef Vec2<double> Vec2d; ///< Two-dimensional vector of doubles.

/**
 * @}
 */

#endif
