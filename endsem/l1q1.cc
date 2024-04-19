#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/netanim-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("FirstScriptExample");

int
main (int argc, char *argv[])
{

  std::string animFile = "ddl1q1.xml"; 
  CommandLine cmd (__FILE__);
  cmd.Parse (argc, argv);
  
  Time::SetResolution (Time::NS);
  LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
  LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);

  NodeContainer nodes;
  nodes.Create (3);
  
  PointToPointHelper p2p1;
  p2p1.SetDeviceAttribute ("DataRate", StringValue ("1Mbps"));
  p2p1.SetChannelAttribute ("Delay", StringValue ("50ms"));
  
  PointToPointHelper p2p2;
  p2p2.SetDeviceAttribute ("DataRate", StringValue ("100kbps"));
  p2p2.SetChannelAttribute ("Delay", StringValue ("5ms"));
  
  NetDeviceContainer device1;
  device1 = p2p1.Install (nodes.Get(0), nodes.Get(1));
  
  NetDeviceContainer device2;
  device2 = p2p2.Install (nodes.Get(1), nodes.Get(2));
  
   InternetStackHelper stack;
  stack.Install (nodes);

  Ipv4AddressHelper address;
  address.SetBase ("10.1.1.0", "255.255.255.0");

  Ipv4InterfaceContainer interface1 = address.Assign (device1);
  Ipv4InterfaceContainer interface2 = address.Assign (device2);
  
   UdpEchoServerHelper echoServer (9);

  ApplicationContainer serverApps = echoServer.Install (nodes.Get (2));
  serverApps.Start (Seconds (1.0));
  serverApps.Stop (Seconds (10.0));
  
  
   UdpEchoClientHelper echoClient (interface2.GetAddress (1), 9);
  echoClient.SetAttribute ("MaxPackets", UintegerValue (6));
  echoClient.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
  echoClient.SetAttribute ("PacketSize", UintegerValue (1024));
  
  ApplicationContainer clientApps = echoClient.Install (nodes.Get (0));
  clientApps.Start (Seconds (2.0));
  clientApps.Stop (Seconds (10.0));
  
  AnimationInterface anim (animFile ); 
	
  Simulator::Run ();
  Simulator::Destroy ();
  return 0;
  
 }

  
  

