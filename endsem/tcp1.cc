/*
* Modified Network Topology:
*
*        n0 ----------- n1 (p2p)
*       p2p \                          
*            \                    
*        	n2 -- n3 ------ n4(csma) 
*                               \
*                                n5 p2p
* Description:
* - n0 and n1 are connected via a point-to-point link.
* - n1, n2, and n3 are connected via CSMA.
* - n3 and n4 are connected via a point-to-point link.
* - n4 and n5 are connected via both CSMA and a point-to-point link.
* n0 - n1 is p2p, n0-n2 is p2p, n2-n3-n4 is csma, n4-n5 is p2p
* source at n0 - receive sink at n5 Trying TCP and UDP packets
*/

#include <string>
#include <fstream>
#include "ns3/core-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"
#include "ns3/network-module.h"
#include "ns3/packet-sink.h"
#include "ns3/csma-module.h"
#include "ns3/netanim-module.h"
#include "ns3/ipv4-global-routing-helper.h"
 
using namespace ns3;
 
NS_LOG_COMPONENT_DEFINE ("TcpBulkSendExample");
 
int
main (int argc, char *argv[])
{
 
  NS_LOG_INFO ("Create nodes.");
  NodeContainer nodes;
  nodes.Create(6);
  NodeContainer p1;
  p1.Add (nodes.Get(0));
  p1.Add (nodes.Get(1));
  NodeContainer p2;
  p2.Add(nodes.Get(0));
  p2.Add(nodes.Get(2));
  NodeContainer csmaNodes;
  csmaNodes.Add(nodes.Get(2)); //n2
  csmaNodes.Add(nodes.Get(3)); 
  csmaNodes.Add(nodes.Get(4)); 
  NodeContainer p3;
  p3.Add(nodes.Get(4));
  p3.Add(nodes.Get(5));
 
//
// Explicitly create the point-to-point link required by the topology (shown above).
//
  PointToPointHelper pointToPoint;
  pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("500Kbps"));
  pointToPoint.SetChannelAttribute ("Delay", StringValue ("5ms"));
  CsmaHelper csma;
  csma.SetChannelAttribute ("DataRate", StringValue ("100Mbps"));
  csma.SetChannelAttribute ("Delay", TimeValue (NanoSeconds (6560)));
  NetDeviceContainer ndp1;
  ndp1 = pointToPoint.Install(p1);
  NetDeviceContainer ndp2;
   ndp2 = pointToPoint.Install(p2);
  NetDeviceContainer ndc1;
  ndc1 = csma.Install(csmaNodes);
  NetDeviceContainer ndp3;
  ndp3 = pointToPoint.Install(p3);
  InternetStackHelper stack;
  stack.Install (nodes);
  Ipv4AddressHelper address;
  address.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer p2pInterfaces1;
  p2pInterfaces1 = address.Assign (ndp1);
 
  address.SetBase ("10.1.2.0", "255.255.255.0");
  Ipv4InterfaceContainer p2pInterfaces2;
  p2pInterfaces2 = address.Assign (ndp2);
  address.SetBase ("10.1.3.0", "255.255.255.0");
  Ipv4InterfaceContainer csmaInterfaces;
  csmaInterfaces = address.Assign (ndc1);
  address.SetBase ("10.1.4.0", "255.255.255.0");
  Ipv4InterfaceContainer p2pInterfaces3;
  p2pInterfaces3 = address.Assign (ndp3);
    uint16_t port = 9;  // well-known echo port number
  BulkSendHelper source ("ns3::TcpSocketFactory",
                         InetSocketAddress (p2pInterfaces3.GetAddress (1), port)); //receiver
  // Set the amount of data to send in bytes.  Zero is unlimited.
  source.SetAttribute ("MaxBytes", UintegerValue (1024));
  ApplicationContainer sourceApps = source.Install (nodes.Get(0)); //sender
  sourceApps.Start (Seconds (0.0));
  sourceApps.Stop (Seconds (10.0));

   PacketSinkHelper sink ("ns3::TcpSocketFactory",
                         InetSocketAddress (p2pInterfaces3.GetAddress(1), port)); //receiver
  ApplicationContainer sinkApps = sink.Install (nodes.Get(5)); //sender
  sinkApps.Start (Seconds (0.0));
  sinkApps.Stop (Seconds (10.0));
   Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  NS_LOG_INFO ("Run Simulation.");
  AnimationInterface anim("AAtcp1.xml");
  
  Simulator::Stop (Seconds (10.0));
  Simulator::Run ();
  Simulator::Destroy ();
  NS_LOG_INFO ("Done.");
 
  Ptr<PacketSink> sink1 = DynamicCast<PacketSink> (sinkApps.Get (0));
  std::cout << "Total Bytes Received: " << sink1->GetTotalRx () << std::endl;

}
