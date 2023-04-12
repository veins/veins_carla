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

#include <omnetpp.h>
#include <google/protobuf/empty.pb.h>
#include <grpcpp/grpcpp.h>
#include <signal.h>

#include "../proto/carla.grpc.pb.h"
#include "veins/base/utils/Coord.h"
#include "veins/modules/mobility/traci/TraCICoordinateTransformation.h"

#include "CarlaMobility.h"

using namespace omnetpp;
// using namespace veins;

namespace veins_carla {
class CarlaScenarioManager : public cSimpleModule {
protected:
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage* msg) override;
    int numInitStages() const override
    {
        return std::max(cSimpleModule::numInitStages(), 2);
    }

    void startCarlaAdapter();
    void startSimulation();
    void insertVehicle(carla::Vehicle request);
    carla::ActorIds GetManagedHostIds();
    carla::Vehicle GetManagedActorById(int actorId);
    carla::Transform getRandomSpawnPoint();

private:
    void executeOneTimestep();
    void addModule(std::string nodeId, std::string moduleType, std::string moduleName, std::string displayString, veins::Coord& position, double speed, double angle, double length, double height, double width);
    void printVehicle(carla::Vehicle vehicle);
    void preInitializeModule(cModule* mod, const std::string& nodeId, const Coord& position, double speed, double angle);
    void processVehicleSubscription(int actorId, Coord location, double speed, double angle);
    cModule* getManagedModule(std::string actorId);
    void updateModulePosition(cModule* mod, const Coord& location, double speed, double angle);
    void testInsertVehicle();
private:
    std::shared_ptr<grpc::Channel> channel;
    // std::unique_ptr<carla::Carla::Stub> stub;
    std::unique_ptr<carla::CarlaAdapter::Stub> stub;
    double updateInterval;
    double simTimeLimit;

    int POSITIONOFFSET = 2000;

    int port;
    std::string host;

    omnetpp::cMessage* initMsg;
    omnetpp::cMessage* startSimulationMsg;
    omnetpp::cMessage* executeOneTimestepTrigger;
    omnetpp::cMessage* checkConnectionMsg;
    int checkConnectionMsgKind = 1337;
    size_t nextNodeVectorIndex; /**< next OMNeT++ module vector index to use */

    std::map<std::string, cModule*> hosts; /**< vector of all hosts managed by us */

    std::string moduleDisplayString;
    std::string moduleType;
    std::string moduleName;

    pid_t pid;


    std::unique_ptr<TraCICoordinateTransformation> coordinateTransformation;
};

} // namespace veins_carla
