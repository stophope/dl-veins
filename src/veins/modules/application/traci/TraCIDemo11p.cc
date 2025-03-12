//
// Copyright (C) 2006-2011 Christoph Sommer <christoph.sommer@uibk.ac.at>
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

#include "veins/modules/application/traci/TraCIDemo11p.h"

#include "veins/modules/application/traci/TraCIDemo11pMessage_m.h"

#include "veins/modules/messages/ReportMessage_m.h"

#include <random>


int getRandomNumber(int min, int max) {
    std::random_device rd;  // 获取随机数种子
    std::mt19937 gen(rd()); // 创建随机数生成器
    std::uniform_int_distribution<> distr(min, max); // 创建分布对象
    return distr(gen);
}

using namespace veins;

Define_Module(veins::TraCIDemo11p);

void TraCIDemo11p::initialize(int stage)
{
    DemoBaseApplLayer::initialize(stage);
    if (stage == 0) {
        sentMessage = false;
        lastDroveAt = simTime();
        currentSubscribedServiceId = -1;
    }

    /*if (stage == 1) {
        ReportMessage* rm = new ReportMessage();
        populateWSM(rm);
        rm->setSenderAddress(myId);
        scheduleAt(simTime() + uniform(0.01, 0.2), rm);
    }*/

        if (stage == 1) {

            ReportMessage* rm = new ReportMessage();
            populateWSM(rm);
                   rm->setSenderAddress(myId);
                   scheduleAt(simTime() + uniform(0.01, 0.2), rm);

               random_device rd;
               mt19937 gen(rd());
               bernoulli_distribution bd_mal(0.2); //车辆是否恶意，服从伯努利分布，恶意概率0.2
               //isMalicious = bd_mal(gen);
               isMalicious =getRandomNumber(0,1);

               if (isMalicious) { // 恶意车辆ID初始化
                   RID = myId;
                   VID = getRandomNumber(1,120);    // generateVID(myId)，伪ID生成函数，可以自己设计
               } else { // 正常车辆ID初始化
                   RID = myId;
                   VID = myId;
               }
           }
}



//void TraCIDemo11p::initialize(int stage)
//{
//    DemoBaseApplLayer::initialize(stage);
//    if (stage == 0) {
//        sentMessage = false;
//        lastDroveAt = simTime();
//        currentSubscribedServiceId = -1;
//    }
//    if (stage == 1) {
//
//        ReportMessage* rm = new ReportMessage();
//        populateWSM(rm);
//               rm->setSenderAddress(myId);
//               scheduleAt(simTime() + uniform(0.01, 0.2), rm);
//
//           random_device rd;
//           mt19937 gen(rd());
//           bernoulli_distribution bd_mal(0.2); //车辆是否恶意，服从伯努利分布，恶意概率0.2
//           //isMalicious = bd_mal(gen);
//           isMalicious =getRandomNumber(0,1);
//
//           if (isMalicious) { // 恶意车辆ID初始化
//               RID = myId;
//               VID = getRandomNumber(1,120);    // generateVID(myId)，伪ID生成函数，可以自己设计
//           } else { // 正常车辆ID初始化
//               RID = myId;
//           }
//       }
//}





void TraCIDemo11p::onWSA(DemoServiceAdvertisment* wsa)
{
    if (currentSubscribedServiceId == -1) {
        mac->changeServiceChannel(static_cast<Channel>(wsa->getTargetChannel()));
        currentSubscribedServiceId = wsa->getPsid();
        if (currentOfferedServiceId != wsa->getPsid()) {
            stopService();
            startService(static_cast<Channel>(wsa->getTargetChannel()), wsa->getPsid(), "Mirrored Traffic Service");
        }
    }
}

void TraCIDemo11p::onWSM(BaseFrame1609_4* frame)
{
    TraCIDemo11pMessage* wsm = check_and_cast<TraCIDemo11pMessage*>(frame);

    findHost()->getDisplayString().setTagArg("i", 1, "green");

    if (mobility->getRoadId()[0] != ':') traciVehicle->changeRoute(wsm->getDemoData(), 9999);
    if (!sentMessage) {
        sentMessage = true;
        // repeat the received traffic update once in 2 seconds plus some random delay
        wsm->setSenderAddress(myId);
        wsm->setSerial(3);
        scheduleAt(simTime() + 2 + uniform(0.01, 0.2), wsm->dup());
    }
}


void TraCIDemo11p::handleSelfMsg(cMessage* msg)
{
    if (TraCIDemo11pMessage* wsm = dynamic_cast<TraCIDemo11pMessage*>(msg)) {
        // send this message on the service channel until the counter is 3 or higher.
        // this code only runs when channel switching is enabled
        sendDown(wsm->dup());
        wsm->setSerial(wsm->getSerial() + 1);
        if (wsm->getSerial() >= 3) {
            // stop service advertisements
            stopService();
            delete (wsm);
        }
        else {
            scheduleAt(simTime() + 1, wsm);
        }
    }
    else {
        DemoBaseApplLayer::handleSelfMsg(msg);
    }

}

//void TraCIDemo11p::handlePositionUpdate(cObject* obj)
//{
//    DemoBaseApplLayer::handlePositionUpdate(obj);
//
//    cModule *car = getParentModule();
//    TraCIMobility *traci = dynamic_cast<TraCIMobility*>(car->getSubmodule("veinsmobility", 0));
//
//    fstream f1;
//           f1.open("test.txt", std::ios::out | std::ios::app);
//
//            if (!f1) {
//                std::cerr << "Cannot open the file." << std::endl;
//            }
//
//            // Assuming curPosition and curSpeed are of a type that supports this formatting
//            f1 << this->myId<<" "
//               << std::fixed << std::setprecision(2) << this->curPosition.x << " "
//               << this->curPosition.y << " "
//               << this->curPosition.z << " "
//               << this->curSpeed.x << " "
//               << this->curSpeed.y << " "
//               << this->curSpeed.z << " "
//               << traci->getSpeed()<<endl;
//
//            f1.close();
//
//            cout<<1<<endl;
//
//            EV<<traci->getRoadId()<<endl;
//
//                EV<<traci->getHeading()<<endl;
//
//                if (traci) {
//                    //Coord position = traci->getPosition();  // 调整此行代码
//                    double speed = traci->getSpeed();        // 调整此行代码
//
//                    /*const TraCIMobility::Statistics& stats = traci->getStatistics();
//                    EV << "First Road Number: " << stats.firstRoadNumber << endl;
//                        EV << "Start Time: " << stats.startTime << endl;
//                        EV << "Total Time: " << stats.totalTime << endl;
//                        EV << "Stop Time: " << stats.stopTime << endl;
//                        EV << "Min Speed: " << stats.minSpeed << endl;
//                        EV << "Max Speed: " << stats.maxSpeed << endl;
//                        EV << "Total Distance: " << stats.totalDistance << endl;
//                        EV << "Total CO2 Emission: " << stats.totalCO2Emission << endl;*/
//
//                    EV << traci->getExternalId()<<endl;
//
//                    // 加速度可能需要手动计算或使用不同的方法
//                    // Coord acceleration = calculateAcceleration();
//
//                    //EV << "Position: " << position << "\n";
//                    EV << "Speed: " << speed << "\n";
//                    // EV << "Acceleration: " << acceleration << "\n";
//                } else {
//                    EV << "未找到 TraCIMobility 模块！\n";
//                }
//
//
//
//
//    // stopped for for at least 10s?
//    if (mobility->getSpeed() < 1) {
//        if (simTime() - lastDroveAt >= 10 && sentMessage == false) {
//            findHost()->getDisplayString().setTagArg("i", 1, "red");
//            sentMessage = true;
//
//            TraCIDemo11pMessage* wsm = new TraCIDemo11pMessage();
//            populateWSM(wsm);
//            wsm->setDemoData(mobility->getRoadId().c_str());
//
//            // host is standing still due to crash
//            if (dataOnSch) {
//                startService(Channel::sch2, 42, "Traffic Information Service");
//                // started service and server advertising, schedule message to self to send later
//                scheduleAt(computeAsynchronousSendingTime(1, ChannelType::service), wsm);
//            }
//            else {
//                // send right away on CCH, because channel switching is disabled
//                sendDown(wsm);
//            }
//        }
//    }
//    else {
//        lastDroveAt = simTime();
//    }
//}


void TraCIDemo11p::handlePositionUpdate(cObject* obj)
{
    DemoBaseApplLayer::handlePositionUpdate(obj);

    ostringstream oss;
    string beaconMsg;
    double timestamp = simTime().dbl();

    cModule *car = getParentModule();
    TraCIMobility *traci = dynamic_cast<TraCIMobility*>(car->getSubmodule("veinsmobility", 0));

//    cout<<this->myId<<" "
//                   << this->curPosition.x << " "
//                   << this->curPosition.y << " "
//                   << this->curPosition.z << " "
//                   << this->curSpeed.x << " "
//                   << this->curSpeed.y << " "
//                   << this->curSpeed.z << " "
//                   << traci->getSpeed()<< " "
//                   << traci->getHeading()<<endl;

       if (!isMalicious) { // 正常车辆行为

            oss << this->RID << "||" << this->curPosition.x << "||" << this->curPosition.y << "||"
                    <<  this->curSpeed.x << "||" << this->curSpeed.y << "||"
                    <<  traci->getSpeed() << "||" << traci->getHeading() << "||"<< 0 <<endl;

            beaconMsg = oss.str();
            cout << "正常车辆[" << this->RID << "]广播位置：" << beaconMsg << endl;


            fstream f1;
                                       f1.open("test.txt", std::ios::out | std::ios::app);

                                        if (!f1) {
                                            std::cerr << "Cannot open the file." << std::endl;
                                        }

                                        // Assuming curPosition and curSpeed are of a type that supports this formatting
                                        f1 << this->RID<<" "
                                           << std::fixed << std::setprecision(2) << this->curPosition.x << " "
                                           << this->curPosition.y << " "
                                           << this->curSpeed.x << " "
                                           << this->curSpeed.y << " "
                                           << traci->getSpeed() << " "
                                           << traci->getHeading()<< " "
                                           << 0<<endl;

                                        f1.close();

        }

        if (isMalicious) {

        int MalBehaviorSelect = getRandomNumber(1,4); // 随机恶意行为选择 1-4;

        if (MalBehaviorSelect == 1) { // 恶意节点行为1：广播固定位置（其实在动，却告诉别人不动）

            this->curPosition.x = 0;
            this->curPosition.y = 0;

        }
        else if (MalBehaviorSelect == 2) { // 恶意节点行为2：广播随机位置（地图上任意点）

             this->curPosition.x = getRandomNumber(200,2500);
             this->curPosition.y = getRandomNumber(10000,20000);

        }
        else if (MalBehaviorSelect == 3) { // 恶意节点行为3：广播真实位置+固定偏移

            double pos_x_add = 20; // 固定偏移量
            double pos_y_add = 20;
            this->curPosition.x += pos_x_add;
            this->curPosition.y += pos_y_add;

        }
        else if (MalBehaviorSelect == 4) { // 恶意节点行为4：广播真实位置+随机偏移

            double pos_x_add = getRandomNumber(0,500); // 随机偏移量
            double pos_y_add = getRandomNumber(0,500);
            this->curPosition.x += pos_x_add;
            this->curPosition.y += pos_y_add;
        }
        else { // 可扩展设置默认恶意行为

        }
        oss << this->VID << "||" << this->curPosition.x << "||" << this->curPosition.y << "||"
                            <<  this->curSpeed.x << "||" << this->curSpeed.y << "||"
                            <<  traci->getSpeed() << "||" << traci->getHeading() << "||" << MalBehaviorSelect << endl;
        beaconMsg = oss.str();
        cout << "not正常车辆[" << this->VID << "]广播位置：" << beaconMsg << endl;



        fstream f1;
                           f1.open("test.txt", std::ios::out | std::ios::app);

                            if (!f1) {
                                std::cerr << "Cannot open the file." << std::endl;
                            }

                            // Assuming curPosition and curSpeed are of a type that supports this formatting
                            f1 << this->RID<<" "
                               << std::fixed << std::setprecision(2) << this->curPosition.x << " "
                               << this->curPosition.y << " "
                               << this->curSpeed.x << " "
                               << this->curSpeed.y << " "
                               << traci->getSpeed() << " "
                               << traci->getHeading()<< " "
                               << MalBehaviorSelect<<endl;

                            f1.close();


    }




//        // 打包消息
//        TraCIDemo11pMessage* newWSM = new TraCIDemo11pMessage();
//        populateWSM(newWSM);
//        newWSM->setDemoData(beaconMsg.data());
//        sendDelayedDown(newWSM->dup(), uniform(0.01, 0.1));
//
//        delete newWSM;
}




