#include <gtest/gtest.h>
#include <Eigen/Core>
#include <eigen3/unsupported/Eigen/MatrixFunctions>
#include <iostream>
#include <random>

#include "so2.h"

//The SO2 class is templated but I mostly care that it works with doubles and ceres jet
double getAngle(const Eigen::Matrix2d &mat)
{
    double y{mat(1,0)}, x{mat(0,0)};
    return atan2(y, x);
}

double wrap(double ang)
{
    const double pi{3.14159265};
    ang = ang > pi ? ang - 2 * pi : ang;
    ang = ang <= -pi ? ang + 2 * pi : ang;
    return ang;
}

double getRandomDouble(double min, double max)
{
    static std::random_device rd;
    static std::mt19937 generator(rd());
    static std::uniform_real_distribution<double> dist(min, max);
    return dist(generator);
}

Eigen::Vector2d randVec2d(double min, double max)
{
    Eigen::Vector2d vec;
    vec << getRandomDouble(min, max), getRandomDouble(min, max);
    return vec;
}


TEST(AskForTheRotationMatrix, ReturnsTheRotationMatrix)
{
    Eigen::Matrix2d R_true;
    R_true << 1, 0, 0, 1;

    SO2<double> R(R_true);

    EXPECT_TRUE(R_true.isApprox(R.R()));
}

TEST(AskForRandomMatrix, ReturnsMatrixWithDeterminantOne)
{
    for(int i{0}; i != 100; ++i)
    {
        SO2<double> R = SO2<double>::random();
        EXPECT_TRUE(R.isValidRotation());
    }
}

TEST(AskForMatrixFromAngle, ReturnsRotationMatrix)
{
    for(int i{0}; i != 100; ++i)
    {
        SO2<double> R = SO2<double>::random();
        double ang = atan2(R.R()(1,0), R.R()(0,0));
        SO2<double> R2 = SO2<double>::fromAngle(ang);
        EXPECT_TRUE(R.R().isApprox(R2.R()));
    }
}

TEST(AskForMatrixFromArray, ReturnsRotationMatrix)
{
    for(int i{0}; i != 100; ++i)
    {
        SO2<double> R{SO2<double>::random()};
        double ang(getAngle(R.R()));
        double ct{cos(ang)}, st{sin(ang)};
        // double *arr{new double[4]};
        double arr[4]{ct, st, -st, ct}; //Order this way because column major order
        SO2<double> R2{arr};

        EXPECT_TRUE(R == R2);
    }
}

TEST(ResultOfGroupMultiplication, ReturnsNewMemberOfGroup)
{
    for(int i{0}; i != 100; ++i)
    {
        SO2<double> R1 = SO2<double>::random();
        SO2<double> R2 = SO2<double>::random();

        SO2<double> R3 = R1 * R2;
        double ang1{getAngle(R1.R())}, ang2{getAngle(R2.R())};
        double ang3{ang1 + ang2};
        ang3 = wrap(ang3);
        SO2<double> R3_true = SO2<double>::fromAngle(ang3);

        EXPECT_TRUE(R3_true.R().isApprox(R3.R(), 1e-8));
    }
}

TEST(InverseOfSO2Object, ReturnsIdentityWhenMultipliedByInverse) //FIX THIS ONE
{
    for(int i{0}; i != 100; i++)
    {
        SO2<double> R{SO2<double>::random()};
        SO2<double> R_inv{R.inv()};

        SO2<double> I = R * R_inv;

        EXPECT_TRUE(I.R().isApprox(Eigen::Matrix2d::Identity()));
    }
}

TEST(InverseInPlace, InvertsObject)
{
    for(int i{0}; i != 100; ++i)
    {
        SO2<double> R{SO2<double>::random()};
        Eigen::Matrix2d inv = R.R().transpose();
        R.selfInv();

        EXPECT_TRUE(inv.isApprox(R.R()));
    }
}

TEST(ActiveRotation, RotatedVector)
{
    for(int i{0}; i != 100; ++i)
    {
        SO2<double> R{SO2<double>::random()};
        Eigen::Vector2d vec{randVec2d(-10.0, 10.0)};

        Eigen::Vector2d res{R.rota(vec)};

        double ang{getAngle(R.R())};
        double ct{cos(ang)}, st{sin(ang)};
        Eigen::Vector2d res_true{Eigen::Vector2d::Zero()};
        res_true(0) = ct * vec(0) - st * vec(1);
        res_true(1) = st * vec(0) + ct * vec(1);

        EXPECT_TRUE(res_true.isApprox(res));
    }
}

TEST(PassiveRotation, RotatedVector)
{
    for(int i{0}; i != 100; ++i)
    {
        SO2<double> R{SO2<double>::random()};
        Eigen::Vector2d vec{randVec2d(-10.0, 10.0)};

        Eigen::Vector2d res{R.rotp(vec)};

        double ang{getAngle(R.R())};
        double ct{cos(ang)}, st{sin(ang)};
        Eigen::Vector2d res_true{Eigen::Vector2d::Zero()};
        res_true(0) = ct * vec(0) + st * vec(1);
        res_true(1) = -st * vec(0) + ct * vec(1);

        EXPECT_TRUE(res_true.isApprox(res));
    }
}

TEST(HatOperator, GivenAnAngleReturnSkewSymmetricMatrix)
{
    for(int i{0}; i != 100; ++i)
    {
        SO2<double> R{SO2<double>::random()};
        double angle{getAngle(R.R())};

        Eigen::Matrix2d r{SO2<double>::hat(angle)};
        Eigen::Matrix2d r_true;
        r_true << 0.0, -angle, angle, 0.0;

        EXPECT_TRUE(r_true.isApprox(r));
    }
}

TEST(VeeOperator, GivenSkewSymmetricMatrixReturnAngle)
{
    for(int i{0}; i != 100; ++i)
    {
        SO2<double> R{SO2<double>::random()};
        double angle{getAngle(R.R())};

        Eigen::Matrix2d tmp{SO2<double>::hat(angle)};
        double ang{SO2<double>::vee(tmp)};

        EXPECT_TRUE(ang==angle);
    }
}

TEST(MatrixLogarithm, GivenSO2Element_ReturnMatrixLog)
{
    for(int i{0}; i != 100; ++i)
    {
        SO2<double> R{SO2<double>::random()};

        Eigen::Matrix2d log_R{SO2<double>::log(R)};
        Eigen::Matrix2d log_R2(SO2<double>::log(R.R()));
        Eigen::Matrix2d log_R3(R.log());
        Eigen::Matrix2d log_R_true{R.R().log()};

        EXPECT_TRUE(log_R_true.isApprox(log_R));
        EXPECT_TRUE(log_R_true.isApprox(log_R2));
        EXPECT_TRUE(log_R_true.isApprox(log_R3));
    }
}

TEST(MatrixLogarithm, GivenSO2Element_ReturnsDouble)
{
    for(int i{0}; i != 100; ++i)
    {
        SO2<double> R{SO2<double>::random()};

        double phi(SO2<double>::Log(R));
        double phi2(SO2<double>::Log(R.R()));
        double phi3(R.Log());
        double phi_true{getAngle(R.R())};

        EXPECT_TRUE(phi_true == phi);
        EXPECT_TRUE(phi_true == phi2);
        EXPECT_TRUE(phi_true == phi3);
    }
}

TEST(MatrixExponential, GivenSkewSymetricMatrix_ReturnsSO2Element)
{
    for(int i{0}; i != 100; ++i)
    {
        SO2<double> R{SO2<double>::random()};

        Eigen::Matrix2d log_R{R.log()};
        SO2<double> R2{SO2<double>::exp(log_R)};

        EXPECT_TRUE(R == R2);
    }
}

TEST(MatrixExponential, GivenDouble_ReturnSO2Element)
{
    for(int i{0}; i != 100; ++i)
    {
        SO2<double> R{SO2<double>::random()};

        double phi{R.Log()};
        SO2<double> R2{SO2<double>::Exp(phi)};

        EXPECT_TRUE(R == R2);
    }
}

TEST(Ajoint, GivenSO2ElementAndAngle_TestAdjoint)
{
    for(int i{0}; i != 100; ++i)
    {
        SO2<double> R{SO2<double>::random()};
        double phi{R.Log()};

        SO2<double> R1{R * SO2<double>::Exp(phi)};
        SO2<double> R2{SO2<double>::Exp(phi) * R}; //Adj is identity

        EXPECT_TRUE(R1 == R2);
    }
}

TEST(BoxPlus, GivenSO2AndDelta_ReturnNewSO2)
{
    for(int i{0}; i!=100; ++i)
    {
        SO2<double> R{SO2<double>::random()};
        double delta{getRandomDouble(-PI, PI)};
        double phi = getAngle(R.R());

        SO2<double> R2_true{SO2<double>::fromAngle(delta + phi)};
        SO2<double> R2{R.boxplus(delta)};

        EXPECT_TRUE(R2 == R2_true);
    }
}

TEST(BoxMinus, Given2SO2Elements_ReturnDelta)
{
    for(int i{0}; i!=100; ++i)
    {
        SO2<double> R1{SO2<double>::random()};
        SO2<double> R2{SO2<double>::random()};
        double phi1{getAngle(R1.R())}, phi2{getAngle(R2.R())};

        double delta{R1.boxminus(R2)};
        SO2<double> R{R2.boxplus(delta)};

        EXPECT_TRUE(R1 == R);
    }
}

TEST(Identity, AskedForIdentity_ReturnsIdentity)
{
    Eigen::Matrix2d I{Eigen::Matrix2d::Identity()};
    SO2<double> R{SO2<double>::Identity()};

    EXPECT_TRUE(I.isApprox(R.R()));
}
