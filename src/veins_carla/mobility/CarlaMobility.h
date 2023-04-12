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
#include "veins_carla/veins_carla.h"
#include "veins/base/utils/Coord.h"
#include "veins/base/modules/BaseMobility.h"
#include "veins/base/utils/FindModule.h"
#include "veins/base/utils/Heading.h"
using namespace omnetpp;
using namespace veins;

namespace veins_carla {

class CarlaMobility : public BaseMobility {

protected:
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage* msg) override;
    int numInitStages() const override
    {
        return std::max(cSimpleModule::numInitStages(), 2);
    }

public:
    CarlaMobility()
        : BaseMobility()
        , isPreInitialized(false)
    {

    }
    void preInitialize(std::string external_id, const Coord& position, double speed, double angle);
    virtual void changePosition();
    void nextPosition(const Coord& position, double speed, double angle);
protected:
    cOutVector currentPosXVec;   /**< vector plotting posx */
    cOutVector currentPosYVec;   /**< vector plotting posy */
    cOutVector currentSpeedVec;   /**< vector plotting speed */
    cOutVector currentAccelerationVec;   /**< vector plotting acceleration */
    cOutVector currentCO2EmissionVec;   /**< vector plotting current CO2 emission */

    bool isPreInitialized;   /**< true if preInitialize() has been called immediately before initialize() */

    std::string external_id;   /**< updated by setExternalId() */
    double hostPositionOffset;   /**< front offset for the antenna on this car */
    bool setHostSpeed;   /**< whether to update the speed of the host (along with its position)  */

    simtime_t lastUpdate;   /**< updated by nextPosition() */
    Coord roadPosition;   /**< position of front bumper, updated by nextPosition() */
    double speed;   /**< updated by nextPosition() */
    Heading heading;   /**< updated by nextPosition() */
private:

    /**
     * Calculates where the OMNeT++ module position of this UAV should be, given its front position
     */
    Coord calculateHostPosition(const Coord& vehiclePos) const;

    void fixIfHostGetsOutside() override;   /**< called after each read to check for (and handle) invalid positions */
};


class VEINS_API CarlaMobilityAccess {
public:
    CarlaMobility* get(cModule* host)
    {
        CarlaMobility* droci = FindModule<CarlaMobility*>::findSubModule(host);
        ASSERT(droci);
        return droci;
    };
};
}   // namespace airmobisim
