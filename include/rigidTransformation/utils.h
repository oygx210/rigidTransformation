#ifndef UTILS_H
#define UTILS_H

#include <Eigen/Core>

constexpr double PI = 3.141592653589793238462643383279;

template<typename T>
Eigen::Matrix<T,3,3> skew3(const Eigen::Matrix<T,3,1> &v)
{
    Eigen::Matrix<T,3,3> mat;
    mat << T(0.0), -v(2), v(1), v(2), T(0.0), -v(0), -v(1), v(0), T(0.0);
    return mat;
}

template<typename T>
Eigen::Matrix<T,2,2> skew2(const T &v)
{
    Eigen::Matrix<T,2,2> mat;
    mat << T(0.0), -v, v, T(0.0);
    return mat;
}

template<int n>
Eigen::Matrix<double,n,1> ei(const int i)
{
    Eigen::Matrix<double,n,1> arr{Eigen::Matrix<double,n,1>::Zero()};
    if(i > n || i < 0)
        return arr;

    arr(i) = 1.0;    
    return arr;
}

#endif