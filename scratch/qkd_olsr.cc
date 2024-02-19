/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
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
 *
 */

//
// Simple example of OLSR routing over some point-to-point links
//
// Network topology
//
//               ---- n1 ----
// 10 Mb/s, 4ms /            \ 10 Mb/s, 2ms
//             /              \  
//            /  10 Mb/s, 7ms  \   2Mb/s, 10ms   
//           n0 -------------- n3 -------------- n4 
//            \                /
//             \              / 
// 5 Mb/s, 2ms  \            / 5 Mb/s, 7ms
//               ---- n2 ----

// - all links are point-to-point links with indicated one-way BW/delay
// - CBR/UDP flows from n0 to n4, and from n3 to n1
// - UDP packet size of 210 bytes, with per-packet interval 0.00375 sec.
//   (i.e., DataRate of 448,000 bps)
// - DropTail queues
// - Tracing of queues and packet receptions to file "simple-point-to-point-olsr.tr"

#include <iostream>
#include <fstream>
#include <string>
#include <cassert>

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/olsr-helper.h"
#include "ns3/ipv4-static-routing-helper.h"
#include "ns3/ipv4-list-routing-helper.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("QkdOlsrExample");

int
main (int argc, char *argv[])
{
  //
  // Set up logs and environmental variables
  //

#if 1
  LogComponentEnable ("QkdOlsrExample", LOG_LEVEL_INFO, LOG_PREFIX_ALL);
  LogComponentEnable ("QKDManager", LOG_LEVEL_INFO, LOG_PREFIX_ALL);
  LogComponentEnable ("QKDNetDevice", LOG_LEVEL_INFO, LOG_PREFIX_ALL);
  LogComponentEnable ("QKDSend", LOG_LEVEL_INFO, LOG_PREFIX_ALL);
  LogComponentEnable ("QKDBuffer", LOG_LEVEL_INFO, LOG_PREFIX_ALL);
  LogComponentEnable ("QKDCrypto", LOG_LEVEL_WARN, LOG_PREFIX_ALL);
  LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO, LOG_PREFIX_ALL);
  LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO, LOG_PREFIX_ALL);
  LogComponentEnable ("UdpSocketImpl", LOG_LEVEL_INFO, LOG_PREFIX_ALL);
  LogComponentEnable ("UdpL4Protocol", LOG_LEVEL_INFO, LOG_PREFIX_ALL);
  LogComponentEnable ("Ipv4L3Protocol", LOG_LEVEL_INFO, LOG_PREFIX_ALL);
  LogComponentEnable ("PointToPointNetDevice", LOG_LEVEL_INFO, LOG_PREFIX_ALL);
  LogComponentEnable ("PointToPointChannel", LOG_LEVEL_INFO, LOG_PREFIX_ALL);
  LogComponentEnable ("OlsrRoutingProtocol", LOG_LEVEL_INFO, LOG_PREFIX_ALL);
#endif

  // Set up some default values for the simulation.  Use the

  Config::SetDefault ("ns3::OnOffApplication::PacketSize", UintegerValue (210));
  Config::SetDefault ("ns3::OnOffApplication::DataRate", StringValue ("448kb/s"));

  //DefaultValue::Bind ("DropTailQueue::m_maxPackets", 30);

  // Allow the user to override any of the defaults and the above
  // DefaultValue::Bind ()s at run-time, via command-line arguments
  CommandLine cmd;
  cmd.Parse (argc, argv);

  //
  // Create nodes
  //

  NS_LOG_INFO ("Create nodes.");
  NodeContainer c;
  c.Create (5);
  // Create NodeContainer for each connection
  NodeContainer n01 = NodeContainer (c.Get (0), c.Get (1));
  NodeContainer n02 = NodeContainer (c.Get (0), c.Get (2));
  NodeContainer n03 = NodeContainer (c.Get (0), c.Get (3));
  NodeContainer n13 = NodeContainer (c.Get (1), c.Get (3));
  NodeContainer n23 = NodeContainer (c.Get (2), c.Get (3));
  NodeContainer n34 = NodeContainer (c.Get (3), c.Get (4));

  //
  // Create channels
  //

  NS_LOG_INFO ("Create channels and assign IPs to them.");
  PointToPointHelper p2p;
  Ipv4AddressHelper ipv4;

  // Create channel for n01
  p2p.SetDeviceAttribute ("DataRate", StringValue ("10Mbps"));
  p2p.SetChannelAttribute ("Delay", StringValue ("4ms"));
  NetDeviceContainer nd01 = p2p.Install (n01);
  // Assign IP addresses to channel nd01
  ipv4.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer i01 = ipv4.Assign (nd01);
  // Create channel for n02
  p2p.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
  p2p.SetChannelAttribute ("Delay", StringValue ("2ms"));
  NetDeviceContainer nd02 = p2p.Install (n02);
  // Assign IP addresses to channel nd02
  ipv4.SetBase ("10.1.2.0", "255.255.255.0");
  Ipv4InterfaceContainer i02 = ipv4.Assign (nd02);
  // Create channel for n03
  p2p.SetDeviceAttribute ("DataRate", StringValue ("10Mbps"));
  p2p.SetChannelAttribute ("Delay", StringValue ("7ms"));
  NetDeviceContainer nd03 = p2p.Install (n03);
  // Assign IP addresses to channel nd03
  ipv4.SetBase ("10.1.3.0", "255.255.255.0");
  Ipv4InterfaceContainer i03 = ipv4.Assign (nd03);
  // Create channel for n13
  p2p.SetDeviceAttribute ("DataRate", StringValue ("10Mbps"));
  p2p.SetChannelAttribute ("Delay", StringValue ("2ms"));
  NetDeviceContainer nd13 = p2p.Install (n13);
  // Assign IP addresses to channel nd13
  ipv4.SetBase ("10.1.13.0", "255.255.255.0");
  Ipv4InterfaceContainer i13 = ipv4.Assign (nd13);
  // Create channel for n23
  p2p.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
  p2p.SetChannelAttribute ("Delay", StringValue ("7ms"));
  NetDeviceContainer nd23 = p2p.Install (n23);
  // Assign IP addresses to channel nd23
  ipv4.SetBase ("10.1.23.0", "255.255.255.0");
  Ipv4InterfaceContainer i23 = ipv4.Assign (nd23);
  // Create channel for n34
  p2p.SetDeviceAttribute ("DataRate", StringValue ("2Mbps"));
  p2p.SetChannelAttribute ("Delay", StringValue ("10ms"));
  NetDeviceContainer nd34 = p2p.Install (n34);
  // Assign IP addresses to channel nd34
  ipv4.SetBase ("10.1.34.0", "255.255.255.0");
  Ipv4InterfaceContainer i34 = ipv4.Assign (nd34);

  //
  // Enable OLSR
  //
  NS_LOG_INFO ("Enabling OLSR Routing.");
  OlsrHelper olsr;
  InternetStackHelper internet;
  internet.SetRoutingHelper (olsr); // has effect on the next Install ()
  internet.Install (c);

  //
  // Create OnOff Application
  //
  
  NS_LOG_INFO ("Create Applications.");
  uint16_t port = 9;   // Discard port (RFC 863)
  
  // Create the application on n0 to send UDP datagrams 
  // of size 210 bytes at a rate of 448kb/s from n0 to n4
  OnOffHelper onoff1 ("ns3::UdpSocketFactory", InetSocketAddress (i34.GetAddress (1), port));
  onoff1.SetConstantRate (DataRate ("448kb/s"));
  ApplicationContainer onOffApp1 = onoff1.Install (c.Get (0));
  onOffApp1.Start (Seconds (10.0));
  onOffApp1.Stop (Seconds (20.0));

  // Create packet sinks on n4 to receive these packets
  PacketSinkHelper sink ("ns3::UdpSocketFactory",
                         InetSocketAddress (Ipv4Address::GetAny (), port));
  NodeContainer sinks = NodeContainer (c.Get (4), c.Get (1));
  ApplicationContainer sinkApps = sink.Install (sinks);
  sinkApps.Start (Seconds (0.0));
  sinkApps.Stop (Seconds (21.0));

  //
  // Enable Pcap collection
  //
  
  // AsciiTraceHelper ascii;
  // p2p.EnableAsciiAll (ascii.CreateFileStream ("qdk-olsr.tr"));
  // p2p.EnablePcapAll ("qdk-olsr");

  //
  // Run simulation
  //

  Simulator::Stop (Seconds (30));
  NS_LOG_INFO ("Run Simulation.");
  Simulator::Run ();
  Simulator::Destroy ();
  NS_LOG_INFO ("Done.");

  return 0;
}
