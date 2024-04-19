/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
 
#include <iostream>
#include <fstream>


#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/netanim-module.h"

// wifi includes
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"
#include "ns3/ssid.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/qos-utils.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("SecondScriptExample");

int
main(int argc, char* argv[])
{
    bool verbose = true;
    uint32_t nCsma = 3;


    // Passing user defined data
    CommandLine cmd(__FILE__);
    cmd.AddValue("nCsma", "Number of \"extra\" CSMA nodes/devices", nCsma);
    cmd.AddValue("verbose", "Tell echo applications to log if true", verbose);
    
    cmd.Parse(argc, argv);

    std::cout<<nCsma<<"CSMA value\n";
    if (verbose)
    {
        LogComponentEnable("UdpEchoClientApplication", LOG_LEVEL_INFO);
        LogComponentEnable("UdpEchoServerApplication", LOG_LEVEL_INFO);
    }

    nCsma = nCsma == 0 ? 1 : nCsma;

    

    NodeContainer csmaNodes;
    csmaNodes.Create(3);
    
    NodeContainer p2pNodes;
    p2pNodes.Add(csmaNodes.Get(2));
    p2pNodes.Create(1); //ERROR, made change
    
    CsmaHelper csma;
    csma.SetChannelAttribute("DataRate", StringValue("200Mbps"));
    csma.SetChannelAttribute("Delay", TimeValue(MilliSeconds(12)));

    NetDeviceContainer csmaDevices;
    csmaDevices = csma.Install(csmaNodes);
    
    PointToPointHelper pointToPoint;
    pointToPoint.SetDeviceAttribute("DataRate", StringValue("8Mbps"));
    pointToPoint.SetChannelAttribute("Delay", StringValue("5ms"));

    NetDeviceContainer p2pDevices;
    p2pDevices = pointToPoint.Install(p2pNodes.Get(0), p2pNodes.Get(1));

    
    InternetStackHelper stack;

// Install Internet stack on the CSMA nodes
stack.Install(csmaNodes);

// Install Internet stack on the point-to-point node
stack.Install(p2pNodes.Get(1));

	
	Ipv4AddressHelper address;

address.SetBase("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer csmaInterfaces;
    csmaInterfaces = address.Assign(csmaDevices);

    
    address.SetBase("10.1.2.0", "255.255.255.0");
    Ipv4InterfaceContainer p2pInterfaces;
    p2pInterfaces = address.Assign(p2pDevices);

    

    UdpEchoServerHelper echoServer(9);

    ApplicationContainer serverApps = echoServer.Install(p2pNodes.Get(1));
    serverApps.Start(Seconds(1.0));
    serverApps.Stop(Seconds(10.0));

    UdpEchoClientHelper echoClient(p2pInterfaces.GetAddress(1), 9);
    echoClient.SetAttribute("MaxPackets", UintegerValue(1));
    echoClient.SetAttribute("Interval", TimeValue(Seconds(1.0)));
    echoClient.SetAttribute("PacketSize", UintegerValue(1024));

    ApplicationContainer clientApps = echoClient.Install(csmaNodes.Get(0));
    clientApps.Start(Seconds(2.0));
    clientApps.Stop(Seconds(10.0));
    
    //InternetStackHelper stack1; WTF WHY IS THIS THERE???
    //stack1.Install (csmaNodes);
    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    AnimationInterface anim("mycode.xml");
    anim.SetConstantPosition(p2pNodes.Get(0), 30.0, 100.0);
    anim.SetConstantPosition(p2pNodes.Get(1), 50.0, 80.0);
    anim.SetConstantPosition(csmaNodes.Get(0), 30.0, 50.0);
    anim.SetConstantPosition(csmaNodes.Get(1), 50.0, 50.0);

    AsciiTraceHelper ascii;
    pointToPoint.EnableAsciiAll(ascii.CreateFileStream("endsem1.tr"));
    csma.EnableAsciiAll(ascii.CreateFileStream("endsem1.tr"));

    Simulator::Run();
    Simulator::Destroy();
    return 0;

}
