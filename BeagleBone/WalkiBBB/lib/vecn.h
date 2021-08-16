#ifndef DEF_LIB_VECN_H
#define DEF_LIB_VECN_H

#include <string>
#include <sstream>
#include <cmath>
#include <array>
#include <initializer_list>
#include <algorithm>
#include <numeric>

/**
 * @defgroup VecN VecN
 * @brief Generic N-dimensions vector.
 * @ingroup Lib
 * @{
 */

/**
 * @brief Generic N-dimensions vector.
 * @tparam T vector elements type.
 * @tparam N number of elements of the vector.
 */
template<typename T, std::size_t N>
class VecN
{
public:
    /**
     * @brief Constructs a VecN with all components set to zero.
     */
    VecN()
    {
        values.fill((T)0);
    }

    /**
     * @brief Constructs a VecN with all components set to the desired values.
     * @param values values to affect to the vector elements.
     * @throw Throws a runtime_error if the size of the given list does not
     * match the vector size.
     */
    VecN(std::initializer_list<T> values)
    {
        if(values.size() == N)
        {
            int i = 0;

            for(auto it = values.begin(); it != values.end(); it++)
            {
                this->values[i] = *it;
                i++;
            }
        }
        else
            throw std::runtime_error("VecN(): wrong values list size.");
    }

    /**
     * @brief Constructs a VecN from a string.
     * @param expression the string to interpret. It should include numbers
     * separated with commas or spaces. They can be surrounded with parenthesis.
     */
    VecN(std::string expression)
    {
        fromString(expression);
    }

    /**
     * @brief Copy constructor.
     */
    VecN(const VecN &o)
    {
        values = o.values;
    }

    /**
     * @brief Gets a reference to a vector component from its index.
     * @param index: vector index of the component to get.
     * @return A reference to the selected component of the vector.
     */
    T& operator[](int index)
    {
        return values[index];
    }

    /**
     * @brief Gets a vector component from its index.
     * @param index: vector index of the component to get.
     * @return The value of the selected component of the vector.
     */
    T operator[](int index) const
    {
        return values[index];
    }

    /**
     * @brief Assigns o to this vector.
     * @param o the vector that this vector should be equal to.
     */
    void operator=(const VecN &o)
    {
        values = o.values;
    }

    /**
     * @brief Assigns a VecN from a string.
     * @param str the string to interpret. It should include numbers separated
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
    VecN operator+(const VecN &o) const
    {
        VecN<T, N> result;

        for(size_t i=0; i<N; i++)
            result[i] = values[i] + o[i];

        return result;
    }

    /**
     * @brief Compute the difference of this vector with another one (this - o).
     * @param o the vector to subtract to this vector.
     * @return A vector that is the difference of this vector and o.
     */
    VecN operator-(const VecN &o) const
    {
        VecN<T, N> result;

        for(size_t i=0; i<N; i++)
            result[i] = values[i] - o[i];

        return result;
    }

    /**
     * @brief Adds a given vector to this vector.
     * @param o the vector to add to this vector.
     */
    void operator+=(const VecN &o)
    {
        for(size_t i=0; i<N; i++)
            values[i] += o[i];
    }

    /**
     * @brief Compute the product of this vector with a scalar.
     * @param s the scalar to multiply this vector.
     * @return A vector that is the product of this vector and s.
     */
    VecN operator*(const T s) const
    {
        VecN<T, N> result;

        for(size_t i=0; i<N; i++)
            result[i] = values[i] * s;

        return result;
    }

    /**
     * @brief Multiply the current vector by a scalar.
     * @param s the scalar number to multiply this vector.
     */
    void operator*=(const T s)
    {
        for(size_t i=0; i<N; i++)
            values[i] *= s;
    }

    /**
     * @brief Compute the division of this vector by a scalar.
     * @param s the scalar to divide this vector.
     * @return A vector that is the result of this vector divided by s.
     */
    VecN operator/(const T s) const
    {
        VecN<T, N> result;

        for(size_t i=0; i<N; i++)
            result[i] = values[i] / s;

        return result;
    }

    /**
     * @brief Compute the element-wise product of this vector with another one.
     * @param o the other vector for the element-wise product.
     */
    VecN operator*(const VecN<T, N> &o) const
    {
        VecN<T, N> result;

        for(size_t i=0; i<N; i++)
            result[i] = values[i] * o.values[i];

        return result;
    }

    /**
     * @brief Multiply element-wise the current vector by another one.
     * @param o the other vector for the element-wise product.
     */
    void operator*=(const VecN<T, N> &o)
    {
        for(size_t i=0; i<N; i++)
            values[i] *= o.values[i];
    }

    /**
     * @brief Compute the element-wise division of this vector with another one.
     * @param o the other vector for the element-wise division.
     */
    VecN operator/(const VecN<T, N> &o) const
    {
        VecN<T, N> result;

        for(size_t i=0; i<N; i++)
            result[i] /= o.values[i];

        return result;
    }

    /**
     * @brief Divide element-wise the current vector by another one.
     * @param o the other vector for the element-wise division.
     */
    void operator/=(const VecN<T, N> &o)
    {
        for(size_t i=0; i<N; i++)
            values[i] /= o.values[i];
    }

    /**
     * @brief Checks if two vectors are equal.
     * @param o the vector to compare to this vector.
     * @return true the vectors have the same components, false otherwise.
     * @note For VecNf and VecNd, the exact floating-point comparison is used.
     */
    bool operator==(const VecN &o) const
    {
        return values == o.values;
    }

    /**
     * @brief Checks if two vectors are different.
     * @param o the vector to compare to this vector.
     * @return false the vectors have the same components, true otherwise.
     * @note For VecNf and VecNd, the exact floating-point comparison is used.
     */
    bool operator!=(const VecN &o) const
    {
        return values != o.values;
    }
	
	/**
     * @brief Compute the sum of the vector components.
     * @return the sum of the vector components.
     */
    T sum() const
    {
        return std::accumulate(values.begin(), values.end(), 0.0f);
    }

    /**
     * @brief Computes the norm (or length) of the vector.
     * @return The norm of the vector.
     */
    T length() const
    {
        T squaredSum = (T)0;

        for(size_t i=0; i<N; i++)
            squaredSum += values[i] * values[i];

        return (T) sqrt(squaredSum);
    }

    /**
     * @brief Normalizes the vector, such that its norm is 1.
     * @return The previous norm of the vector.
     */
    T normalize()
    {
        T length = length();

        if(length != (T)0)
        {
            for(size_t i=0; i<N; i++)
                values[i] = values[i] / length;
        }

        return length;
    }

    /**
     * @brief Sets all components of this vector to zero.
     */
    void nullify()
    {
        values.fill((T)0);
    }

    /**
     * @brief Set all components with specified values.
     * @param values initializer_list with the new vector values.
     * @throw Throws a runtime_error if the size of the given list does not
     * match the vector size.
     */
    void setValue(std::initializer_list<T> values)
    {
        if(values.size() == N)
        {
            this->values = values;
        }
        else
            throw std::runtime_error("VecN::setValue(): wrong values list size.");
    }

    /**
     * @brief Assigns a VecN from a string.
     * @param str the string to interpret. It should include four numbers
     * separated with commas or spaces. They can be surrounded with parenthesis.
     * @throw Throws a runtime_error if the size of the given list does not
     * match the vector size.
     */
    void fromString(std::string str)
    {
        std::istringstream separator;

        str.erase(str.find("("), 1);
        str.erase(str.find(")"));

        while(str.find(",") != std::string::npos)
            str.replace(str.find(","), 1, " ");

        separator.str(str);

        for(size_t i=0; i<N; i++)
        {
            if(separator)
                separator >> values[i];
            else
                throw std::runtime_error("VecN::fromString(): wrong values list size.");
        }
    }

private:
    std::array<T,N> values; ///< Components of the vector.
};

/**
 * @brief Assigns a VecN from a stream.
 * Reads N numbers from the stream (with N equal to the size of the vector), and
 * assign them to the components of the vector.
 * @param stream the stream to read.
 * @param vec the vector to affect.
 */
template <typename T, size_t N>
std::istream& operator>>(std::istream& stream, VecN<T,N> &vec)
{
    for(size_t i = 0; i<N; i++)
    {
        if(!(stream >> vec[i]))
        {
            throw std::runtime_error("VecN: not enough values in the stream.");
            break;
        }
    }

    return stream;
}

/**
 * @brief Prints a vector to a stream.
 * @param stream the stream to print to.
 * @param vec the vector to print, in the format "v1 v2 v3...".
 */
template <typename T, size_t N>
std::ostream& operator<<(std::ostream &stream, const VecN<T,N> &vec)
{
    for(size_t i=0; i<N; i++)
    {
        stream << vec[i];

        if(i < N-1)
            stream << " ";
    }

    return stream;
}

template <size_t N>
using VecNi = VecN<int, N>; ///< N-dimensional vector of ints.
template <size_t N>
using VecNu = VecN<unsigned int, N>; ///< N-dimensional vector of unsigned ints.
template <size_t N>
using VecNf = VecN<float, N>; ///< N-dimensional vector of floats.
template <size_t N>
using VecNd = VecN<double, N>; ///< N-dimensional vector of doubles.

/**
 * @}
 */

#endif
