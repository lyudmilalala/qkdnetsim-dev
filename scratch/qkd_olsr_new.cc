/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2015 LIPTEL.ieee.org
 *
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
 * Author: Miralem Mehic <miralem.mehic@ieee.org>
 */


// Network topology
//
//       n0 ---p2p-- n1 --p2p-- n2
//        |---------qkd---------|
//
// - udp flows from n0 to n2

#include <fstream>
#include "ns3/core-module.h" 
#include "ns3/applications-module.h"
#include "ns3/internet-module.h" 
#include "ns3/flow-monitor-module.h" 
#include "ns3/mobility-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/gnuplot.h" 

#include "ns3/qkd-helper.h" 
#include "ns3/qkd-app-charging-helper.h"
#include "ns3/qkd-send.h"

#include "ns3/aodv-module.h" 
#include "ns3/olsr-module.h"
#include "ns3/dsdv-module.h"
 
#include "ns3/network-module.h"
#include "ns3/fd-net-device-module.h"
#include "ns3/internet-apps-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("QkdOlsrExample");
   
uint32_t m_bytes_total = 0; 
uint32_t m_bytes_received = 0; 
uint32_t m_bytes_sent = 0; 
uint32_t m_packets_received = 0; 
double m_time = 0;

void
SentPacket(std::string context, Ptr<const Packet> p){

    m_bytes_sent += p->GetSize();  
}

void
ReceivedPacket(std::string context, Ptr<const Packet> p, const Address& addr){
     
    m_bytes_received += p->GetSize(); 
    m_bytes_total += p->GetSize(); 
    m_packets_received++;

}

void
Ratio(uint32_t m_bytes_sent, uint32_t m_packets_sent ){
    std::cout << "Sent (bytes):\t" <<  m_bytes_sent
    << "\tReceived (bytes):\t" << m_bytes_received 
    << "\nSent (Packets):\t" <<  m_packets_sent
    << "\tReceived (Packets):\t" << m_packets_received 
    
    << "\nRatio (bytes):\t" << (float)m_bytes_received/(float)m_bytes_sent
    << "\tRatio (packets):\t" << (float)m_packets_received/(float)m_packets_sent << "\n";
}

 
int main (int argc, char *argv[])
{

#if 0
  LogComponentEnable ("QkdOlsrExample", LOG_LEVEL_INFO);
//   LogComponentEnable ("QKDManager", LOG_LEVEL_INFO);
  LogComponentEnable ("QKDNetDevice", LOG_LEVEL_INFO);
  LogComponentEnable ("QKDSend", LOG_LEVEL_INFO);
  LogComponentEnable ("QKDBuffer", LOG_LEVEL_INFO);
  LogComponentEnable ("QKDCrypto", LOG_LEVEL_WARN);
  LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
  LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);
  LogComponentEnable ("UdpSocketImpl", LOG_LEVEL_INFO);
  LogComponentEnable ("UdpL4Protocol", LOG_LEVEL_INFO);
  LogComponentEnable ("Ipv4L3Protocol", LOG_LEVEL_INFO);
  LogComponentEnable ("PointToPointNetDevice", LOG_LEVEL_INFO);
  LogComponentEnable ("PointToPointChannel", LOG_LEVEL_INFO);
  LogComponentEnable ("OlsrRoutingProtocol", LOG_LEVEL_INFO);
#endif

    Packet::EnablePrinting(); 
    PacketMetadata::Enable ();

    //
    // Create nodes
    //
    NS_LOG_INFO ("Create nodes.");
    NodeContainer n;
    n.Create (4); 

    // Set Mobility for all nodes
    MobilityHelper mobility;
    Ptr<ListPositionAllocator> positionAlloc = CreateObject <ListPositionAllocator>();
    positionAlloc ->Add(Vector(0, 200, 0)); // node0 
    positionAlloc ->Add(Vector(200, 400, 0)); // node1
    positionAlloc ->Add(Vector(200, 0, 0)); // node2 
    positionAlloc ->Add(Vector(400, 200, 0)); // node3 
    mobility.SetPositionAllocator(positionAlloc);
    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobility.Install(n);

    NodeContainer n01 = NodeContainer (n.Get(0), n.Get (1));
    NodeContainer n02 = NodeContainer (n.Get(0), n.Get (2)); 
    NodeContainer n13 = NodeContainer (n.Get(1), n.Get (3));
    NodeContainer n23 = NodeContainer (n.Get(2), n.Get (3)); 

    //
    // Enable OLSR
    //

    NS_LOG_INFO ("Apply routing alogrithm.");
    //AodvHelper routingProtocol;
    OlsrHelper routingProtocol;
    // DsdvHelper routingProtocol; 
      
    InternetStackHelper internet;
    internet.SetRoutingHelper (routingProtocol);
    internet.Install (n);

    //
    // Create channels
    //

    NS_LOG_INFO ("Create channels and assign IPs to them.");
    PointToPointHelper p2p;
    p2p.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
    p2p.SetChannelAttribute ("Delay", StringValue ("2ms")); 
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
    // // Create channel for n03
    // p2p.SetDeviceAttribute ("DataRate", StringValue ("10Mbps"));
    // p2p.SetChannelAttribute ("Delay", StringValue ("7ms"));
    // NetDeviceContainer nd03 = p2p.Install (n03);
    // // Assign IP addresses to channel nd03
    // ipv4.SetBase ("10.1.3.0", "255.255.255.0");
    // Ipv4InterfaceContainer i03 = ipv4.Assign (nd03);
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

    // 
    //  Create QKD Managers and connections
    // 

    NS_LOG_INFO ("Create QKD Managers and connections on nodes.");
    QKDHelper QHelper;  
    QHelper.InstallQKDManager (n); 
 
    //create QKD connection between nodes 0 and 1 
    NetDeviceContainer qkdNetDevices01 = QHelper.InstallQKD (
        nd01.Get(0), nd01.Get(1),
        1048576,    //min
        11324620,   //thr
        52428800,   //max
        52428800    //current    //20485770
    );
    //Create graph to monitor buffer changes
    // QHelper.AddGraph(n.Get(0), d0d1.Get(0)); //srcNode, destinationAddress, BufferTitle

    //create QKD connection between nodes 1 and 2 
    NetDeviceContainer qkdNetDevices02 = QHelper.InstallQKD (
        nd02.Get(0), nd02.Get(1),
        1048576,    //min
        11324620,   //thr
        52428800,   //max
        52428800    //current    //20485770
    );
    //Create graph to monitor buffer changes
    // QHelper.AddGraph(n.Get(1), d0d1.Get(0)); //srcNode, destinationAddress, BufferTitle

    //create QKD connection between nodes 1 and 3
    NetDeviceContainer qkdNetDevices13 = QHelper.InstallQKD (
        nd13.Get(0), nd13.Get(1),
        1048576,    //min
        11324620,   //thr
        52428800,   //max
        52428800    //current    //20485770
    );

    //create QKD connection between nodes 1 and 3
    NetDeviceContainer qkdNetDevices23 = QHelper.InstallQKD (
        nd23.Get(0), nd23.Get(1),
        1048576,    //min
        11324620,   //thr
        52428800,   //max
        52428800    //current    //20485770
    );

    // 
    //  Create QKD Charging Applications
    // 

    NS_LOG_INFO ("Create QKD Charging Applications.");

    QKDAppChargingHelper qkdChargingHelper01("ns3::TcpSocketFactory", i01.GetAddress(0),  i01.GetAddress(1), 3072000);
    ApplicationContainer qkdChrgApp01 = qkdChargingHelper01.Install ( nd01.Get(0), nd01.Get(1) );
    qkdChrgApp01.Start (Seconds (5.));
    qkdChrgApp01.Stop (Seconds (1500.)); 

    QKDAppChargingHelper qkdChargingHelper02("ns3::TcpSocketFactory", i02.GetAddress(0),  i02.GetAddress(1), 3072000);
    ApplicationContainer qkdChrgApp02 = qkdChargingHelper02.Install ( nd02.Get(0), nd02.Get(1) );
    qkdChrgApp02.Start (Seconds (5.));
    qkdChrgApp02.Stop (Seconds (1500.)); 
    
    QKDAppChargingHelper qkdChargingHelper13("ns3::TcpSocketFactory", i13.GetAddress(0),  i13.GetAddress(1), 3072000);
    ApplicationContainer qkdChrgApp13 = qkdChargingHelper13.Install ( nd13.Get(0), nd13.Get(1) );
    qkdChrgApp13.Start (Seconds (5.));
    qkdChrgApp13.Stop (Seconds (1500.)); 

    QKDAppChargingHelper qkdChargingHelper23("ns3::TcpSocketFactory", i23.GetAddress(0),  i23.GetAddress(1), 3072000);
    ApplicationContainer qkdChrgApp23 = qkdChargingHelper23.Install ( nd23.Get(0), nd23.Get(1) );
    qkdChrgApp23.Start (Seconds (5.));
    qkdChrgApp23.Stop (Seconds (1500.)); 

    // 
    //  Create QKD Send Packets Applications
    // 

    NS_LOG_INFO ("Create QKD Send Packets Applications. Send packets from n0 to n3.");
    
    std::cout << "Source IP address: " << i01.GetAddress(0) << std::endl;
    std::cout << "Destination IP address: " << i13.GetAddress(1) << std::endl;

    // Create sink app
    uint16_t sinkPort = 9090;
    QKDSinkAppHelper packetSinkHelper ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), sinkPort));
    ApplicationContainer sinkApps = packetSinkHelper.Install (n.Get (3));
    sinkApps.Start (Seconds (25.));
    sinkApps.Stop (Seconds (300.));
    
    // Create source app
    Address sourceAddress (InetSocketAddress (i01.GetAddress(0), sinkPort));
    Address sinkAddress (InetSocketAddress (i13.GetAddress(1), sinkPort));
    Ptr<Socket> socket = Socket::CreateSocket (n.Get (0), UdpSocketFactory::GetTypeId ());
    Ptr<QKDSend> app = CreateObject<QKDSend> ();
    app->Setup (socket, sourceAddress, sinkAddress, 640, 5, DataRate ("160kbps"));
    n.Get (0)->AddApplication (app);
    app->SetStartTime (Seconds (25.));
    app->SetStopTime (Seconds (300.));
  
    // 
    //  Run Simulation
    // 

    //if we need we can create pcap files
    p2p.EnablePcapAll ("QKD_channel_test");  

    Config::Connect("/NodeList/*/ApplicationList/*/$ns3::QKDSend/Tx", MakeCallback(&SentPacket));
    Config::Connect("/NodeList/*/ApplicationList/*/$ns3::QKDSink/Rx", MakeCallback(&ReceivedPacket));
 
    Simulator::Stop (Seconds (50));
    Simulator::Run ();

    Ratio(app->sendDataStats(), app->sendPacketStats());
 
    //Finally print the graphs
    // QHelper.PrintGraphs();
    Simulator::Destroy ();
}
