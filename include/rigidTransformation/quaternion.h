#ifndef QUATERNION_H
#define QUATERNION_H

#include <Eigen/Core>
#include <random>
#include <iostream>
#include <cmath>

#include "utils.h"

namespace rigidTransform
{

template<typename T>
class Quaternion
{
public:
    using Vec4T = Eigen::Matrix<T,4,1>;
    using Mat3T = Eigen::Matrix<T,3,3>;
    using Vec3T = Eigen::Matrix<T,3,1>;

    Quaternion() : arr_(data_)
    {
        arr_(0) = T(1.0);
    }

    Quaternion(const T* data) : arr_(const_cast<T*>(data)) {}

    Quaternion(const Vec4T &q) : arr_(data_) { arr_ = q; }

    Quaternion(const T& phi, const T& theta, const T& psi) : arr_(data_)
    {
        T cp{cos(phi/2.0)}, sp{sin(phi/2.0)};
        T ct{cos(theta/2.0)}, st{sin(theta/2.0)};
        T cps{cos(psi/2.0)}, sps{sin(psi/2.0)};

        Vec4T q{Vec4T::Zero()};
        q(0) = cps * ct * cp + sps * st * sp;
        q(1) = cps * ct * sp - sps * st * cp;
        q(2) = cps * st * cp + sps * ct * sp;
        q(3) = sps * ct * cp - cps * st * sp;

        arr_ = q;
    }

    Quaternion(const Mat3T &R) : arr_(data_)
    {
        T d{R.trace()};
        Vec4T q;
        if(d > 0)
        {
            T s{2 * sqrt(d + 1)};
            q << s/4, (R(1,2) - R(2,1))/s, (R(2,0) - R(0,2))/s, (R(0,1) - R(1,0))/s;
        }
        else if(R(0,0) > R(1,1) && R(0,0) > R(2,2))
        {
            T s{2 * sqrt(1 + R(0,0) - R(1,1) - R(2,2))};
            q << (R(1,2) - R(2,1))/s , s/4, (R(1,0) + R(0,1))/s, (R(2,0) + R(0,2))/s;
        }
        else if(R(1,1) > R(2,2))
        {
            T s{2 * sqrt(1 + R(1,1) - R(0,0) - R(2,2))};
            q << (R(2,0) - R(0,2))/s, (R(1,0) + R(0,1))/s, s/4, (R(2,1) + R(1,2))/s;
        }
        else
        {
            T s{2 * sqrt(1 + R(2,2) - R(0,0) - R(1,1))};
            q << (R(0,1) - R(1,0))/s, (R(2,0) + R(0,2))/s, (R(2,1) + R(1,2))/s, s/4;
        }
        q.template tail<3>() *= -1;

        arr_ = q;
    }

    Vec4T q() const { return arr_; }

    T qw() const { return arr_(0); }
    T qx() const { return arr_(1); }
    T qy() const { return arr_(2); }
    T qz() const { return arr_(3); }
    Vec3T qv() const { return arr_.template tail<3>(); }

    Mat3T R() const
    {
        Vec3T qv = this->qv();
        Mat3T R = (2 * pow(qw(),2) - 1) * Mat3T::Identity() - 2 * qw() * skew3<T>(qv) + 2 * qv * qv.transpose();
        return R;
    }

private:
    T data_[4];
public:
    Eigen::Map<Vec4T> arr_;
};
}

#endif
