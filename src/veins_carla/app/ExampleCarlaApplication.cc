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

#include "ExampleCarlaApplication.h"
#include "veins/modules/application/traci/TraCIDemo11pMessage_m.h"
using namespace veins_carla;
using namespace veins;
Define_Module(ExampleCarlaApplication);

void ExampleCarlaApplication::initialize(int stage)
{
    if (stage == 0) {
        txMessage = new cMessage();
        txMessage->setKind(1337);
        std::cout << getFullPath() << ": Initializing " << par("appName").stringValue() << std::endl;
    }
    else {
        scheduleAt(simTime() + intuniform(1, 10), txMessage);
    }
}

void ExampleCarlaApplication::finish()
{
    DemoBaseApplLayer::finish();
    // statistics recording goes here
}

void ExampleCarlaApplication::onBSM(DemoSafetyMessage* bsm)
{
    // Your application has received a beacon message from another car or RSU
    // code for handling the message goes here
}

void ExampleCarlaApplication::onWSM(BaseFrame1609_4* wsm)
{
    // Your application has received a data message from another car or RSU
    // code for handling the message goes here, see TraciDemo11p.cc for examples
}

void ExampleCarlaApplication::onWSA(DemoServiceAdvertisment* wsa)
{
    // Your application has received a service advertisement from another car or RSU
    // code for handling the message goes here, see TraciDemo11p.cc for examples
}
void ExampleCarlaApplication::handleUpperMsg(cMessage* msg)
{

}
void ExampleCarlaApplication::handleLowerMsg(cMessage* msg)
{

}

void ExampleCarlaApplication::handleSelfMsg(cMessage* msg)
{
    switch (msg->getKind()) {
    case SEND_BEACON_EVT: {
        return;
    }
    case SEND_WSA_EVT: {
        return;
    }
    case 1337: {
        TraCIDemo11pMessage* wsm = new TraCIDemo11pMessage();
        populateWSM(wsm, -1);
        wsm->setDemoData("demodata");
        send(wsm, "lowerLayerOut");
        std::cout << getFullPath() << ": Sending message" << std::endl;
        return;
    }
    }
    DemoBaseApplLayer::handleSelfMsg(msg);
    // this method is for self messages (mostly timers)
    // it is important to call the DemoBaseApplLayer function for BSM and WSM transmission

}

void ExampleCarlaApplication::handlePositionUpdate(cObject* obj)
{
    DemoBaseApplLayer::handlePositionUpdate(obj);
    // the vehicle has moved. Code that reacts to new positions goes here.
    // member variables such as currentPosition and currentSpeed are updated in the parent class
}
