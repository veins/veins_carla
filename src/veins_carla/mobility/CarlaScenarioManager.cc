//
// Copyright (C) 2023 Tobias Hardes <tobias.hardes@uni-paderborn.de>
//
// Documentation for these modules is at http://veins.car2x.org/
//
// SPDX-License-Identifier: GPL-2.0-or-later
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//

#include "CarlaScenarioManager.h"
using namespace omnetpp;
using namespace veins_carla;

Define_Module(CarlaScenarioManager);

void CarlaScenarioManager::initialize(int stage)
{
    if (stage == 0) {
        updateInterval = par("updateInterval").doubleValue();

        host = par("host").stdstringValue();
        port = par("port").intValue();
        moduleType = par("moduleType").stringValue();
        moduleName = par("moduleName").stringValue();
        moduleDisplayString = par("moduleDisplayString").stdstringValue();

        nextNodeVectorIndex = 0;
        hosts.clear();

        initMsg = new cMessage("init");
        startSimulationMsg = new cMessage();
        checkConnectionMsg = new cMessage();
        executeOneTimestepTrigger = new cMessage();
        checkConnectionMsg->setKind(checkConnectionMsgKind);
    }
    else {
        startCarlaAdapter();
    }
}

void CarlaScenarioManager::startCarlaAdapter()
{
    pid = fork();

    if (pid == 0) {
        signal(SIGINT, SIG_IGN);
        int runnumber = getSimulation()->getEnvir()->getConfigEx()->getActiveRunNumber();

        std::string sCommand = "cd ../../ && python3 ./carla_adapter.py";

        std::stringstream ssRunnumber;
        ssRunnumber << runnumber;
        std::string sRunnumber = ssRunnumber.str();
        sRunnumber = " -s " + sRunnumber;
        std::string sPort = std::string(" -p ") + std::to_string(port);
        std::cout << "update interval: " << updateInterval << std::endl;
        std::string sStepLength = std::string(" -l ") + std::to_string(updateInterval);

        std::cout << simTime().dbl() << ": executing: " << (sCommand + sRunnumber + sPort + sStepLength).c_str() << std::endl;

        int r = execl("/bin/sh", "sh", "-c", (sCommand + sRunnumber + sPort + sStepLength).c_str(), NULL);
        std::cout << simTime().dbl() << ": executing: " << sCommand << std::endl;

        if (r == -1) {
            throw cRuntimeError("system failed");
        }
        if (WEXITSTATUS(r) != 0) {
            throw cRuntimeError("cannot run");
        }
        throw cRuntimeError("returned from exec");
        exit(1);
    }
    scheduleAt(simTime() + updateInterval, startSimulationMsg);
    scheduleAt(simTime() + updateInterval, checkConnectionMsg);
}

void CarlaScenarioManager::handleMessage(cMessage* msg)
{
    if (msg == initMsg) {
        std::cout << simTime().dbl() << ": initMsg" << std::endl;
        startCarlaAdapter();
        return;
    }
    else if (msg == startSimulationMsg) {
        std::cout << simTime().dbl() << ": startSimulation" << std::endl;
        startSimulation();
        return;
    }
    else if (msg == executeOneTimestepTrigger) {
        std::cout << simTime().dbl() << ": executeOneTimestepTrigger" << std::endl;
        executeOneTimestep();
        return;
    }
}

void CarlaScenarioManager::startSimulation()
{
    std::cout <<"CarlaScenarioManager::startSimulation()" << std::endl;
    std::string hostAndPort = host+":" + std::to_string(port);
    channel = CreateChannel(hostAndPort, grpc::InsecureChannelCredentials());
    std::cout <<"CarlaScenarioManager::startSimulation(): Got a channel" << std::endl;
    stub = carla::CarlaAdapter::NewStub(channel);
    std::cout <<"CarlaScenarioManager::startSimulation(): Got new stub" << std::endl;
    auto state = channel->GetState(true);
    std::cout <<"CarlaScenarioManager::startSimulation(): Got a state" << std::endl;

    while (state != GRPC_CHANNEL_READY) {
        if (!channel->WaitForStateChange(state, std::chrono::system_clock::now() + std::chrono::seconds(15))) {
            error("Could not connect to gRPC");
        }
        state = channel->GetState(true);
        if (state == GRPC_CHANNEL_IDLE) {
            std::cout << "I do have a GRPC_CHANNEL_IDLE in WHILE -> " << state << std::endl;
        }
        else if (state == GRPC_CHANNEL_CONNECTING) {
            std::cout << "I do have a GRPC_CHANNEL_CONNECTING in WHILE -> " << state << std::endl;
        }
        else if (state == GRPC_CHANNEL_READY) {
            std::cout << "I do have a GRPC_CHANNEL_READY in WHILE -> " << state << std::endl;
        }
        else if (state == GRPC_CHANNEL_TRANSIENT_FAILURE) {
            std::cout << "I do have a GRPC_CHANNEL_TRANSIENT_FAILURE in WHILE -> " << state << std::endl;
        }
        else if (state == GRPC_CHANNEL_SHUTDOWN) {
            std::cout << "I do have a GRPC_CHANNEL_SHUTDOWN in WHILE -> " << state << std::endl;
        }
    }

    // Add all the Actors that are already part of the simulation
    carla::ActorIds actorIds = GetManagedHostIds();
    for (uint32_t i = 0; i < actorIds.actorid_size(); i++) {
        int actorId = actorIds.actorid(i);
        carla::Vehicle vehicle = GetManagedActorById(actorId);
        printVehicle(vehicle);
        Coord location;
        location.x = vehicle.location().x();
        location.y = vehicle.location().y();
        location.z = vehicle.location().z();
        std::cout << "Adding vehicle with location x = " << location.x << " y = " << location.y << " z = " << location.z << std::endl;

        addModule(std::to_string(vehicle.id()), moduleType.c_str(), moduleName.c_str(), moduleDisplayString, location, 0, 0, 4, 1.8, 4);// TODO
    }
    executeOneTimestep();
}

void CarlaScenarioManager::testInsertVehicle()
{
    carla::Vehicle vehicle;
    carla::Transform v = getRandomSpawnPoint();
    carla::Vector* pos = new carla::Vector();
    pos->set_x(v.location().x());
    pos->set_y(v.location().y());
    pos->set_z(v.location().z());

    vehicle.set_allocated_location(pos);
    insertVehicle(vehicle);
}
void CarlaScenarioManager::printVehicle(carla::Vehicle vehicle)
{
    std::cout << "=========================================" << std::endl;
    std::cout << vehicle.acceleration().x() << std::endl;
    std::cout << "Vehicle" << vehicle.id() << "\n"
        << "Acceleration.x = " << vehicle.acceleration().x() << "\n"
        << "Acceleration.y = " << vehicle.acceleration().y() << "\n"
        << "Acceleration.z = " << vehicle.acceleration().z() << "\n"
        << "Speed.x = " << vehicle.speed().x() << "\n"
        << "Speed.y = " << vehicle.speed().y() << "\n"
        << "Speed.z = " << vehicle.speed().z() << "\n"
        << "Position.x = " << vehicle.location().x() << "\n"
        << "Position.y = " << vehicle.location().y() << "\n"
        << "Position.z = " << vehicle.location().z() <<std::endl;
    std::cout << "=========================================" << std::endl;
}

carla::Vehicle CarlaScenarioManager::GetManagedActorById(int actorId)
{
    carla::Vehicle vehicle;
    carla::Number vehicleId;
    grpc::ClientContext clientContext;
    vehicleId.set_num(actorId);

    grpc::Status status = stub->GetManagedActorById(&clientContext, vehicleId, &vehicle);
    if (status.ok()) {
        return vehicle;
    }
    else {
        error((std::string("CarlaScenarioManager::GetManagedActorById() failed with error: " + std::string(status.error_message())).c_str()));
    }
}

void CarlaScenarioManager::insertVehicle(carla::Vehicle request)
{
    carla::Number vehicleId;
    grpc::ClientContext clientContext;
    std::cout << "Inserting vehicle at location: " << request.location().x() << "/" << request.location().y() << "/" << request.location().z() << std::endl;
    grpc::Status status = stub->InsertVehicle(&clientContext, request, &vehicleId);
    if (status.ok()) {
        std::cout << "Yay there is a new vehicle" << std::endl;
    }
    else {
        error((std::string("CarlaScenarioManager::insertVehicle() failed with error: " + std::string(status.error_message())).c_str()));
    }
    Coord pos;
    pos.x = request.location().x();
    pos.y = request.location().y();
    pos.z = request.location().z();

    addModule(std::to_string(vehicleId.num()), moduleType.c_str(), moduleName.c_str(), moduleDisplayString, pos, 0, 0, 4, 1.8, 4);// TODO
}

carla::Transform CarlaScenarioManager::getRandomSpawnPoint()
{
    google::protobuf::Empty request;
    carla::Transform response;
    grpc::ClientContext clientContext;
    grpc::Status status = stub->GetRandomSpawnPoint(&clientContext, request, &response);
    if (status.ok()) {
        std::cout << "getRandomSpawnPoint:" << response.location().x() << "/" << response.location().y() << "/" << response.location().z() << std::endl;
        return response;
    }
    else {
        error((std::string("CarlaScenarioManager::getRandomSpawnPoint() failed with error: " + std::string(status.error_message())).c_str()));
    }
}


void CarlaScenarioManager::executeOneTimestep()
{
    std::cout << simTime().dbl() << ": executeOneTimestep()" << std::endl;
    google::protobuf::Empty responseEmpty;
    google::protobuf::Empty empty;
    grpc::ClientContext clientContext;
    grpc::Status status = stub->ExecuteOneTimeStep(&clientContext, empty, &responseEmpty);

    if (status.ok()) {

        carla::ActorIds actors = GetManagedHostIds();
        for (uint32_t i = 0; i < actors.actorid_size(); i++) {
            auto actorId = actors.actorid(i);
            carla::Vehicle v = GetManagedActorById(actorId);
            Coord location;
            location.x = v.location().x();
            location.y = v.location().y();
            location.z = v.location().z();
            double speed = 0; // TODO
            double angle = 0;
            processVehicleSubscription(v.id(), location, speed, angle);

        }
        if (!executeOneTimestepTrigger->isScheduled()) {
            scheduleAt(simTime() + updateInterval, executeOneTimestepTrigger);
        }

    }
    else {
        error((std::string("CarlaScenarioManager::executeOneTimestep() failed with error: " + std::string(status.error_message())).c_str()));
    }
}

void CarlaScenarioManager::processVehicleSubscription(int actorId, Coord location, double speed, double angle)
{

    cModule* mod = getManagedModule(std::to_string(actorId));
    if (!mod) {

        carla::Vehicle vehicle = GetManagedActorById(actorId);
        printVehicle(vehicle);
        Coord location;
        location.x = vehicle.location().x();
        location.y = vehicle.location().y();
        location.z = vehicle.location().z();
        std::cout << "Adding vehicle with location x = " << location.x << " y = " << location.y << " z = " << location.z << std::endl;

        addModule(std::to_string(vehicle.id()), moduleType.c_str(), moduleName.c_str(),
            moduleDisplayString, location, 0, 0, 4, 1.8, 4);            // TODO
    }
    else {
        // module existed - update position

        /**********************************************/
        location.x = location.x + POSITIONOFFSET;
        location.y = location.y + POSITIONOFFSET;
        /**********************************************/

        EV_DEBUG << "module " << actorId << " moving to " << location.x << "," << location.y << " with heading" << angle << endl;
        std::cout << "module " << actorId << " moving to " << location.x << "," << location.y << " with heading" << angle << std::endl;


        updateModulePosition(mod, location, speed, angle);
    }
}
void CarlaScenarioManager::updateModulePosition(cModule* mod, const Coord& location, double speed, double angle)
{
    // update position in CarlaMobility
    auto mobilityModules = getSubmodulesOfType<CarlaMobility>(mod);
    for (auto mm : mobilityModules) {
        mm->nextPosition(location, speed, angle);
    }
}

cModule* CarlaScenarioManager::getManagedModule(std::string actorId)
{
    if (hosts.find(actorId) == hosts.end())
        return nullptr;
    return hosts[actorId];
}
carla::ActorIds CarlaScenarioManager::GetManagedHostIds()
{
    carla::ActorIds actorIds;
    google::protobuf::Empty empty;
    grpc::ClientContext clientContext;
    grpc::Status status = stub->GetManagedActorsIds(&clientContext, empty, &actorIds);

    if (status.ok()) {
        std::cout << "I got " << actorIds.actorid_size() << " actors with these ids: " << std::endl;
        for (uint32_t i = 0; i < actorIds.actorid_size(); i++) {
            std::cout << actorIds.actorid(i)<< std::endl;
        }
    }
    else {
        error((std::string("CarlaScenarioManager::GetManagedHostIds() failed with error: " + std::string(status.error_message())).c_str()));
    }
    return actorIds;
}


void CarlaScenarioManager::addModule(std::string nodeId, std::string moduleType, std::string moduleName, std::string displayString, veins::Coord& position, double speed, double angle, double length, double height, double width)
{
    std::cout << "addModule called for module id " << nodeId << std::endl;
    int32_t nodeVectorIndex = nextNodeVectorIndex++;

    cModule* parentmod = getParentModule();
    if (!parentmod)
        throw cRuntimeError("Parent Module not found");

    cModuleType* nodeType = cModuleType::get(moduleType.c_str());
    if (!nodeType)
        throw cRuntimeError("Module Type \"%s\" not found", moduleType.c_str());

#if OMNETPP_BUILDNUM >= 1525
    parentmod->setSubmoduleVectorSize(moduleName.c_str(), nodeVectorIndex + 1);
    cModule* mod = nodeType->create(moduleName.c_str(), parentmod, nodeVectorIndex);
#else
    cModule* mod = nodeType->create(moduleName.c_str(), parentmod, nodeVectorIndex, nodeVectorIndex);
#endif
    mod->finalizeParameters();
    std::cout << "Display string is: " << displayString << std::endl;
    if (displayString.length() > 0) {
        mod->getDisplayString().parse(displayString.c_str());
    }
    mod->buildInside();
    mod->scheduleStart(simTime() + updateInterval);

    /**********************************************/
    position.x = position.x + POSITIONOFFSET;
    position.y = position.y + POSITIONOFFSET;
    /**********************************************/
    preInitializeModule(mod, nodeId, position, speed, angle);

    mod->callInitialize();
    hosts[nodeId] = mod;
}



void CarlaScenarioManager::preInitializeModule(cModule* mod, const std::string& nodeId, const Coord& position, double speed, double angle)
{
    // pre-initialize Mobility
    auto mobilityModules = getSubmodulesOfType<CarlaMobility>(mod);
    for (auto mm : mobilityModules) {
        mm->preInitialize(nodeId, position, speed, angle);
    }
}

