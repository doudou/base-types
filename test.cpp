#define BOOST_TEST_MODULE BaseTypes
#include <boost/test/included/unit_test.hpp>

#include "base/angle.h"
#include "base/time.h"
#include "base/timemark.h"
#include "base/pose.h"
#include "base/samples/frame.h"
#include "base/samples/imu.h"
#include "base/samples/laser_scan.h"
#include "base/samples/sonar_beam.h"
#include "base/samples/rigid_body_state.h"

#define BASE_LOG_DEBUG
#include "base/logging.h"

#include <Eigen/SVD>
#include <Eigen/LU>

using namespace std;

BOOST_AUTO_TEST_CASE( time_test )
{
    std::cout << base::Time::fromSeconds( 35.553 ) << std::endl;
    std::cout << base::Time::fromSeconds( -5.553 ) << std::endl;
}

BOOST_AUTO_TEST_CASE( time_fromSeconds )
{
    base::Time seconds;

    seconds = base::Time::fromSeconds( 35.553 );
    BOOST_REQUIRE_EQUAL( 35553000, seconds.toMicroseconds() );
    seconds = base::Time::fromSeconds( -5.553 );
    BOOST_REQUIRE_EQUAL( -5553000, seconds.toMicroseconds() );
    seconds = base::Time::fromSeconds( 0.01 );
    BOOST_REQUIRE_EQUAL( 10000, seconds.toMicroseconds() );
}

BOOST_AUTO_TEST_CASE( time_multiply )
{
    base::Time t = base::Time::fromSeconds( 35 );
    BOOST_REQUIRE_EQUAL( 35 * 1e6 * 0.025, (t * 0.025).toMicroseconds() );
}

BOOST_AUTO_TEST_CASE( laser_scan_test )
{
    //configure laser scan
    base::samples::LaserScan laser_scan;
    laser_scan.start_angle = M_PI*0.25;
    laser_scan.angular_resolution = M_PI*0.01;
    laser_scan.speed = 330;
    laser_scan.minRange = 1000;
    laser_scan.maxRange = 20000;

    //add some points
    laser_scan.ranges.push_back(1000);
    laser_scan.ranges.push_back(1000);
    laser_scan.ranges.push_back(2000);
    laser_scan.ranges.push_back(999);
    laser_scan.ranges.push_back(2000);

    Eigen::Affine3d trans;
    trans.setIdentity();
    trans.translation() = Eigen::Vector3d(-1.0,0.0,0.0);
    std::vector<Eigen::Vector3d> points;
    laser_scan.convertScanToPointCloud(points,trans,false);

    //check translation
    BOOST_CHECK(points.size() == 5);
    BOOST_CHECK(abs(points[0].x()-( -1+cos(M_PI*0.25))) < 0.000001);
    BOOST_CHECK(abs(points[0].y()-( sin(M_PI*0.25))) < 0.000001);
    BOOST_CHECK(points[0].z() == 0);
    BOOST_CHECK(abs(points[1].x()-( -1+cos(M_PI*0.25+laser_scan.angular_resolution))) < 0.000001);
    BOOST_CHECK(abs(points[1].y()-( sin(M_PI*0.25+laser_scan.angular_resolution))) < 0.000001);
    BOOST_CHECK(abs(points[2].x()-( -1+2.0*cos(M_PI*0.25+laser_scan.angular_resolution*2))) < 0.000001);
    BOOST_CHECK(abs(points[2].y()-( 2.0*sin(M_PI*0.25+laser_scan.angular_resolution*2))) < 0.000001);
    BOOST_CHECK(isnan(points[3].x()));
    BOOST_CHECK(isnan(points[3].y()));
    BOOST_CHECK(isnan(points[3].z()));

    //check rotation and translation
    trans.setIdentity();
    trans.translation() = Eigen::Vector3d(-1.0,0.0,0.0);
    trans.rotate(Eigen::AngleAxisd(0.1*M_PI,Eigen::Vector3d::UnitZ()));
    laser_scan.convertScanToPointCloud(points,trans,false);
    BOOST_CHECK(points.size() == 5);
    double x = cos(M_PI*0.25);
    double y = sin(M_PI*0.25);
    BOOST_CHECK(abs(points[0].x()-(-1+x*cos(0.1*M_PI)-y*sin(0.1*M_PI))) < 0.0000001);
    BOOST_CHECK(abs(points[0].y()-(x*sin(0.1*M_PI)+y*cos(0.1*M_PI))) < 0.0000001);
    BOOST_CHECK(points[0].z() == 0);
    x = cos(M_PI*0.25+laser_scan.angular_resolution);
    y = sin(M_PI*0.25+laser_scan.angular_resolution);
    BOOST_CHECK(abs(points[1].x()-(-1+x*cos(0.1*M_PI)-y*sin(0.1*M_PI))) < 0.0000001);
    BOOST_CHECK(abs(points[1].y()-(x*sin(0.1*M_PI)+y*cos(0.1*M_PI))) < 0.0000001);
    BOOST_CHECK(isnan(points[3].x()));
    BOOST_CHECK(isnan(points[3].y()));
    BOOST_CHECK(isnan(points[3].z()));

    //check skipping of invalid scan points  
    laser_scan.convertScanToPointCloud(points,trans);
    BOOST_CHECK(points.size() == 4);
    x = cos(M_PI*0.25);
    y = sin(M_PI*0.25);
    BOOST_CHECK(abs(points[0].x()-(-1+x*cos(0.1*M_PI)-y*sin(0.1*M_PI))) < 0.0000001);
    BOOST_CHECK(abs(points[0].y()-(x*sin(0.1*M_PI)+y*cos(0.1*M_PI))) < 0.0000001);
    BOOST_CHECK(points[0].z() == 0);
    x = cos(M_PI*0.25+laser_scan.angular_resolution);
    y = sin(M_PI*0.25+laser_scan.angular_resolution);
    BOOST_CHECK(abs(points[1].x()-(-1+x*cos(0.1*M_PI)-y*sin(0.1*M_PI))) < 0.0000001);
    BOOST_CHECK(abs(points[1].y()-(x*sin(0.1*M_PI)+y*cos(0.1*M_PI))) < 0.0000001);
    BOOST_CHECK(!isnan(points[3].x()));
    BOOST_CHECK(!isnan(points[3].y()));
    BOOST_CHECK(!isnan(points[3].z()));
}

BOOST_AUTO_TEST_CASE( pose_test )
{
    Eigen::Vector3d pos( 10, -1, 20.5 );
    Eigen::Quaterniond orientation( Eigen::AngleAxisd( 0.2, Eigen::Vector3d(0.5, 1.4, 0.1) ) );

    base::Pose p( pos, orientation ); 
    Eigen::Affine3d t( p.toTransform() );

    BOOST_CHECK( pos.isApprox( t.translation() ) );
    BOOST_CHECK( orientation.isApprox( Eigen::Quaterniond(t.rotation()), 0.01 ) );

    cout << Eigen::Quaterniond(t.rotation()).coeffs().transpose() << endl;
    cout << orientation.coeffs().transpose() << endl;
}


base::Angle rand_angle()
{
    return base::Angle::fromRad(((rand() / (RAND_MAX + 1.0))-0.5) * M_PI);
}

BOOST_AUTO_TEST_CASE( angle_test )
{
    using namespace base;

    Angle a = Angle::fromDeg( 90 );
    BOOST_CHECK( a.isApprox( Angle::fromRad( M_PI/2.0 )) );
    BOOST_CHECK( a.isApprox( Angle::fromDeg( 90 + 720 )) );
    BOOST_CHECK( a.isApprox( Angle::fromDeg( 90 ) + Angle::fromDeg( 720 )) );
    BOOST_CHECK( a.isApprox( Angle::fromDeg( 90 - 720 )) );
    BOOST_CHECK( a.isApprox( Angle::fromDeg( 90 ) - Angle::fromDeg( 720 )) );
    BOOST_CHECK( (2*a).isApprox( Angle::fromDeg( 180 )) );
    BOOST_CHECK_CLOSE( (Angle::fromDeg(45)+Angle::fromDeg(-45)).getRad(), Angle::fromRad(0).getRad(), 1e-3 );
    std::cout << a << std::endl;

    BOOST_CHECK( a.isInRange(Angle::fromDeg(89),Angle::fromDeg(91)));
    BOOST_CHECK( a.isInRange(Angle::fromDeg(89),Angle::fromDeg(180)));
    BOOST_CHECK( a.isInRange(Angle::fromDeg(89),Angle::fromDeg(190)));
    BOOST_CHECK( a.isInRange(Angle::fromDeg(89),Angle::fromDeg(360)));
    BOOST_CHECK( !a.isInRange(Angle::fromDeg(91),Angle::fromDeg(89)));

    a = Angle::fromDeg( 190 );
    BOOST_CHECK( a.isInRange(Angle::fromDeg(170),Angle::fromDeg(200)));
    BOOST_CHECK( a.isInRange(Angle::fromDeg(185),Angle::fromDeg(200)));
    BOOST_CHECK( a.isInRange(Angle::fromDeg(170),Angle::fromDeg(160)));
    BOOST_CHECK( a.isInRange(Angle::fromDeg(170),Angle::fromDeg(350)));
    BOOST_CHECK( !a.isInRange(Angle::fromDeg(200),Angle::fromDeg(380)));
}

BOOST_AUTO_TEST_CASE( yaw_test )
{
    using namespace base;

    for(int i=0;i<10;i++)
    {
	Angle roll = rand_angle();
	Angle pitch = rand_angle();
	Angle yaw = rand_angle();
	Eigen::Quaterniond pitchroll = 
	    Eigen::AngleAxisd( pitch.getRad(), Eigen::Vector3d::UnitY() ) *
	    Eigen::AngleAxisd( roll.getRad(), Eigen::Vector3d::UnitX() );

	Eigen::Quaterniond rot =
	    Eigen::AngleAxisd( yaw.getRad(), Eigen::Vector3d::UnitZ() ) *
	    pitchroll;

	BOOST_CHECK_CLOSE( yaw.getRad(), Angle::fromRad(getYaw( rot )).getRad(), 1e-3 );

	rot = base::removeYaw( rot );
	BOOST_CHECK( rot.isApprox( pitchroll ) );
    }
}


BOOST_AUTO_TEST_CASE( angle_between_vectors )
{
    using base::Angle;
    using base::Vector3d;
    BOOST_CHECK_SMALL(Angle::vectorToVector(Vector3d(2, 0, 0), Vector3d(3, 0, 0)).getRad(), 1e-3);
    BOOST_CHECK_SMALL(Angle::vectorToVector(Vector3d(2, 0, 0), Vector3d(3, 0, 0), Vector3d::UnitZ()).getRad(), 1e-3);

    BOOST_CHECK_SMALL(M_PI/2 - Angle::vectorToVector(Vector3d(2, 0, 0), Vector3d(0, 3, 0)).getRad(), 1e-3);
    BOOST_CHECK_SMALL(M_PI/2 - Angle::vectorToVector(Vector3d(2, 0, 0), Vector3d(0, 3, 0), Vector3d::UnitZ()).getRad(), 1e-3);
    BOOST_CHECK_SMALL(-M_PI/2 - Angle::vectorToVector(Vector3d(2, 0, 0), Vector3d(0, 3, 0), -Vector3d::UnitZ()).getRad(), 1e-3);

    BOOST_CHECK_SMALL(M_PI/2 - Angle::vectorToVector(Vector3d(0, 2, 0), Vector3d(3, 0, 0)).getRad(), 1e-3);
    BOOST_CHECK_SMALL(-M_PI/2 - Angle::vectorToVector(Vector3d(0, 2, 0), Vector3d(3, 0, 0), Vector3d::UnitZ()).getRad(), 1e-3);
    BOOST_CHECK_SMALL(M_PI/2 - Angle::vectorToVector(Vector3d(0, 2, 0), Vector3d(3, 0, 0), -Vector3d::UnitZ()).getRad(), 1e-3);

    BOOST_CHECK_SMALL(M_PI - Angle::vectorToVector(Vector3d(2, 0, 0), Vector3d(-3, 0.001, 0)).getRad(), 1e-3);
    BOOST_CHECK_SMALL(M_PI - Angle::vectorToVector(Vector3d(2, 0, 0), Vector3d(-3, 0.001, 0), Vector3d::UnitZ()).getRad(), 1e-3);
    BOOST_CHECK_SMALL(-M_PI - Angle::vectorToVector(Vector3d(2, 0, 0), Vector3d(-3, 0.001, 0), -Vector3d::UnitZ()).getRad(), 1e-3);

    BOOST_CHECK_SMALL(M_PI - Angle::vectorToVector(Vector3d(2, 0, 0), Vector3d(-3, -0.001, 0)).getRad(), 1e-3);
    BOOST_CHECK_SMALL(-M_PI - Angle::vectorToVector(Vector3d(2, 0, 0), Vector3d(-3, -0.001, 0), Vector3d::UnitZ()).getRad(), 1e-3);
    BOOST_CHECK_SMALL(M_PI - Angle::vectorToVector(Vector3d(2, 0, 0), Vector3d(-3, -0.001, 0), -Vector3d::UnitZ()).getRad(), 1e-3);
}

BOOST_AUTO_TEST_CASE( logging_test )
{
        FILE* s = fopen("test.out", "w");
#ifdef BASE_LONG_NAMES

#ifdef WIN32
        BASE_LOG_CONFIGURE(INFO_P, s);
#else
        BASE_LOG_CONFIGURE(INFO, s);
#endif
        BASE_LOG_INFO("info-message")
#else

#ifdef WIN32
        LOG_CONFIGURE(INFO_P, s);
#else 
	LOG_CONFIGURE(INFO, s);
#endif

        LOG_INFO("info-message")
#endif

        std::string test("additional-argument");

        int number = 1000000;
        time_t start,stop;
        time(&start);
        for(int i = 0; i < number; i++)
        {
#ifdef BASE_LONG_NAMES
            BASE_LOG_FATAL("test fatal log %s", test.c_str())
#else
            LOG_FATAL("test fatal log %s", test.c_str())
#endif
        }
        time(&stop);
        double seconds = difftime(stop, start)/(number*1.0);
        printf("Estimated time per log msg %f seconds", seconds);
}

#include <base/float.h>

BOOST_AUTO_TEST_CASE( test_inf_nan )
{
    {
        float inf = base::infinity<float>();
        BOOST_REQUIRE( base::isInfinity(inf) );
        BOOST_REQUIRE( base::isInfinity(inf * 10) );
        BOOST_REQUIRE(inf == inf);
    }

    {
        double inf = base::infinity<double>();
        BOOST_REQUIRE( base::isInfinity(inf) );
        BOOST_REQUIRE( base::isInfinity(inf * 10) );
        BOOST_REQUIRE(inf == inf);
    }

    {
        float nan = base::unset<float>();
        BOOST_REQUIRE( base::isUnset(nan) );
        BOOST_REQUIRE( base::isUnset(nan * 10) );
        BOOST_REQUIRE(nan != nan);
    }

    {
        double nan = base::unset<double>();
        BOOST_REQUIRE( base::isUnset(nan) );
        BOOST_REQUIRE( base::isUnset(nan * 10) );
        BOOST_REQUIRE(nan != nan);
    }

    {
        float nan = base::unknown<float>();
        BOOST_REQUIRE( base::isUnknown(nan) );
        BOOST_REQUIRE( base::isUnknown(nan * 10) );
        BOOST_REQUIRE(nan != nan);
    }

    {
        double nan = base::unknown<double>();
        BOOST_REQUIRE( base::isUnknown(nan) );
        BOOST_REQUIRE( base::isUnknown(nan * 10) );
        BOOST_REQUIRE(nan != nan);
    }
}


BOOST_AUTO_TEST_CASE( frame_test )
{
    using namespace base::samples::frame;

    Frame frame;
    frame.init(200,300,8,MODE_GRAYSCALE);
    BOOST_CHECK(frame.getNumberOfBytes() == 200*300*1);
    BOOST_CHECK(frame.getPixelSize() == 1);
    BOOST_CHECK(frame.getPixelCount() == 200*300);
    BOOST_CHECK(frame.getChannelCount() == 1);
    BOOST_CHECK(frame.isCompressed() == false);
    BOOST_CHECK(frame.getHeight() == 300);
    BOOST_CHECK(frame.getWidth() == 200);

    frame.init(200,300,9,MODE_GRAYSCALE);
    BOOST_CHECK(frame.getNumberOfBytes() == 200*300*2);
    BOOST_CHECK(frame.getPixelSize() == 2);
    BOOST_CHECK(frame.getPixelCount() == 200*300);
    BOOST_CHECK(frame.getChannelCount() == 1);
    BOOST_CHECK(frame.isCompressed() == false);
    BOOST_CHECK(frame.getHeight() == 300);
    BOOST_CHECK(frame.getWidth() == 200);

    frame.init(200,300,8,MODE_RGB);
    BOOST_CHECK(frame.getNumberOfBytes() == 200*300*3);
    BOOST_CHECK(frame.getPixelSize() == 3);
    BOOST_CHECK(frame.getPixelCount() == 200*300);
    BOOST_CHECK(frame.getChannelCount() == 3);
    BOOST_CHECK(frame.isCompressed() == false);
    BOOST_CHECK(frame.getHeight() == 300);
    BOOST_CHECK(frame.getWidth() == 200);

    frame.init(200,300,8,MODE_GRAYSCALE,-1,200*300*1);
    BOOST_CHECK(frame.getNumberOfBytes() == 200*300*1);
    BOOST_CHECK(frame.getPixelSize() == 1);
    BOOST_CHECK(frame.getPixelCount() == 200*300);
    BOOST_CHECK(frame.getChannelCount() == 1);
    BOOST_CHECK(frame.isCompressed() == false);
    BOOST_CHECK(frame.getHeight() == 300);
    BOOST_CHECK(frame.getWidth() == 200);

    frame.init(200,300,8,MODE_PJPG,-1,0.5*200*300);
    BOOST_CHECK(frame.getNumberOfBytes() == 0.5*200*300);
    BOOST_CHECK(frame.getPixelSize() == 1);
    BOOST_CHECK(frame.getPixelCount() == 200*300);
    BOOST_CHECK(frame.getChannelCount() == 1);
    BOOST_CHECK(frame.isCompressed() == true);
    BOOST_CHECK(frame.getHeight() == 300);
    BOOST_CHECK(frame.getWidth() == 200);

    BOOST_CHECK_THROW(frame.init(200,300,8,MODE_RGB,-1,0.5*200*300),std::runtime_error);

    frame.init(200,300,8,MODE_GRAYSCALE);
    Frame frame2(frame);
    BOOST_CHECK(frame2.getNumberOfBytes() == 200*300);
    BOOST_CHECK(frame2.getPixelSize() == 1);
    BOOST_CHECK(frame2.getPixelCount() == 200*300);
    BOOST_CHECK(frame2.getChannelCount() == 1);
    BOOST_CHECK(frame2.isCompressed() == false);
    BOOST_CHECK(frame2.getHeight() == 300);
    BOOST_CHECK(frame2.getWidth() == 200);
}

BOOST_AUTO_TEST_CASE( rbs_validity )
{
    base::samples::RigidBodyState rbs;
    rbs.invalidate();
    BOOST_CHECK(!rbs.hasValidPosition());
    BOOST_CHECK(rbs.hasValidPositionCovariance());
    BOOST_CHECK(!rbs.hasValidOrientation());
    BOOST_CHECK(rbs.hasValidOrientationCovariance());
    BOOST_CHECK(!rbs.hasValidAngularVelocity());
    BOOST_CHECK(rbs.hasValidAngularVelocityCovariance());

    rbs.invalidate();
    rbs.invalidatePositionCovariance();
    BOOST_CHECK(rbs.hasValidPosition());
    BOOST_CHECK(!rbs.hasValidPositionCovariance());
    BOOST_CHECK(!rbs.hasValidOrientation());
    BOOST_CHECK(rbs.hasValidOrientationCovariance());
    BOOST_CHECK(!rbs.hasValidAngularVelocity());
    BOOST_CHECK(rbs.hasValidAngularVelocityCovariance());

    rbs.invalidate();
    rbs.invalidateOrientationCovariance();
    BOOST_CHECK(!rbs.hasValidPosition());
    BOOST_CHECK(rbs.hasValidPositionCovariance());
    BOOST_CHECK(rbs.hasValidOrientation());
    BOOST_CHECK(!rbs.hasValidOrientationCovariance());
    BOOST_CHECK(!rbs.hasValidAngularVelocity());
    BOOST_CHECK(rbs.hasValidAngularVelocityCovariance());

    rbs.invalidate();
    rbs.invalidateAngularVelocityCovariance();
    BOOST_CHECK(!rbs.hasValidPosition());
    BOOST_CHECK(rbs.hasValidPositionCovariance());
    BOOST_CHECK(!rbs.hasValidOrientation());
    BOOST_CHECK(rbs.hasValidOrientationCovariance());
    BOOST_CHECK(rbs.hasValidAngularVelocity());
    BOOST_CHECK(!rbs.hasValidAngularVelocityCovariance());
}

