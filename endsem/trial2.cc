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
#include "ns3/mobility-module.h" // Include mobility module
#include "ns3/constant-position-mobility-model.h"
#include "ns3/ssid.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/qos-utils.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("SecondScriptExample");

int main(int argc, char* argv[]) {
    bool verbose = true;
    uint32_t nCsma = 3;

    std::cout << nCsma << " CSMA value\n";

    if (verbose) {
        LogComponentEnable("UdpEchoClientApplication", LOG_LEVEL_INFO);
        LogComponentEnable("UdpEchoServerApplication", LOG_LEVEL_INFO);
        LogComponentEnable("OnOffApplication", LOG_LEVEL_INFO);
    }

    nCsma = nCsma == 0 ? 1 : nCsma;

    NodeContainer csmaNodes;
    csmaNodes.Create(3);

    NodeContainer p2pNodes;
    p2pNodes.Add(csmaNodes.Get(2));
    p2pNodes.Create(1);

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
    
    NodeContainer wifiStaNodes, wifiAPNodes;
    wifiStaNodes.Create(5);
    wifiAPNodes.Add(p2pNodes.Get(1));
    
    NodeContainer p2pNodes1;
    p2pNodes1.Create(2);
    NetDeviceContainer p2pDevices1, csmaDevices1;
    p2pDevices1 = pointToPoint.Install(p2pNodes1.Get(0), p2pNodes1.Get(1));
    
    wifiStaNodes.Add(p2pNodes1.Get(0));
    
    NodeContainer csmaNodes1;
    csmaNodes1.Add(p2pNodes1.Get(1));
    csmaNodes1.Create(2);
    
    csmaDevices1 = csma.Install(csmaNodes1);

    InternetStackHelper stack;
    stack.Install(csmaNodes);
    stack.Install(wifiAPNodes);
    stack.Install(wifiStaNodes);
    stack.Install(csmaNodes1);
    
    // Configure wifi
    WifiMacHelper wifiMac;
    WifiHelper wifiHelper;
    wifiHelper.SetStandard(WIFI_STANDARD_80211n_5GHZ);

    // Set up wifi channel
    YansWifiChannelHelper wifiChannel;
    wifiChannel.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");
    wifiChannel.AddPropagationLoss("ns3::FriisPropagationLossModel", "Frequency", DoubleValue(5e9));

    // Set up wifi physical layer
    YansWifiPhyHelper wifiPhy;
    wifiPhy.SetChannel(wifiChannel.Create());
    wifiPhy.SetErrorRateModel("ns3::YansErrorRateModel");
    wifiHelper.SetRemoteStationManager("ns3::ConstantRateWifiManager",
                                       "DataMode", StringValue("HtMcs7"),
                                       "ControlMode", StringValue("HtMcs0"));
   
    Ssid ssid = Ssid("network");                                  
    // Configure AP
    wifiMac.SetType("ns3::ApWifiMac",
                    "Ssid", SsidValue(ssid));
    NetDeviceContainer apDevice = wifiHelper.Install(wifiPhy, wifiMac, wifiAPNodes);                                  
                                     
    // Configure STA
    wifiMac.SetType("ns3::StaWifiMac",
                    "Ssid", SsidValue(ssid));
    NetDeviceContainer staDevices;
    staDevices = wifiHelper.Install(wifiPhy, wifiMac, wifiStaNodes);
  
    // Set up mobility model for all nodes
    MobilityHelper mobility;
    Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator>();

    positionAlloc->Add(Vector(30.0, 0.0, 0.0));
    positionAlloc->Add(Vector(50.0, 0.0, 0.0));
    positionAlloc->Add(Vector(30.0, 120.0, 0.0));
    positionAlloc->Add(Vector(50.0, 80.0, 0.0));
    positionAlloc->Add(Vector(70.0, 70.0, 0.0));
    positionAlloc->Add(Vector(110.0, 20.0, 0.0));
    positionAlloc->Add(Vector(110.0, 70.0, 0.0));
    positionAlloc->Add(Vector(110.0, 120.0, 0.0));
    positionAlloc->Add(Vector(160.0, 70.0, 0.0));
    positionAlloc->Add(Vector(190.0, 80.0, 0.0));
    positionAlloc->Add(Vector(210.0, 120.0, 0.0));
    positionAlloc->Add(Vector(190.0, 20.0, 0.0));
    positionAlloc->Add(Vector(240.0, 20.0, 0.0));
    
    mobility.SetPositionAllocator(positionAlloc);
    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobility.Install(csmaNodes);
    mobility.Install(wifiAPNodes);
    mobility.Install(wifiStaNodes);
    mobility.Install(csmaNodes1);

    // Set up IP addresses
    Ipv4AddressHelper address;
    address.SetBase("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer csmaInterfaces, csmaInterfaces1;
    csmaInterfaces = address.Assign(csmaDevices);

    address.SetBase("10.1.2.0", "255.255.255.0");
    Ipv4InterfaceContainer p2pInterfaces, p2pInterfaces1;
    p2pInterfaces = address.Assign(p2pDevices);
    
    address.SetBase("10.1.3.0", "255.255.255.0");
    Ipv4InterfaceContainer apInterface, staInterface;
    apInterface = address.Assign(apDevice);
    staInterface = address.Assign(staDevices);
    
    address.SetBase("10.1.4.0", "255.255.255.0");
    p2pInterfaces1 = address.Assign(p2pDevices1);
    
    address.SetBase("10.1.5.0", "255.255.255.0");
    csmaInterfaces1 = address.Assign(csmaDevices1);

    // Application setup (UdpEcho, TCP)
    UdpEchoServerHelper echoServer(9);
    ApplicationContainer serverApps = echoServer.Install(p2pNodes.Get(1)); //n3
    serverApps.Start(Seconds(0.5));
    serverApps.Stop(Seconds(5.0));

    UdpEchoClientHelper echoClient(p2pInterfaces.GetAddress(1), 9);
    echoClient.SetAttribute("MaxPackets", UintegerValue(4));
    echoClient.SetAttribute("Interval", TimeValue(Seconds(1.0)));
    echoClient.SetAttribute("PacketSize", UintegerValue(1024));
    ApplicationContainer clientApps = echoClient.Install(p2pNodes1.Get(0)); //N9
    clientApps.Start(Seconds(1.0));
    clientApps.Stop(Seconds(5.0)); 

    // TCP application setup
 /*   uint16_t port = 9; // Discard port (RFC 863)
    OnOffHelper TcpOnoff("ns3::TcpSocketFactory", Address(InetSocketAddress(csmaInterfaces.GetAddress(0), port)));
    TcpOnoff.SetConstantRate(DataRate("448kb/s"));
    TcpOnoff.SetAttribute("OnTime", StringValue("ns3::ConstantRandomVariable[Constant=0.05]")); // Set the OnTime to 50 milliseconds (0.05 seconds)
    ApplicationContainer TcpApps = TcpOnoff.Install(csmaNodes1.Get(2)); // N12
    TcpApps.Start(Seconds(3.0));
    TcpApps.Stop(Seconds(11.0));

    PacketSinkHelper TcpSink("ns3::TcpSocketFactory", Address(InetSocketAddress(Ipv4Address::GetAny(), port)));
    TcpApps = TcpSink.Install(csmaNodes.Get(0)); // N1
    TcpApps.Start(Seconds(2.5));
    TcpApps.Stop(Seconds(11.0)); */

    Ipv4GlobalRoutingHelper::PopulateRoutingTables();
    
    // Animation setup
    AnimationInterface anim("mycode2.xml");
    anim.SetMaxPktsPerTraceFile(50000000);
    
    Simulator::Stop(Seconds(15));

    AsciiTraceHelper ascii;
    pointToPoint.EnableAsciiAll(ascii.CreateFileStream("endsem1.tr"));
    csma.EnableAsciiAll(ascii.CreateFileStream("endsem1.tr"));

    Simulator::Run();
    Simulator::Destroy();

    return 0;
}

