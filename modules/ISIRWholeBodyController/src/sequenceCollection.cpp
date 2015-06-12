#include <ISIRWholeBodyController/sequenceCollection.h>
#include <ISIRWholeBodyController/ocraWbiModel.h>


#include "wocra/Trajectory/wOcraMinimumJerkTrajectory.h"
#include "wocra/Trajectory/wOcraLinearInterpolationTrajectory.h"

#ifndef PI
#define PI 3.1415926
#endif

void getNominalPosture(wocra::wOcraModel &model, VectorXd &q);

// Sequence_FixedBaseMinimalTasks
void Sequence_FixedBaseMinimalTasks::doInit(wocra::wOcraController& ctrl, wocra::wOcraModel& model)
{
    Eigen::VectorXd q_init = model.getJointPositions();
    std::cout << "q init: " << q_init << std::endl;
    tmFull = new wocra::wOcraFullPostureTaskManager(ctrl, model, "fullPostureTask", ocra::FullState::INTERNAL, 20.0, 3.0, 1.0, q_init);
}

void Sequence_FixedBaseMinimalTasks::doUpdate(double time, wocra::wOcraModel& state, void** args)
{
}



// Sequence_FloatingBaseMinimalTasks
void Sequence_FloatingBaseMinimalTasks::doInit(wocra::wOcraController& ctrl, wocra::wOcraModel& model)
{
    // Initialise full posture task
    // Eigen::VectorXd q_full = Eigen::VectorXd::Zero(model.nbInternalDofs());
    Eigen::VectorXd q_full = model.getJointPositions();

    std::cout << "\n\n q_full = \n"<< q_full <<" \n\n";

    // q_full[model.getDofIndex("l_elbow")]      = PI/8.0;
    // q_full[model.getDofIndex("r_elbow")]      = PI/8.0;
    // q_full[model.getDofIndex("l_knee")]             = -0.05;
    // q_full[model.getDofIndex("r_knee")]             = -0.05;
    // q_full[model.getDofIndex("l_ankle_pitch")]      = -0.05;
    // q_full[model.getDofIndex("r_ankle_pitch")]      = -0.05;
    // q_full[model.getDofIndex("l_shoulder_roll")]    = PI/8.0;
    // q_full[model.getDofIndex("r_shoulder_roll")]    = PI/8.0;

    std::cout << "\n\n q_full = \n"<< q_full <<" \n\n";

    std::cout << "\n\n tmFull \n\n";
    taskManagers["tmFull"] = new wocra::wOcraFullPostureTaskManager(ctrl, model, "fullPostureTask", ocra::FullState::INTERNAL, 10.0, 2*sqrt(10.0), 0.0001, q_full);



/* This part needs to be fixed... Not sure why but the orientation
    // Initialise waist pose

    // std::cout << "\n\n tmSegPoseWaist \n\n";
    // Eigen::Displacementd desiredWaistPose = Eigen::Displacementd(0.0,0.0,0.59,1.0,0.0,0.0,0.0);
    // taskManagers["tmSegPoseWaist"] = new wocra::wOcraSegPoseTaskManager(ctrl, model, "waistPoseTask", "root_link", ocra::XYZ, 36.0, 2*sqrt(36.0), 1.0, desiredWaistPose);
    */
    Eigen::Vector3d desiredWaistPosition, XYZdisp;
    desiredWaistPosition = model.getSegmentPosition(model.getSegmentIndex("root_link")).getTranslation();



    // XYZdisp << -0.03, 0.0, 0.0;
    // desiredWaistPosition = desiredWaistPosition + XYZdisp;
    taskManagers["tmSegPoseWaist"] = new wocra::wOcraSegCartesianTaskManager(ctrl, model, "waistPoseTask", "root_link", ocra::XYZ, 36.0, 2*sqrt(36.0), 1.0, desiredWaistPosition);


    // Initialise partial posture task


    // Eigen::VectorXi sdofs(3);
    // sdofs << model.getDofIndex("torso_pitch"), model.getDofIndex("torso_roll"), model.getDofIndex("torso_yaw");
    // Eigen::VectorXd zero = Eigen::VectorXd::Zero(3);
    // std::cout << "\n\n tmPartialBack \n\n";
    // taskManagers["tmPartialBack"] = new wocra::wOcraPartialPostureTaskManager(ctrl, model, "partialPostureBackTask", ocra::FullState::INTERNAL, sdofs, 16.0, 2*sqrt(16.0), 0.001, zero);

    double mu_sys = 0.5;
    double margin = 0.0;
    double sqrt2on2 = sqrt(2.0)/2.0;

    Eigen::Rotation3d rotLZdown = Eigen::Rotation3d(sqrt2on2, 0.0, 0.0, sqrt2on2) * Eigen::Rotation3d(0.0, 1.0, 0.0, 0.0);
    Eigen::Rotation3d rotRZdown = Eigen::Rotation3d(-sqrt2on2, 0.0, 0.0, -sqrt2on2) * Eigen::Rotation3d(0.0, 1.0, 0.0, 0.0);


    // Initialise left foot contacts
    Eigen::Displacementd LFContacts[4];
    LFContacts[0] = Eigen::Displacementd(Eigen::Vector3d(-0.02,-0.02,0.0), rotLZdown);
    LFContacts[1] = Eigen::Displacementd(Eigen::Vector3d( 0.06,-0.02,0.0), rotLZdown);
    LFContacts[2] = Eigen::Displacementd(Eigen::Vector3d(-0.02, 0.02,0.0), rotLZdown);
    LFContacts[3] = Eigen::Displacementd(Eigen::Vector3d( 0.06, 0.02,0.0), rotLZdown);
    std::cout << "\n\n tmFootContactLeft \n\n";
    taskManagers["tmFootContactLeft"] = new wocra::wOcraContactSetTaskManager(ctrl, model, "leftFootContactTask", "l_sole", LFContacts, 4, mu_sys, margin);

    // Initailise right foot contacts
    Eigen::Displacementd RFContacts[4];
    RFContacts[0] = Eigen::Displacementd(Eigen::Vector3d(-0.02,-0.02,0.0), rotRZdown);
    RFContacts[1] = Eigen::Displacementd(Eigen::Vector3d( 0.06,-0.02,0.0), rotRZdown);
    RFContacts[2] = Eigen::Displacementd(Eigen::Vector3d(-0.02, 0.02,0.0), rotRZdown);
    RFContacts[3] = Eigen::Displacementd(Eigen::Vector3d( 0.06, 0.02,0.0), rotRZdown);
    std::cout << "\n\n tmFootContactRight \n\n";
    taskManagers["tmFootContactRight"] = new wocra::wOcraContactSetTaskManager(ctrl, model, "RightFootContactTask", "r_sole", RFContacts, 4, mu_sys, margin);
    std::cout << "\n\n Fully Initialized \n\n";

}

void Sequence_FloatingBaseMinimalTasks::doUpdate(double time, wocra::wOcraModel& state, void** args)
{
}



// Sequence_InitialPoseHold
void Sequence_InitialPoseHold::doInit(wocra::wOcraController& ctrl, wocra::wOcraModel& model)
{
    Eigen::VectorXd q_init = model.getJointPositions();
    std::cout << "q init: " << q_init << std::endl;
    tmFull = new wocra::wOcraFullPostureTaskManager(ctrl, model, "fullPostureTask", ocra::FullState::INTERNAL, 20.0, 3.0, 1.0, q_init);
}

void Sequence_InitialPoseHold::doUpdate(double time, wocra::wOcraModel& state, void** args)
{
}

// Sequence_NominalPoseHold
void Sequence_NominalPose::doInit(wocra::wOcraController& ctrl, wocra::wOcraModel& model)
{
    tFinal = 5.0;
    tInitial = 0.0;
    t_pich_index = model.getDofIndex("torso_pitch");
    q_init = model.getJointPositions();
    nominal_q = Eigen::VectorXd::Zero(model.nbInternalDofs());
    getNominalPosture(model, nominal_q);

    tmFull = new wocra::wOcraFullPostureTaskManager(ctrl, model, "fullPostureTask", ocra::FullState::INTERNAL, 20.0, 5.0, 1, q_init);
}

void Sequence_NominalPose::doUpdate(double time, wocra::wOcraModel& state, void** args)
{
    Eigen::VectorXd q_current;
    if (time <= tFinal){
        q_current = (time - tInitial)/(tFinal-tInitial) * (nominal_q - q_init) + q_init;
        tmFull->setPosture(q_current);
    }
    else{
        q_current = nominal_q;
        tmFull->setPosture(q_current);
    }

    Eigen::VectorXd taskError = tmFull->getTaskError();
    //std::cout << "Time: " << time << "[s], Posture error total: " << tmFull->getTaskErrorNorm() << std::endl;
    // std::cout << "Time: " << time << "[s], Torso Pitch error total: " << taskError(t_pich_index) << std::endl;

    /*

    for (int i = 0; i < taskError.size(); i++)
        //std::cout << "Joint " << i << ", Error : " << taskError(i) << std::endl;
        std::cout << "Joint " << i << ", Error : " << q_current(i) << std::endl;

    std::cout << std::endl << std::endl;
    */
}

// Sequence_LeftHandReach
void Sequence_LeftHandReach::doInit(wocra::wOcraController& ctrl, wocra::wOcraModel& model)
{
    ocraWbiModel& wbiModel = dynamic_cast<ocraWbiModel&>(model);
    // Full posture task
    Eigen::VectorXd nominal_q = Eigen::VectorXd::Zero(model.nbInternalDofs());
    getNominalPosture(model, nominal_q);

    tmFull = new wocra::wOcraFullPostureTaskManager(ctrl, model, "fullPostureTask", ocra::FullState::INTERNAL, 20.0, 3.0, 0.001, nominal_q);

    // Left hand cartesian task
    Eigen::Vector3d posLHandDes(-0.3, -0.1, 0.3);
    tmSegCartHandLeft = new wocra::wOcraSegCartesianTaskManager(ctrl, model, "leftHandCartesianTask", "l_hand", ocra::XYZ, 10.0, 3.0, 100.0, posLHandDes);
}

void Sequence_LeftHandReach::doUpdate(double time, wocra::wOcraModel& state, void** args)
{
}


// Sequence_LeftRightHandReach
void Sequence_LeftRightHandReach::doInit(wocra::wOcraController& ctrl, wocra::wOcraModel& model)
{
    ocraWbiModel& wbiModel = dynamic_cast<ocraWbiModel&>(model);
    // Full posture task
    Eigen::VectorXd nominal_q = Eigen::VectorXd::Zero(model.nbInternalDofs());
    getNominalPosture(model, nominal_q);

    tmFull = new wocra::wOcraFullPostureTaskManager(ctrl, model, "fullPostureTask", ocra::FullState::INTERNAL, 20.0, 3.0, 0.01, nominal_q);

    // Partial (torso) posture task

    Eigen::VectorXi torso_indices(3);
    Eigen::VectorXd torsoTaskPosDes(3);
    torso_indices << wbiModel.getDofIndex("torso_pitch"), wbiModel.getDofIndex("torso_roll"), wbiModel.getDofIndex("torso_yaw");
    torsoTaskPosDes << M_PI / 18, 0, 0;
    tmPartialTorso = new wocra::wOcraPartialPostureTaskManager(ctrl, model, "partialPostureTorsoTask", ocra::FullState::INTERNAL, torso_indices, 10.0, 3.0, 5.0, torsoTaskPosDes);


    // CoM Task
    Eigen::Vector3d posCoM = model.getCoMPosition();
    tmCoM = new wocra::wOcraCoMTaskManager(ctrl, model, "CoMTask", 10.0, 3.0, 10.0, posCoM);

    // Left hand cartesian task
    Eigen::Vector3d posLHandDes(-0.3, -0.2, 0.15);
    tmSegCartHandLeft = new wocra::wOcraSegCartesianTaskManager(ctrl, model, "leftHandCartesianTask", "l_hand", ocra::XYZ, 10.0, 3.0, 100.0, posLHandDes);


    // Right hand cartesian task
    Eigen::Vector3d posRHandDes(-0.15, 0.2, -0.1);
    tmSegCartHandRight = new wocra::wOcraSegCartesianTaskManager(ctrl, model, "rightHandCartesianTask", "r_hand", ocra::XYZ, 10.0, 3.0, 100.0, posRHandDes);


}

void Sequence_LeftRightHandReach::doUpdate(double time, wocra::wOcraModel& state, void** args)
{
}

void Sequence_CartesianTest::doInit(wocra::wOcraController& ctrl, wocra::wOcraModel& model)
{
    ocraWbiModel& wbiModel = dynamic_cast<ocraWbiModel&>(model);

    // Task Coeffs
    double Kp = 10.0;
    double Kd = 2.0 * sqrt(Kp);
    double wFullPosture = 0.001;
    double wPartialPosture = 0.01;
    double wLeftHandTask = 1.0;

    // Full posture task
    Eigen::VectorXd nominal_q = Eigen::VectorXd::Zero(model.nbInternalDofs());
    getNominalPosture(model, nominal_q);
    tmFull = new wocra::wOcraFullPostureTaskManager(ctrl, model, "fullPostureTask", ocra::FullState::INTERNAL, Kp, Kd, wFullPosture, nominal_q);

    // Partial (torso) posture task
    Eigen::VectorXi torso_indices(3);
    Eigen::VectorXd torsoTaskPosDes(3);
    torso_indices << wbiModel.getDofIndex("torso_pitch"), wbiModel.getDofIndex("torso_roll"), wbiModel.getDofIndex("torso_yaw");
    torsoTaskPosDes << M_PI / 18, 0, 0;
    tmPartialTorso = new wocra::wOcraPartialPostureTaskManager(ctrl, model, "partialPostureTorsoTask", ocra::FullState::INTERNAL, torso_indices, Kp, Kd, wPartialPosture, torsoTaskPosDes);

    lHandIndex = model.getSegmentIndex("l_hand");

    Eigen::Vector3d YZ_disp;
    YZ_disp << 0.0, 0.2, 0.2;


    Eigen::Displacementd startingDispd  = model.getSegmentPosition(lHandIndex);

    // start and end positions
    Eigen::Vector3d startingPos = startingDispd.getTranslation();
    desiredPos  = startingPos+ YZ_disp;


    // Left hand cartesian task
    tmLeftHandCart = new wocra::wOcraSegCartesianTaskManager(ctrl, model, "leftHandCartesianTask", "l_hand", ocra::XYZ, Kp, Kd, wLeftHandTask, desiredPos);

}

void Sequence_CartesianTest::doUpdate(double time, wocra::wOcraModel& state, void** args)
{
    // tmLeftHandCart->setPosition(desiredPos);
    std::cout << "\n---\nDesired position: " << desiredPos.transpose() << std::endl;
    std::cout << "Current pose: " << state.getSegmentPosition(lHandIndex).getTranslation().transpose()<< std::endl;
    std::cout << "Error: " << tmLeftHandCart->getTaskError().transpose() << "\n" << std::endl;
}











void Sequence_PoseTest::doInit(wocra::wOcraController& ctrl, wocra::wOcraModel& model)
{
    ocraWbiModel& wbiModel = dynamic_cast<ocraWbiModel&>(model);

    // Task Coeffs
    double Kp = 10.0;
    double Kd = 2.0 * sqrt(Kp);
    double wFullPosture = 0.001;
    double wPartialPosture = 0.01;
    double wLeftHandTask = 1.0;

    // Full posture task
    Eigen::VectorXd nominal_q = Eigen::VectorXd::Zero(model.nbInternalDofs());
    getNominalPosture(model, nominal_q);
    tmFull = new wocra::wOcraFullPostureTaskManager(ctrl, model, "fullPostureTask", ocra::FullState::INTERNAL, Kp, Kd, wFullPosture, nominal_q);

    // Partial (torso) posture task
    Eigen::VectorXi torso_indices(3);
    Eigen::VectorXd torsoTaskPosDes(3);
    torso_indices << wbiModel.getDofIndex("torso_pitch"), wbiModel.getDofIndex("torso_roll"), wbiModel.getDofIndex("torso_yaw");
    torsoTaskPosDes << M_PI / 18, 0, 0;
    tmPartialTorso = new wocra::wOcraPartialPostureTaskManager(ctrl, model, "partialPostureTorsoTask", ocra::FullState::INTERNAL, torso_indices, Kp, Kd, wPartialPosture, torsoTaskPosDes);

    lHandIndex = model.getSegmentIndex("l_hand");

    Eigen::Vector3d YZ_disp;
    YZ_disp << 0.0, 0.2, 0.2;


    Eigen::Displacementd startingDispd  = model.getSegmentPosition(lHandIndex);
    std::cout << startingDispd << std::endl;
    // start and end positions
    Eigen::Vector3d desiredPos = startingDispd.getTranslation() + YZ_disp;
    // Eigen::Rotation3d desiredOrientation = Eigen::Rotation3d(1.0, 0.0, 0.0, 0.0);
    Eigen::Rotation3d desiredOrientation = startingDispd.getRotation();

    endingDispd    = Eigen::Displacementd(desiredPos, desiredOrientation);//, startingDispd.getRotation());//.inverse());

    // endingDispd    = startingDispd;
    std::cout << endingDispd << std::endl;
    // Left hand cartesian task
    tmLeftHandPose      = new wocra::wOcraSegPoseTaskManager(ctrl, model, "leftHandPoseTask", "l_hand", ocra::XYZ, Kp, Kd, wLeftHandTask, endingDispd);
}

void Sequence_PoseTest::doUpdate(double time, wocra::wOcraModel& state, void** args)
{
    // tmLeftHandCart->setPosition(desiredPos);
    std::cout << "\n---\nDesired pose: " << endingDispd << std::endl;
    std::cout << "Current pose: " << state.getSegmentPosition(lHandIndex)<< std::endl;
    std::cout << "Error: " << tmLeftHandPose->getTaskError().transpose() << "\n" << std::endl;
    // tmLeftHandPose->printTaskErrors();
}















void Sequence_OrientationTest::doInit(wocra::wOcraController& ctrl, wocra::wOcraModel& model)
{
    ocraWbiModel& wbiModel = dynamic_cast<ocraWbiModel&>(model);

    // Task Coeffs
    double Kp = 10.0;
    double Kd = 2.0 * sqrt(Kp);
    double wFullPosture = 0.001;
    double wPartialPosture = 0.01;
    double wLeftHandTask = 1.0;

    // Full posture task
    Eigen::VectorXd nominal_q = Eigen::VectorXd::Zero(model.nbInternalDofs());
    getNominalPosture(model, nominal_q);
    tmFull = new wocra::wOcraFullPostureTaskManager(ctrl, model, "fullPostureTask", ocra::FullState::INTERNAL, Kp, Kd, wFullPosture, nominal_q);

    // Partial (torso) posture task
    Eigen::VectorXi torso_indices(3);
    Eigen::VectorXd torsoTaskPosDes(3);
    torso_indices << wbiModel.getDofIndex("torso_pitch"), wbiModel.getDofIndex("torso_roll"), wbiModel.getDofIndex("torso_yaw");
    torsoTaskPosDes << M_PI / 18, 0, 0;
    tmPartialTorso = new wocra::wOcraPartialPostureTaskManager(ctrl, model, "partialPostureTorsoTask", ocra::FullState::INTERNAL, torso_indices, Kp, Kd, wPartialPosture, torsoTaskPosDes);

    lHandIndex = model.getSegmentIndex("l_hand");

    Eigen::Displacementd startingDispd  = model.getSegmentPosition(lHandIndex);

    // start and end rotations
    startingRotd      = startingDispd.getRotation();
    endingRotd        = Eigen::Rotation3d(0.105135,   0.0828095,   0.253438,    -0.958049);// palm down //startingRotd.inverse();


    // Left hand orientation task
    tmLeftHandOrient    = new wocra::wOcraSegOrientationTaskManager(ctrl, model, "leftHandOrientationTask", "l_hand", Kp, Kd, wLeftHandTask, endingRotd);
}

void Sequence_OrientationTest::doUpdate(double time, wocra::wOcraModel& state, void** args)
{
    std::cout << "\n---\nStarting orientation: " << startingRotd << std::endl;
    std::cout << "Desired orientation: " << endingRotd << std::endl;
    std::cout << "Current orientation: " << state.getSegmentPosition(lHandIndex).getRotation()<< std::endl;
    std::cout << "Error: " << tmLeftHandOrient->getTaskError().transpose() << "\n" << std::endl;
    // tmLeftHandOrient->printTaskErrors();
}











// Sequence_TrajectoryTrackingTest
void Sequence_TrajectoryTrackingTest::doInit(wocra::wOcraController& ctrl, wocra::wOcraModel& model)
{
    ocraWbiModel& wbiModel = dynamic_cast<ocraWbiModel&>(model);

    // Task Coeffs
    double Kp = 10.0;
    double Kd = 2.0 * sqrt(Kp);

    double Kp_hand = 40.0;
    double Kd_hand = 8.0 ;//* sqrt(Kp_hand);
    double wFullPosture = 0.0001;
    double wPartialPosture = 0.1;
    double wLeftHandTask = 1.0;

    // Full posture task
    Eigen::VectorXd nominal_q = Eigen::VectorXd::Zero(model.nbInternalDofs());
    getNominalPosture(model, nominal_q);
    tmFull = new wocra::wOcraFullPostureTaskManager(ctrl, model, "fullPostureTask", ocra::FullState::INTERNAL, Kp, Kd, wFullPosture, nominal_q);

    // Partial (torso) posture task
    Eigen::VectorXi torso_indices(3);
    Eigen::VectorXd torsoTaskPosDes(3);
    torso_indices << wbiModel.getDofIndex("torso_pitch"), wbiModel.getDofIndex("torso_roll"), wbiModel.getDofIndex("torso_yaw");
    torsoTaskPosDes << 0, -10.0*(M_PI / 180.0), 40.0*(M_PI / 180.0);
    // torsoTaskPosDes << 0.0, 0.0, 0.0;
    tmPartialTorso = new wocra::wOcraPartialPostureTaskManager(ctrl, model, "partialPostureTorsoTask", ocra::FullState::INTERNAL, torso_indices, 6., 2.0 * sqrt(6.), wPartialPosture, torsoTaskPosDes);

    // Right hand cartesian task
    Eigen::Vector3d posRHandDesDelta(0.1, 0.08, 0.15);

    Eigen::Vector3d posRHandDes = model.getSegmentPosition(model.getSegmentIndex("r_hand")).getTranslation();
    posRHandDes = posRHandDes + posRHandDesDelta;


    tmSegCartHandRight = new wocra::wOcraSegCartesianTaskManager(ctrl, model, "rightHandCartesianTask", "r_hand", ocra::XYZ, Kp_hand, Kd_hand, 1.0, posRHandDes);

    /**
    * Left hand task. Pick one of these booleans to test the different constructors.
    */
    //*************** Type of Trajectory ******************//
    bool isLinInterp = false;
    bool isMinJerk = true;
    //*****************************************************//

    //***************** Type of Reference ******************//
    isDisplacementd         = false;
    isRotation3d            = false;
    isCartesion             = false;
    isCartesionWaypoints    = true;
    //*****************************************************//
    int boolSum = isCartesion + isCartesionWaypoints + isDisplacementd + isRotation3d;
    if (boolSum>1){std::cout << "\nYou picked too many reference types." << std::endl;}
    else if (boolSum<1){std::cout << "\nYou picked too few reference types." << std::endl;}

    lHandIndex = model.getSegmentIndex("l_hand");

    Eigen::Vector3d YZ_disp;
    YZ_disp << -0.15, 0.1, 0.2;

    // start and end displacements & rotations
    Eigen::Displacementd startingDispd  = model.getSegmentPosition(lHandIndex);
    Eigen::Rotation3d startingRotd      = startingDispd.getRotation();

    endingRotd  = Eigen::Rotation3d(0.105135,   0.0828095,   0.253438,    -0.958049);
    endingDispd = Eigen::Displacementd(startingDispd.getTranslation() + YZ_disp, endingRotd);

    // start and end positions
    Eigen::Vector3d startingPos = startingDispd.getTranslation();
    desiredPos  = startingPos + YZ_disp;

    // multiple position waypoints
    Eigen::MatrixXd waypoints(3,5);
    Eigen::MatrixXd squareDisplacement(3,5);
    waypoints << startingPos, startingPos, startingPos, startingPos, startingPos;
    squareDisplacement << 0.0, 0.0, 0.0, 0.0, 0.0,
                          0.0, 0.2, 0.2, 0.0, 0.0,
                          0.0, 0.0, 0.2, 0.2, 0.0;
    waypoints += squareDisplacement;

    if (isLinInterp)
    {
        /**
        * Linear interpolation trajectory constructor tests:
        */
        if      (isDisplacementd)       {leftHandTrajectory = new wocra::wOcraLinearInterpolationTrajectory(startingDispd, endingDispd);}
        else if (isRotation3d)          {leftHandTrajectory = new wocra::wOcraLinearInterpolationTrajectory(startingRotd, endingRotd);}
        else if (isCartesion)           {leftHandTrajectory = new wocra::wOcraLinearInterpolationTrajectory(startingPos, desiredPos);}
        else if (isCartesionWaypoints)  {leftHandTrajectory = new wocra::wOcraLinearInterpolationTrajectory(waypoints);}
        else                            {std::cout << "\nGotta pick a reference type motherfucker!" << std::endl;}
    }
    else if (isMinJerk)
    {
        /**
        * Minimum jerk trajectory constructor tests:
        */
        if      (isDisplacementd)       {leftHandTrajectory = new wocra::wOcraMinimumJerkTrajectory(startingDispd, endingDispd);}
        else if (isRotation3d)          {leftHandTrajectory = new wocra::wOcraMinimumJerkTrajectory(startingRotd, endingRotd);}
        else if (isCartesion)           {leftHandTrajectory = new wocra::wOcraMinimumJerkTrajectory(startingPos, desiredPos);}
        else if (isCartesionWaypoints)  {leftHandTrajectory = new wocra::wOcraMinimumJerkTrajectory(waypoints);}
        else                            {std::cout << "\nGotta pick a reference type motherfucker!" << std::endl;}
    }
    else{std::cout << "\nGotta pick a trajectory type motherfucker!" << std::endl;}


    leftHandTrajectory->generateTrajectory(3.0); // set a 4 second duration

    if      (isDisplacementd)      {tmLeftHandPose      = new wocra::wOcraSegPoseTaskManager(ctrl, model, "leftHandPoseTask", "l_hand", ocra::XYZ, Kp_hand, Kd_hand, wLeftHandTask, startingDispd);}
    else if (isRotation3d)         {tmLeftHandOrient    = new wocra::wOcraSegOrientationTaskManager(ctrl, model, "leftHandOrientationTask", "l_hand", Kp_hand, Kd_hand, wLeftHandTask, startingRotd);}
    else if (isCartesion)          {tmLeftHandCart      = new wocra::wOcraSegCartesianTaskManager(ctrl, model, "leftHandCartesianTask", "l_hand", ocra::XYZ, Kp_hand, Kd_hand, wLeftHandTask, startingPos);}
    else if (isCartesionWaypoints) {tmLeftHandCart      = new wocra::wOcraSegCartesianTaskManager(ctrl, model, "leftHandCartesianTask", "l_hand", ocra::XYZ, Kp_hand, Kd_hand, wLeftHandTask, startingPos);}


// tmSegCartHandRight = new wocra::wOcraSegCartesianTaskManager(ctrl, model, "rightHandCartesianTask", "r_hand", ocra::XYZ, Kp_hand, Kd_hand, wLeftHandTask, posRHandDes);
}



void Sequence_TrajectoryTrackingTest::doUpdate(double time, wocra::wOcraModel& state, void** args)
{
    if (isDisplacementd)
    {
        Eigen::Displacementd desiredPose;
        Eigen::Twistd desiredVelocity;
        Eigen::Twistd desiredAcceleration;
        leftHandTrajectory->getDesiredValues(time, desiredPose, desiredVelocity, desiredAcceleration);

        tmLeftHandPose->setState(desiredPose, desiredVelocity, desiredAcceleration);

        // tmLeftHandPose->setState(desiredPose, desiredVelocity, Eigen::Twistd::Zero());

        std::cout << "\nFinal desired pose: " << endingDispd << std::endl;
        std::cout << "Desired pose: " << desiredPose << std::endl;
        std::cout << "Desired vel: " << desiredVelocity.transpose() << std::endl;
        std::cout << "Desired acc: " << desiredAcceleration.transpose() << std::endl;
        std::cout << "Current pose: " << state.getSegmentPosition(lHandIndex) << std::endl;
        std::cout << "Error: " << tmLeftHandPose->getTaskError().transpose() << "   norm: " << tmLeftHandPose->getTaskErrorNorm() << std::endl;

    }
    else if (isRotation3d)
    {
        Eigen::Rotation3d desiredOrientation;
        leftHandTrajectory->getDesiredValues(time, desiredOrientation);
        tmLeftHandOrient->setOrientation(desiredOrientation);

        std::cout << "\nFinal desired orientation: " << endingRotd << std::endl;
        std::cout << "Desired orientation: " << desiredOrientation << std::endl;
        std::cout << "Current orientation: " << state.getSegmentPosition(lHandIndex).getRotation() << std::endl;
        std::cout << "Error: " << tmLeftHandOrient->getTaskError().transpose() << "   norm: " << tmLeftHandOrient->getTaskErrorNorm() << std::endl;
    }
    else if (isCartesion || isCartesionWaypoints)
    {
        Eigen::MatrixXd desiredPosVelAcc = leftHandTrajectory->getDesiredValues(time);
        // Eigen::MatrixXd H_adj = state.getSegmentPosition(lHandIndex).getRotation().inverse().adjoint();
        // H_adj*
        tmLeftHandCart->setState(desiredPosVelAcc.col(0));//,  desiredPosVelAcc.col(1), desiredPosVelAcc.col(2));

        // if(isCartesion){std::cout << "\nFinal desired position: " << desiredPos.transpose() << std::endl;}
        // std::cout << "\nDesired position: " << desiredPosVelAcc.col(0).transpose() << std::endl;
        // std::cout << "Current position: " << state.getSegmentPosition(lHandIndex).getTranslation().transpose()<< std::endl;
        // std::cout << "Error: " << tmLeftHandCart->getTaskError().transpose() << "   norm: " << tmLeftHandCart->getTaskErrorNorm() << std::endl;
    }



}


void Sequence_FloatingBaseEstimationTests::doInit(wocra::wOcraController& ctrl, wocra::wOcraModel& model)
{
    Eigen::VectorXd q_init = model.getJointPositions();
    std::cout << "q init: " << q_init << std::endl;
    tmFull = new wocra::wOcraFullPostureTaskManager(ctrl, model, "fullPostureTask", ocra::FullState::INTERNAL, 20.0, 3.0, 1.0, q_init);
}

void Sequence_FloatingBaseEstimationTests::doUpdate(double time, wocra::wOcraModel& state, void** args)
{
}





// Sequence_LeftRightHandReach
void Sequence_JointTest::doInit(wocra::wOcraController& ctrl, wocra::wOcraModel& model)
{
    ocraWbiModel& wbiModel = dynamic_cast<ocraWbiModel&>(model);
    // Full posture task
    nDoF = model.nbInternalDofs();
    // Eigen::VectorXd nominal_q = Eigen::VectorXd::Zero(nDoF);
    // getNominalPosture(model, nominal_q);


    q_init = model.getJointPositions();

    q_des = q_init;

    jointMin = model.getJointLowerLimits();

    jointMax = model.getJointUpperLimits();

    tmFull = new wocra::wOcraFullPostureTaskManager(ctrl, model, "fullPostureTask", ocra::FullState::INTERNAL, 20.0, 2.0*sqrt(20), 1.0, q_init);
    for (int i=0; i<nDoF; i++){
        jointNames[i] = wbiModel.getJointName(i);
    }
    taskErr = 0.0;
    jIndex = 0;
    goToMin = true;
    goToMax = false;
    counter = 401;



}

void Sequence_JointTest::doUpdate(double time, wocra::wOcraModel& state, void** args)
{
    Eigen::VectorXd taskErrorVector = tmFull->getTaskError();
    // std::cout << taskErrorVector.transpose() << std::endl;
    taskErr = abs(taskErrorVector(jIndex));
    // std::cout << taskErr << std::endl;




    if (( counter>=400) && jIndex<nDoF){
        if ((goToMin == true) && (goToMax==false)){

            q_des(jIndex) = jointMin(jIndex);
            tmFull->setPosture(q_des);
            goToMin = false;
            goToMax = true;
            std::cout << "\nJoint: " << jointNames[jIndex] << "-> moving to lower limit, " << jointMin(jIndex) << std::endl;
        }

        else if ((goToMin == false )&& (goToMax==true)){
            q_des(jIndex) = jointMax(jIndex);
            tmFull->setPosture(q_des);
            goToMax = false;
            std::cout << "\nJoint: " << jointNames[jIndex] << "-> moving to upper limit, " << jointMax(jIndex) << std::endl;

        }
        else if ((goToMin == false) && (goToMax==false)){
            q_des = q_init;
            tmFull->setPosture(q_des);
            jIndex++;
            goToMin = true;

        }
        counter = 0;
    }

    counter ++ ;


}


void getNominalPosture(wocra::wOcraModel& model, VectorXd &q)
{
    q[model.getDofIndex("torso_pitch")] = M_PI / 18;
    q[model.getDofIndex("r_elbow")] = M_PI / 4;
    q[model.getDofIndex("l_elbow")] = M_PI / 4;
    q[model.getDofIndex("l_shoulder_roll")] = M_PI / 6;
    q[model.getDofIndex("r_shoulder_roll")] = M_PI / 6;
    q[model.getDofIndex("l_shoulder_pitch")] = -M_PI / 6;
    q[model.getDofIndex("r_shoulder_pitch")] = -M_PI / 6;
    q[model.getDofIndex("l_hip_pitch")] = M_PI / 8;
    q[model.getDofIndex("r_hip_pitch")] = M_PI / 8;
    q[model.getDofIndex("l_hip_roll")] = M_PI / 18;
    q[model.getDofIndex("r_hip_roll")] = M_PI / 18;
    q[model.getDofIndex("l_knee")] = -M_PI / 6;
    q[model.getDofIndex("r_knee")] = -M_PI / 6;
}
