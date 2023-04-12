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

#ifndef __VEINS_CARLA_EXAMPLECARLAAPPLICATION_H_
#define __VEINS_CARLA_EXAMPLECARLAAPPLICATION_H_

#include <omnetpp.h>

#include "veins/modules/application/ieee80211p/DemoBaseApplLayer.h"


using namespace omnetpp;
using namespace veins;
namespace veins_carla {
class ExampleCarlaApplication : public DemoBaseApplLayer {
public:
    void initialize(int stage) override;
    void finish() override;

protected:
    void onBSM(DemoSafetyMessage* bsm) override;
    void onWSM(BaseFrame1609_4* wsm) override;
    void onWSA(DemoServiceAdvertisment* wsa) override;

    void handleSelfMsg(cMessage* msg) override;
    void handlePositionUpdate(cObject* obj) override;
    void handleLowerMsg(cMessage* msg) override;
    void handleUpperMsg(cMessage* msg) override;

private:
    cMessage* txMessage;
};

} // namespace veins_carla

#endif
