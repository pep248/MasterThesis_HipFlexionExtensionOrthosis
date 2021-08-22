#ifndef DEF_LIB_VEC3_H
#define DEF_LIB_VEC3_H

#include "vecn.h"
#include "utils.h"

/**
 * @defgroup Vec3 Vec3
 * @brief Generic 3-dimensions vector.
 * @ingroup Lib
 * @{
 */

/**
 * @brief Generic 3-dimensions vector.
 * @tparam T vector elements type.
 */
template<typename T>
class Vec3
{
public:
    /**
     * @brief Constructs a Vec3 with all components set to zero.
     */
    Vec3()
    {
        x = 0;
        y = 0;
        z = 0;
    }

    /**
     * @brief Constructs a Vec3 with all components set to the desired values.
     * @param x first vector component.
     * @param y second vector component.
     * @param z third vector component.
     */
    Vec3(T x, T y, T z)
    {
        this->x = x;
        this->y = y;
        this->z = z;
    }

    /**
     * @brief Constructs a Vec3 from a VecN.
     * @param o VecN to initialize from.
     */
    Vec3(VecN<T,3> &o)
    {
        *this = o;
    }

    /**
     * @brief Constructs a Vec3 from a string.
     * @param expression the string to interpret. It should include four numbers
     * separated with commas or spaces. They can be surrounded with parenthesis.
     */
    Vec3(std::string expression)
    {
        fromString(expression);
    }

    /**
     * @brief Copy constructor.
     */
    Vec3(const Vec3 &o)
    {
        x = o.x;
        y = o.y;
        z = o.z;
    }

    /**
     * @brief Assigns o to this vector.
     * @param o the vector that this vector should be equal to.
     */
    void operator=(const Vec3 &o)
    {
        x = o.x;
        y = o.y;
        z = o.z;
    }

    /**
     * @brief Assigns a VecN of size 3 to this vector.
     * @param o the vector that this vector should be equal to.
     */
    void operator=(const VecN<T,3> &o)
    {
        x = o[0];
        y = o[1];
        z = o[2];
    }

    /**
     * @brief Assigns a Vec3 from a string.
     * @param str the string to interpret. It should include four numbers separated
     * with commas or spaces. They can be surrounded with parenthesis.
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
    Vec3 operator+(const Vec3 &o) const
    {
        return Vec3(x + o.x, y + o.y, z + o.z);
    }

    /**
     * @brief Compute the difference of this vector with another one (this - o).
     * @param o the vector to subtract to this vector.
     * @return A vector that is the difference of this vector and o.
     */
    Vec3 operator-(const Vec3 &o) const
    {
        return Vec3(x - o.x, y - o.y, z - o.z);
    }

    /**
     * @brief Adds a given vector to this vector.
     * @param o the vector to add to this vector.
     */
    void operator+=(const Vec3 &o)
    {
        x += o.x;
        y += o.y;
        z += o.z;
    }

    /**
     * @brief Compute the product of this vector with a scalar.
     * @param o the scalar to multiply this vector.
     * @return A vector that is the product of this vector and s.
     */
    void operator-=(const Vec3 &o)
    {
        x -= o.x;
        y -= o.y;
        z -= o.z;
    }

    /**
     * @brief Multiply the current vector by a scalar.
     * @param s the scalar number to multiply this vector.
     */
    Vec3 operator*(const T s) const
    {
        return Vec3(x * s, y * s, z * s);
    }

    /**
     * @brief Multiply the current vector by a scalar.
     * @param s the scalar number to multiply this vector.
     */
    void operator*=(const T s)
    {
        x *= s;
        y *= s;
        z *= s;
    }

    /**
     * @brief Compute the element-wise product of this vector with another one.
     * @param o the other vector for the element-wise product.
     */
    Vec3 operator*(const Vec3 &o) const
    {
        return Vec3(x * o.x, y * o.y, z * o.z);
    }

    /**
     * @brief Multiply element-wise the current vector by another one.
     * @param o the other vector for the element-wise product.
     */
    void operator*=(const Vec3 &o)
    {
        x *= o.x;
        y *= o.y;
        z *= o.z;
    }

    /**
     * @brief Compute the element-wise division of this vector with another one.
     * @param o the other vector for the element-wise division.
     */
    Vec3 operator/(const Vec3 &o) const
    {
        return Vec3(x / o.x, y / o.y, z / o.z);
    }

    /**
     * @brief Divide element-wise the current vector by another one.
     * @param o the other vector for the element-wise division.
     */
    void operator/=(const Vec3 &o)
    {
        x /= o.x;
        y /= o.y;
        z /= o.z;
    }

    /**
     * @brief Checks if two vectors are equal.
     * @param o the vector to compare to this vector.
     * @return true the vectors have the same components, false otherwise.
     * @note For Vec2f and Vec2d, the exact floating-point comparison is used.
     */
    bool operator==(const Vec3 &o) const
    {
        return (x == o.x && y == o.y && z == o.z);
    }

    /**
     * @brief Checks if two vectors are different.
     * @param o the vector to compare to this vector.
     * @return false the vectors have the same components, true otherwise.
     * @note For Vec2f and Vec2d, the exact floating-point comparison is used.
     */
    bool operator!=(const Vec3 &o) const
    {
        return !((*this) == o);
    }

    /**
     * @brief Compute the sum of the vector components.
     * @return the sum of the vector components.
     */
    T sum() const
    {
        return x + y + z;
    }

    /**
     * @brief Computes the norm (or length) of the vector.
     * @return The norm of the vector.
     */
    T length() const
    {
        return (T) sqrt(x*x + y*y + z*z);
    }

    /**
     * @brief Normalizes the vector, such that its norm is 1.
     * @return The previous norm of the vector.
     */
    T normalize()
    {
        T length = this->length();

        if(length != 0)
        {
            x = x / length;
            y = y / length;
            z = z / length;
        }

        return length;
    }

    /**
     * @brief Sets all components of this vector to zero.
     */
    void nullify()
    {
        x = 0;
        y = 0;
        z = 0;
    }

    /**
     * @brief Set all components with specified values.
     * @param x first vector component.
     * @param y second vector component.
     * @param z third vector component.
     */
    void setValue(T x, T y, T z)
    {
        this->x = x;
        this->y = y;
        this->z = z;
    }

    /**
     * @brief Assigns a Vec3 from a string.
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
        separator >> x >> y >> z;
    }

    /**
     * @brief Rotates the vector around the X axis.
     * @param angle the angle of rotation [deg].
     */
    void rotateAroundX(T angle)
    {
        Vec3<T> initial(*this);

        angle = DEG_TO_RAD(angle);

        x = initial.x;
        y = initial.y*cos(angle) - initial.z*sin(angle);
        z = initial.y*sin(angle) + initial.z*cos(angle);
    }

    /**
     * @brief Rotates the vector around the Y axis.
     * @param angle the angle of rotation [deg].
     */
    void rotateAroundY(T angle)
    {
        Vec3<T> initial(*this);

        angle = DEG_TO_RAD(angle);

        x = initial.x*cos(angle) + initial.z*sin(angle);
        y = initial.y;
        z = -initial.x*sin(angle) + initial.z*cos(angle);
    }

    /**
     * @brief Rotates the vector around the Z axis.
     * @param angle the angle of rotation [deg].
     */
    void rotateAroundZ(T angle)
    {
        Vec3<T> initial(*this);

        angle = DEG_TO_RAD(angle);

        x = initial.x*cos(angle) - initial.y*sin(angle);
        y = initial.x*sin(angle) + initial.y*cos(angle);
        z = initial.z;
    }

    T x, ///< First vector component. Can be directly accessed.
      y, ///< Second vector component. Can be directly accessed.
      z; ///< Third vector component. Can be directly accessed.
};

/**
 * @brief Assigns a Vec3 from a stream.
 * Three numbers will be pulled out of stream, and assigned to the components of
 * the vector.
 * @param stream the stream to read.
 * @param vec the vector to affect.
 * @throw Throws a runtime_exception if there are not enough values in the
 * given stream.
 */
template <typename T>
std::istream& operator>>(std::istream& stream, Vec3<T> &vec)
{
    if(!(stream >> vec.x) || !(stream >> vec.y) || !(stream >> vec.z))
        throw std::runtime_error("Vec3: not enough values in the stream.");

    return stream;
}

/**
 * @brief Prints a vector to a stream.
 * @param stream the stream to print to.
 * @param vec the vector to print, in the format "x y z".
 */
template <typename T>
std::ostream& operator<<(std::ostream &stream, const Vec3<T> &vec)
{
    stream << vec.x << " " << vec.y << " " << vec.z;
    return stream;
}

typedef Vec3<int> Vec3i; ///< Three-dimensional vector of ints.
typedef Vec3<unsigned int> Vec3u; ///< Three-dimensional vector of unsigned ints.
typedef Vec3<float> Vec3f; ///< Three-dimensional vector of floats.
typedef Vec3<double> Vec3d; ///< Three-dimensional vector of doubles.

/**
 * @}
 */

#endif
