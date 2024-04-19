#include <iostream>
#include <fstream>

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/csma-module.h"
#include "ns3/applications-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/netanim-module.h"
#include "ns3/mobility-helper.h"
#include "ns3/constant-position-mobility-model.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("CsmaMulticastExample");

int 
main (int argc, char *argv[])
{
std::string someFile = "prac2.xml";
  CommandLine cmd (__FILE__);
  cmd.Parse (argc, argv);

  NS_LOG_INFO ("Create nodes.");
  NodeContainer c;
  c.Create (5);
  
  NodeContainer c0 = NodeContainer (c.Get(0), c.Get(1));
  NodeContainer c1 = NodeContainer (c.Get(1), c.Get(2), c.Get(3), c.Get(4));
  
  CsmaHelper csma;
  csma.SetChannelAttribute ("DataRate", DataRateValue (DataRate (5000000)));
  csma.SetChannelAttribute ("Delay", TimeValue (MilliSeconds (2)));
  
  PointToPointHelper pointToPoint;
  pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
  pointToPoint.SetChannelAttribute ("Delay", StringValue ("2ms"));
  
  NetDeviceContainer nd0 = pointToPoint.Install (c0);  // Second LAN
  NetDeviceContainer nd1 = csma.Install (c1);  // Second LAN
  
  NS_LOG_INFO ("Add IP Stack.");
  InternetStackHelper internet;
  internet.Install (c);
  
  NS_LOG_INFO ("Assign IP Addresses.");
  Ipv4AddressHelper ipv4Addr;
  ipv4Addr.SetBase ("10.1.1.0", "255.255.255.0");
  ipv4Addr.Assign (nd0);
  ipv4Addr.SetBase ("10.1.2.0", "255.255.255.0");
  ipv4Addr.Assign (nd1);
  
  NS_LOG_INFO ("Configure multicasting.");
  //
  // Now we can configure multicasting.  As described above, the multicast 
  // source is at node zero, which we assigned the IP address of 10.1.1.1 
  // earlier.  We need to define a multicast group to send packets to.  This
  // can be any multicast address from 224.0.0.0 through 239.255.255.255
  // (avoiding the reserved routing protocol addresses).
  //

  Ipv4Address multicastSource ("10.1.1.1");
  Ipv4Address multicastGroup ("225.1.2.4");
  
   Ipv4StaticRoutingHelper multicast;

  // 1) Configure a (static) multicast route on node n2 (multicastRouter)
  Ptr<Node> multicastRouter = c.Get (1);  // The node in question
  Ptr<NetDevice> inputIf = nd0.Get (1);  // The input NetDevice
  NetDeviceContainer outputDevices;  // A container of output NetDevices
  outputDevices.Add (nd1.Get (0));  // (we only need one NetDevice here)

  multicast.AddMulticastRoute (multicastRouter, multicastSource, 
                               multicastGroup, inputIf, outputDevices);
  
  Ptr<Node> sender = c.Get (0);
  Ptr<NetDevice> senderIf = nd0.Get (0);
  multicast.SetDefaultMulticastRoute (sender, senderIf);
  
   NS_LOG_INFO ("Create Applications.");

  uint16_t multicastPort = 9;   // Discard port (RFC 863)

  // Configure a multicast packet generator that generates a packet
  // every few seconds
  OnOffHelper onoff ("ns3::UdpSocketFactory", 
                     Address (InetSocketAddress (multicastGroup, multicastPort)));
  onoff.SetConstantRate (DataRate ("255b/s"));
  onoff.SetAttribute ("PacketSize", UintegerValue (128));

  ApplicationContainer srcC = onoff.Install (c0.Get (0));

  //
  // Tell the application when to start and stop.
  //
  srcC.Start (Seconds (1.0));
  srcC.Stop (Seconds (10.0));
  
  PacketSinkHelper sink ("ns3::UdpSocketFactory",
                         InetSocketAddress (Ipv4Address::GetAny (), multicastPort));

  ApplicationContainer sinkC = sink.Install (c1.Get (3)); // Node n4 
  // Start the sink
  sinkC.Start (Seconds (1.0));
  sinkC.Stop (Seconds (10.0));
  
  AsciiTraceHelper ascii;
  csma.EnableAsciiAll(ascii.CreateFileStream("prac2.tr"));
  pointToPoint.EnableAsciiAll(ascii.CreateFileStream("prac2.tr"));
  
  for(uint32_t i = 0; i<c.GetN(); i++){
  	Ptr<Node> node =c.Get(i);
  	Ptr<ConstantPositionMobilityModel> mobility = CreateObject<ConstantPositionMobilityModel>();
  	node->AggregateObject(mobility);
  }
  
  Ptr<ConstantPositionMobilityModel> s0 = c.Get(0)->GetObject<ConstantPositionMobilityModel>();
  Ptr<ConstantPositionMobilityModel> s1 = c.Get(1)->GetObject<ConstantPositionMobilityModel>();
  Ptr<ConstantPositionMobilityModel> s2 = c.Get(2)->GetObject<ConstantPositionMobilityModel>();
  Ptr<ConstantPositionMobilityModel> s3 = c.Get(3)->GetObject<ConstantPositionMobilityModel>();
  Ptr<ConstantPositionMobilityModel> s4 = c.Get(4)->GetObject<ConstantPositionMobilityModel>();
  
  s0->SetPosition(Vector(0,0,0));
  s1->SetPosition(Vector(0, 5, 0));
  s2->SetPosition(Vector(5, 5, 0));
  s3->SetPosition(Vector(10, 5, 0));
  s4->SetPosition(Vector(15, 5, 0));
  
  AnimationInterface anim(someFile);
  

  NS_LOG_INFO ("Run Simulation.");
  Simulator::Run ();
  Simulator::Destroy ();
  NS_LOG_INFO ("Done.");
  
}
