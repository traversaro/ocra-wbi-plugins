#ifndef STANDINGREACH_H
#define STANDINGREACH_H

#include <ocraWbiPlugins/ocraWbiModel.h>
#include "../sequenceTools.h"

#include "wocra/Tasks/wOcraTaskSequenceBase.h"
#include <wocra/Trajectory/wOcraGaussianProcessTrajectory.h>

#include <smlt/smltUtilities.hpp>

#include <yarp/os/Network.h>
#include <yarp/os/BufferedPort.h>
#include <yarp/os/Bottle.h>
#include <yarp/os/Time.h>

#include <math.h>



class StandingReach: public wocra::wOcraTaskSequenceBase
{
    public:
        StandingReach();
        ~StandingReach();
    protected:
        virtual void doInit(wocra::wOcraController& c, wocra::wOcraModel& m);
        virtual void doUpdate(double time, wocra::wOcraModel& state, void** args);
    private:
        ocraWbiModel* wbiModel;

        //CoM task
        wocra::wOcraCoMTaskManager*                    comTask;
        Eigen::Vector3d                                initialCoMPosition;

        //Right hand task
        bool useVarianceModulation;

        // wocra::wOcraVariableWeightsTaskManager* rightHandTask;
        wocra::wOcraSegCartesianTaskManager* rightHandTask;
        wocra::wOcraGaussianProcessTrajectory* rightHandTrajectory;
        Eigen::Vector3d rHandPosStart, rHandPosEnd, currentOptWaypoint;
        Eigen::VectorXd optVariables;

        double maxVariance;
        double resetTimeRight;

        Eigen::MatrixXd desiredPosVelAcc_rightHand;
        Eigen::VectorXd rightHandGoalPosition;
        Eigen::Vector3d rightHandGoalPosition_transformed, rightHandStartPosition_transformed;
        Eigen::VectorXd desiredVariance_rightHand, desiredWeights_rightHand;
        Eigen::Array3d varianceThresh;

        Eigen::VectorXd rightHandStaticWeight;



        //YARP
        yarp::os::Network yarp;
        yarp::os::BufferedPort<yarp::os::Bottle> optVarsPortOut;
        yarp::os::BufferedPort<yarp::os::Bottle> costPortOut;
        yarp::os::BufferedPort<yarp::os::Bottle> optVarsPortIn;
        yarp::os::BufferedPort<yarp::os::Bottle> optParamsPortOut;
        std::string optVarsPortOut_name, costPortOut_name, optVarsPortIn_name, optParamsPortOut_name;

        yarp::os::Port r_hand_port;
        yarp::os::Port r_hand_start_port;
        yarp::os::Port r_hand_target_port;
        yarp::os::Port r_hand_waypoint_port;

        Eigen::Vector3d gazeboTranslation;
        void connectYarpPorts();
        void sendFramePositionsToGazebo();
        void bottleEigenVector(yarp::os::Bottle& bottle, const Eigen::VectorXd& vecToBottle, const bool encapsulate=false);



        // weights
        Eigen::VectorXd mapVarianceToWeights(Eigen::VectorXd& variance);



        // control
        bool waitForSolver;
        bool waitForHomePosition;
        bool optimumFound;
        bool sequenceFinished;
        bool initTrigger;
        bool newOptVarsReceived;
        bool dataSent_AwaitReply;

        int testNumber;

        bool initializeStabilization;
        double stabilizationTime;
        bool deactivatingHandTask;


        bool useGoalCost;
        bool useTrackingCost;
        bool useEnergyCost;



        bool logTrajectoryData;

        std::string rootLogFilePathPrefix;

        std::ofstream   totalInstantaneousCostFile,
                        goalInstantaneousCostFile,
                        trackingInstantaneousCostFile,
                        energyInstantaneousCostFile,
                        realTrajectoryFile;


        Eigen::MatrixXd goalCostMat, trackingCostMat, energyCostMat;

        int costIterCounter;

        double energyCostScalingFactor;

        bool replayOptimalTrajectory;


        double bOptCovarianceScalingFactor;



        bool openLogFiles(const std::string testLogFilePathPrefix);
        bool closeLogFiles();
        void sendOptimizationParameters();
        void initializeTrajectory(double time);
        void executeTrajectory(double relativeTime,  wocra::wOcraModel& state);
        bool sendTestDataToSolver();
        double postProcessInstantaneousCosts();
        bool parseNewOptVarsBottle();
        bool isBackInHomePosition(const wocra::wOcraModel& state);
        bool attainedGoal(const wocra::wOcraModel& state);
        void calculateInstantaneousCost(const double time, const wocra::wOcraModel& state);
        double calculateGoalCost(const double time, const wocra::wOcraModel& state);
        double calculateTrackingCost(const double time, const wocra::wOcraModel& state);
        double calculateEnergyCost(const double time, const wocra::wOcraModel& state);
        bool returnToStablePosture(const double time, const wocra::wOcraModel& state);


};




#endif
