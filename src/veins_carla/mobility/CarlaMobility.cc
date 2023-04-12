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

#include "CarlaMobility.h"
using namespace veins_carla;
using namespace veins;
Define_Module(CarlaMobility);

void CarlaMobility::initialize(int stage)
{
    if (stage == 0) {
        BaseMobility::initialize(stage);
        hostPositionOffset = par("hostPositionOffset");
        setHostSpeed = par("setHostSpeed");

        currentPosXVec.setName("posx");
        currentPosYVec.setName("posy");
        currentSpeedVec.setName("speed");
        currentAccelerationVec.setName("acceleration");
        currentCO2EmissionVec.setName("co2emission");

        ASSERT(isPreInitialized);
        isPreInitialized = false;
    }
}

void CarlaMobility::handleMessage(cMessage* msg)
{

}

void CarlaMobility::preInitialize(std::string external_id, const Coord& position, double speed, double angle)
{
    Heading heading_new(angle);

    this->external_id = external_id;
    this->lastUpdate = 0;
    this->roadPosition = position;
    this->speed = speed;
    this->hostPositionOffset = par("hostPositionOffset");
    this->setHostSpeed = par("setHostSpeed");
    this->heading = heading_new;

    Coord nextPos = calculateHostPosition(roadPosition);
    nextPos.z = move.getStartPosition().z;

    move.setStart(nextPos);
    move.setDirectionByVector(heading.toCoord());
    move.setOrientationByVector(heading.toCoord());


    if (this->setHostSpeed) {
        move.setSpeed(speed);
    }

    isPreInitialized = true;
}


void CarlaMobility::nextPosition(const Coord& position, double speed, double angle)
{
    Heading heading_new(angle);

    EV_DEBUG << "nextPosition " << position.x << " " << position.y << " " << speed << " " << std::endl;
    isPreInitialized = false;
    this->roadPosition = position;
    this->speed = speed;
    this->heading = heading_new;

    changePosition();
}
void CarlaMobility::changePosition()
{

    // ensure we're not called twice in one time step
    // ASSERT(lastUpdate != simTime());

    Coord nextPos = calculateHostPosition(roadPosition);
    nextPos.z = move.getStartPosition().z;

    // keep statistics (for current step)
    currentPosXVec.record(nextPos.x);
    currentPosYVec.record(nextPos.y);


    this->lastUpdate = simTime();

    // Update display string to show node is getting updates
    auto hostMod = getParentModule();
    if (std::string(hostMod->getDisplayString().getTagArg("veins", 0))
        == ". ") {
        hostMod->getDisplayString().setTagArg("veins", 0, " .");
    }
    else {
        hostMod->getDisplayString().setTagArg("veins", 0, ". ");
    }

    move.setStart(Coord(nextPos.x, nextPos.y, move.getStartPosition().z)); // keep z position
    move.setDirectionByVector(heading.toCoord());
    move.setOrientationByVector(heading.toCoord());
    if (this->setHostSpeed) {
        move.setSpeed(speed);
    }
    fixIfHostGetsOutside();
    updatePosition();
}

void CarlaMobility::fixIfHostGetsOutside()
{
    Coord pos = move.getStartPos();
    Coord dummy = Coord::ZERO;
    double dum;

    bool outsideX = (pos.x < 0) || (pos.x >= playgroundSizeX());
    bool outsideY = (pos.y < 0) || (pos.y >= playgroundSizeY());
    bool outsideZ = (!world->use2D()) && ((pos.z < 0) || (pos.z >= playgroundSizeZ()));
    if (outsideX || outsideY || outsideZ) {
        throw cRuntimeError("Tried moving host to (%f, %f) which is outside the playground", pos.x, pos.y);
    }

    handleIfOutside(RAISEERROR, pos, dummy, dummy, dum);
}

Coord CarlaMobility::calculateHostPosition(const Coord& vehiclePos) const
{
    Coord corPos;
    if (hostPositionOffset >= 0.001) {
        // calculate antenna position of vehicle according to antenna offset
        corPos = vehiclePos - (heading.toCoord() * hostPositionOffset);
    }
    else {
        corPos = vehiclePos;
    }
    return vehiclePos;
}
