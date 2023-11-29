#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/udp-echo-helper.h"
#include "ns3/tcp-westwood.h"
#include "ns3/tcp-vegas.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/netanim-module.h"
#include "ns3/mobility-module.h"

using namespace ns3;

static bool firstCwnd = true;
static bool firstSshThr = true;
static Ptr<OutputStreamWrapper> cWndStream;
static Ptr<OutputStreamWrapper> ssThreshStream;
static uint32_t cWndValue;
static uint32_t ssThreshValue;

void PrintRoutingTable(Ptr<Ipv4> ipv4) {
    Ipv4StaticRoutingHelper helper;
    Ptr<Ipv4StaticRouting> staticRouting = helper.GetStaticRouting(ipv4);

    for (uint32_t i = 0; i < staticRouting->GetNRoutes(); ++i) {
        Ipv4RoutingTableEntry entry;
        entry = staticRouting->GetRoute(i);
        std::cout << "Destination: " << entry.GetDest() << ", Gateway: " << entry.GetGateway()
                  << ", Interface: " << entry.GetInterface() << std::endl;
    }
}

static void
CwndTracer (uint32_t oldval, uint32_t newval)
{
  if (firstCwnd)
    {
      *cWndStream->GetStream () << "0.0 " << oldval << std::endl;
      firstCwnd = false;
    }
  *cWndStream->GetStream () << Simulator::Now ().GetSeconds () << " " << newval << std::endl;
  cWndValue = newval;

  if (!firstSshThr)
    {
      *ssThreshStream->GetStream () << Simulator::Now ().GetSeconds () << " " << ssThreshValue << std::endl;
    }
}

static void
SsThreshTracer (uint32_t oldval, uint32_t newval)
{
  if (firstSshThr)
    {
      *ssThreshStream->GetStream () << "0.0 " << oldval << std::endl;
      firstSshThr = false;
    }
  *ssThreshStream->GetStream () << Simulator::Now ().GetSeconds () << " " << newval << std::endl;
  ssThreshValue = newval;

  if (!firstCwnd)
    {
      *cWndStream->GetStream () << Simulator::Now ().GetSeconds () << " " << cWndValue << std::endl;
    }
}

static void
TraceCwnd (std::string cwnd_tr_file_name)
{
  AsciiTraceHelper ascii;
  cWndStream = ascii.CreateFileStream (cwnd_tr_file_name.c_str ());
  Config::ConnectWithoutContext ("/NodeList/0/$ns3::TcpL4Protocol/SocketList/0/CongestionWindow", MakeCallback (&CwndTracer));
}

static void
TraceSsThresh (std::string ssthresh_tr_file_name)
{
  AsciiTraceHelper ascii;
  ssThreshStream = ascii.CreateFileStream (ssthresh_tr_file_name.c_str ());
  Config::ConnectWithoutContext ("/NodeList/0/$ns3::TcpL4Protocol/SocketList/0/SlowStartThreshold", MakeCallback (&SsThreshTracer));
}

int 
main (int argc, char *argv[]) 
{
    // Define a logging component
    NS_LOG_COMPONENT_DEFINE("MyTcpApplicationExample"); 

    // Enable logging for specific modules or classes
    //LogComponentEnable("OnOffApplication", LOG_LEVEL_INFO);
    //LogComponentEnable("PacketSink", LOG_LEVEL_INFO);
    //PacketMetadata::Enable();
    //LogComponentEnable("AnimationInterface", LOG_LEVEL_LOGIC);
    //LogComponentEnable("Socket", LOG_LEVEL_FUNCTION);
    
    // Command-line arguments
    uint32_t udpRateMbps = 10;   // UDP sending rate in Mbps
    uint32_t tcpRateMbps = 20;   // TCP sending rate in Mbps
    uint32_t bufferSize = 16; // TCP buffer size
    uint32_t routerBufferSize = 16;
    // MTU refers to payload in Data Link Layer, while MSS refers to payload in Transport Layer
    // MTU = MSS + 20 + 20
    uint32_t mtu_bytes = 400;
    bool sack = true;
    bool tcpNoDelay = true;
    std::string recovery = "ns3::TcpClassicRecovery";
    std::string congestion_control_algo = "TcpNewReno";
    bool tracing = true;
    bool pcap = false;
    bool monitor = false;
    std::string file_name_prefix = "NS-3-Simulation/experiment/04_result/NewReno/ATCN-Program";
    std::string animFile = "ATCN_animation.xml" ;  // Name of file for animation output

    // Add hooks to the command line system
    CommandLine cmd;
    cmd.AddValue ("udpRate", "UDP sending rate in Mbps", udpRateMbps); 
    cmd.AddValue ("tcpRate", "TCP sending rate in Mbps", tcpRateMbps);
    cmd.AddValue ("tcpBufferSize", "TCP buffer size", bufferSize);
    cmd.AddValue ("routerBufferSize", "Router buffer size", routerBufferSize);
    cmd.AddValue ("mtu", "Size of IP packets to send in bytes", mtu_bytes);
    cmd.AddValue ("sack", "Enable or disable SACK option", sack);
    cmd.AddValue ("recovery", "Recovery algorithm type to use (e.g., ns3::TcpPrrRecovery", recovery);
    cmd.AddValue ("tracing", "Flag to enable/disable tracing", tracing);
    cmd.AddValue ("congestion_control_algo", "Transport protocol to use: TcpNewReno, TcpLinuxReno, "
                "TcpHybla, TcpHighSpeed, TcpHtcp, TcpVegas, TcpScalable, TcpVeno, "
                "TcpBic, TcpYeah, TcpIllinois, TcpWestwood, TcpWestwoodPlus, TcpLedbat, "
		        "TcpLp, TcpDctcp, TcpCubic, TcpBbr", congestion_control_algo);
    cmd.AddValue ("animFile",  "File Name for Animation Output", animFile);
    cmd.Parse (argc, argv);

    congestion_control_algo = std::string ("ns3::") + congestion_control_algo;

    // Create nodes
    NodeContainer TCPHost, UDPHost, routers, TCPServer, UDPSink;
    TCPHost.Create(1);
    UDPHost.Create(1);
    routers.Create(4);
    TCPServer.Create(1);
    UDPSink.Create(1);

    // Define and configure point-to-point links between nodes as topology
    PointToPointHelper p2pHelper;
	  p2pHelper.SetDeviceAttribute ("DataRate", StringValue ("10Mbps"));
	  p2pHelper.SetChannelAttribute ("Delay", StringValue ("5ms"));

    // Configure point-to-point NetDevice
    NetDeviceContainer l_0 = p2pHelper.Install(TCPHost.Get(0), routers.Get(0));     // tcphost   --    R1
	  NetDeviceContainer l_1 = p2pHelper.Install(routers.Get(0), routers.Get(1));     //    R1     --    R2
	  NetDeviceContainer l_2 = p2pHelper.Install(routers.Get(1), routers.Get(2));     //    R2     --    R3
	  NetDeviceContainer l_3 = p2pHelper.Install(routers.Get(2), routers.Get(3));     //    R3     --    R4
	  NetDeviceContainer l_4 = p2pHelper.Install(routers.Get(3), TCPServer.Get(0));   //    R4     --  tcpserver
    NetDeviceContainer l_5 = p2pHelper.Install(UDPHost.Get(0), routers.Get(1));     // udphost   --    R2
    NetDeviceContainer l_6 = p2pHelper.Install(routers.Get(2), UDPSink.Get(0));     //    R3     --   udpsink

    // Set the queuing buffer size of routers from the NetDeviceContainer l_2
    Ptr<PointToPointNetDevice> p2pNetDevice = DynamicCast<PointToPointNetDevice>(l_2.Get(0));
    p2pNetDevice->SetQueue(new DropTailQueue<Packet> ());
    Ptr<Queue<Packet>> queue = p2pNetDevice->GetQueue();
    queue->SetMaxSize(std::to_string(1 << routerBufferSize) + "B");
    std::cout << queue->GetMaxSize() << "\n";

    // Install Internet Stack
    InternetStackHelper stack;
    stack.Install (TCPHost);
    stack.Install (UDPHost);
    stack.Install (routers);
    stack.Install (TCPServer);
    stack.Install (UDPSink);

    // Assign IP Addresses
    Ipv4AddressHelper address;
    address.SetBase ("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer l_0_intf = address.Assign (l_0);  //10.1.1.1 - 10.1.1.2
    address.SetBase ("10.1.2.0", "255.255.255.0");
    Ipv4InterfaceContainer l_1_intf = address.Assign (l_1);  //10.1.2.1 - 10.1.2.2
    address.SetBase ("10.1.3.0", "255.255.255.0");
    Ipv4InterfaceContainer l_2_intf = address.Assign (l_2);  //10.1.3.1 - 10.1.3.2
    address.SetBase ("10.1.4.0", "255.255.255.0");
    Ipv4InterfaceContainer l_3_intf = address.Assign (l_3);  //10.1.4.1 - 10.1.4.2
    address.SetBase ("10.1.5.0", "255.255.255.0");
    Ipv4InterfaceContainer l_4_intf = address.Assign (l_4);  //10.1.5.1 - 10.1.5.2
    address.SetBase ("10.1.6.0", "255.255.255.0");
    Ipv4InterfaceContainer l_5_intf = address.Assign (l_5);  //10.1.6.1 - 10.1.6.2
    address.SetBase ("10.1.7.0", "255.255.255.0");
    Ipv4InterfaceContainer l_6_intf = address.Assign (l_6);  //10.1.7.1 - 10.1.7.2

    // Print IP addresses of interface l_0_intf
    for (uint32_t i = 0; i < l_0_intf.GetN(); ++i) {
        Ipv4Address addr = l_0_intf.GetAddress(i);
        std::cout << "Interface " << i << " IP Address: " << addr << std::endl;
    }

    for (uint32_t i = 0; i < l_6_intf.GetN(); ++i) {
        Ipv4Address addr = l_6_intf.GetAddress(i);
        std::cout << "Interface " << i << " IP Address: " << addr << std::endl;
    }

    // Configure routing paths on routers
    Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

    // Print the routing table for a specific router (e.g., node 0)
    uint32_t nodeId = 0; // Replace with the node ID of the router you want to inspect
    Ptr<Ipv4> ipv4 = routers.Get(nodeId)->GetObject<Ipv4>();
    PrintRoutingTable(ipv4);

    uint16_t tcp_server_port = 8080;

    // Configure the TCP stack
    Config::SetDefault ("ns3::TcpSocket::RcvBufSize", UintegerValue (1 << bufferSize));
    Config::SetDefault ("ns3::TcpSocket::SndBufSize", UintegerValue (1 << bufferSize));
    Config::SetDefault ("ns3::TcpSocket::SegmentSize", UintegerValue (mtu_bytes - 20 - 20));
    // Turn off Nagle's Algorithm
    Config::SetDefault ("ns3::TcpSocket::TcpNoDelay", BooleanValue (tcpNoDelay));
    Config::SetDefault ("ns3::TcpSocketBase::Sack", BooleanValue (sack));
    Config::SetDefault ("ns3::TcpL4Protocol::RecoveryType", TypeIdValue (TypeId::LookupByName (recovery)));
    Config::SetDefault ("ns3::OnOffApplication::PacketSize", UintegerValue (512));
    
    // Select TCP Congestion Control Algorithms
    if (congestion_control_algo.compare ("ns3::TcpWestwoodPlus") == 0)
    {
        // TcpWestwoodPlus is not an actual TypeId name; we need TcpWestwood here
        Config::SetDefault ("ns3::TcpL4Protocol::SocketType", TypeIdValue (TcpWestwood::GetTypeId ()));
        // the default protocol type in ns3::TcpWestwood is WESTWOOD
        Config::SetDefault ("ns3::TcpWestwood::ProtocolType", EnumValue (TcpWestwood::WESTWOODPLUS));
    }
    else
    {
        TypeId tcpTid;
        NS_ABORT_MSG_UNLESS (TypeId::LookupByNameFailSafe (congestion_control_algo, &tcpTid), "TypeId " << congestion_control_algo << " not found");
        Config::SetDefault ("ns3::TcpL4Protocol::SocketType", TypeIdValue (TypeId::LookupByName (congestion_control_algo)));
    }

    float time_to_stop_data = 5.0;

    // Create a TCP sender (OnOffApplication)
    OnOffHelper tcpOnOffHelper("ns3::TcpSocketFactory", Address());
    tcpOnOffHelper.SetAttribute("OnTime", StringValue("ns3::ConstantRandomVariable[Constant=1]"));
    tcpOnOffHelper.SetAttribute("OffTime", StringValue("ns3::ConstantRandomVariable[Constant=0]"));
    tcpOnOffHelper.SetAttribute("DataRate", DataRateValue(DataRate(tcpRateMbps * 1000000)));

    // Set the server address to the destination's IP address and port
    Ipv4Address serverAddress = TCPServer.Get(0)->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal();
    InetSocketAddress serverSocketAddress(serverAddress, tcp_server_port);
    tcpOnOffHelper.SetAttribute("Remote", AddressValue(serverSocketAddress));

    // Install the OnOffApplication on the source node
    ApplicationContainer tcpSourceApps = tcpOnOffHelper.Install(TCPHost);

    // Print the basic attributes of TCP socket
    std::cout << std::endl << "Print the basic attributes of TCP socket as follows:" << std::endl;
    TypeId tid = TcpSocket::GetTypeId();
    for(size_t i = 0; i < tid.GetAttributeN(); ++i)
    {
      TypeId::AttributeInformation attr = tid.GetAttribute(i);
      Ptr<const AttributeAccessor> accessor = attr.accessor;
      std::cout << "attr" << i << ":" << std::endl;
      std::cout << "name: " << attr.name << std::endl;
      std::cout << "flags: " << attr.flags << std::endl;
      std::cout << "originalInitialValue: " << attr.originalInitialValue->SerializeToString(attr.checker) << std::endl;
      std::cout << "initialValue: " << attr.initialValue->SerializeToString(attr.checker) << std::endl;
      std::cout << std::endl;
    }

    // Start the applications
    tcpSourceApps.Start(Seconds(0.0));
    NS_LOG_UNCOND("TcpOnOffApplication started");

    // Stop the applications
    tcpSourceApps.Stop(Seconds(time_to_stop_data));
    NS_LOG_UNCOND("TcpOnOffApplication stopped");

    // Create server and client applications
    PacketSinkHelper tcpSinkHelper("ns3::TcpSocketFactory", InetSocketAddress(Ipv4Address::GetAny(), tcp_server_port));
    ApplicationContainer tcpSinkApps = tcpSinkHelper.Install(TCPServer); // Install on a specific node
    tcpSinkApps.Start (Seconds(0.0));

    // Install UDP applications
    // Create a UDP echo server
    uint16_t udp_server_port = 9;

    OnOffHelper udpOnOffHelper("ns3::UdpSocketFactory", Address());
    udpOnOffHelper.SetAttribute("OnTime", StringValue("ns3::ConstantRandomVariable[Constant=1]"));
    udpOnOffHelper.SetAttribute("OffTime", StringValue("ns3::ConstantRandomVariable[Constant=0]"));
    udpOnOffHelper.SetAttribute("DataRate", DataRateValue(DataRate(udpRateMbps * 1000000)));

    Ipv4Address udpServerAddress = UDPSink.Get(0)->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal();
    InetSocketAddress udpServerSocketAddress(udpServerAddress, udp_server_port);
    udpOnOffHelper.SetAttribute("Remote", AddressValue(udpServerSocketAddress));

    ApplicationContainer udpSourceApps = udpOnOffHelper.Install(UDPHost); 
    udpSourceApps.Start(Seconds(0.0));
    NS_LOG_UNCOND("UdpOnOffApplication started");
    udpSourceApps.Stop(Seconds(time_to_stop_data));
    NS_LOG_UNCOND("UdpOnOffApplication stopped");

    PacketSinkHelper udpSinkHelper("ns3::UdpSocketFactory", InetSocketAddress(Ipv4Address::GetAny(), udp_server_port));
    ApplicationContainer udpSinkApps = udpSinkHelper.Install(UDPSink); // Install on a specific node
    udpSinkApps.Start (Seconds(0.0));

    MobilityHelper mobility;
    mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
    mobility.Install (TCPHost);
    mobility.Install (TCPServer);
    mobility.Install (routers);
    mobility.Install (UDPHost);
    mobility.Install (UDPSink);

    // Create the animation object and configure for specified output
    AnimationInterface anim (animFile);
    anim.SetMaxPktsPerTraceFile(0xFFFFFF);
    anim.SetMobilityPollInterval (Seconds (1));
    anim.EnablePacketMetadata(true); // Enable packet animation
    anim.EnableIpv4RouteTracking("routingtable-experiment-0.csv", Seconds(0), Seconds(10));

    // Set TCP nodes as red 
    anim.UpdateNodeDescription (TCPHost.Get (0), "TCPHost"); 
    anim.UpdateNodeColor (TCPHost.Get (0), 255, 0, 0);
    anim.UpdateNodeDescription (TCPServer.Get (0), "TCPServer"); 
    anim.UpdateNodeColor (TCPServer.Get (0), 255, 0, 0);
    
    // Set udp nodes as green
    anim.UpdateNodeDescription (UDPHost.Get (0), "UDPHost"); 
    anim.UpdateNodeColor (UDPHost.Get (0), 0, 255, 0);
    anim.UpdateNodeDescription (UDPSink.Get (0), "UDPSink"); 
    anim.UpdateNodeColor (UDPSink.Get (0), 0, 255, 0);

    // Set router nodes as blue
    for (uint32_t i = 0; i < routers.GetN (); ++i)
    {
      anim.UpdateNodeDescription (routers.Get (i), "router"); 
      anim.UpdateNodeColor (routers.Get (i), 0, 0, 255); 
    }

    // Set constant position of each node
    Ptr<ConstantPositionMobilityModel> s1 = TCPHost.Get (0)->GetObject<ConstantPositionMobilityModel> ();
    Ptr<ConstantPositionMobilityModel> s2 = TCPServer.Get (0)->GetObject<ConstantPositionMobilityModel> ();
    s1->SetPosition (Vector ( 0.0, 150.0, 0 ));
    s2->SetPosition (Vector ( 500.0, 150.0, 0 ));

    Ptr<ConstantPositionMobilityModel> s3 = UDPHost.Get (0)->GetObject<ConstantPositionMobilityModel> ();
    Ptr<ConstantPositionMobilityModel> s4 = UDPSink.Get (0)->GetObject<ConstantPositionMobilityModel> ();
    s3->SetPosition (Vector ( 200.0, 50.0, 0 ));
    s4->SetPosition (Vector ( 300.0, 250.0, 0 ));

    for (uint32_t i = 0; i < routers.GetN (); ++i)
    {
      Ptr<ConstantPositionMobilityModel> s = routers.Get (i)->GetObject<ConstantPositionMobilityModel> ();
      s->SetPosition (Vector ( 0.0 + 100.0 * (i + 1), 150.0, 0 ));
    }

    // Monitor the flows
    Ptr<FlowMonitor> flowMonitor;
    FlowMonitorHelper flowHelper;
    flowMonitor = flowHelper.InstallAll();

    // Set up tracing if enabled
    if (tracing)
    {
        //std::ofstream ascii;
        //Ptr<OutputStreamWrapper> ascii_wrap;
        //ascii.open ((file_name_prefix + "-ascii").c_str ());
        //ascii_wrap = new OutputStreamWrapper ((file_name_prefix + "-ascii").c_str (), std::ios::out);
        //stack.EnableAsciiIpv4All (ascii_wrap);

        Simulator::Schedule (Seconds (0.00001), &TraceCwnd, file_name_prefix + "-cwnd.txt");
        Simulator::Schedule (Seconds (0.00001), &TraceSsThresh, file_name_prefix + "-ssth.txt");
    }

    if(pcap){
        p2pHelper.EnablePcapAll(file_name_prefix, true);
    }

    // Configure and run the simulation
    Simulator::Stop (Seconds (10.0));
    Simulator::Run ();

    if(monitor)
    {
        // Check for lost packets and serialize flow data to XML
        flowMonitor->CheckForLostPackets();
        flowMonitor->SerializeToXmlFile("flow-monitor.xml", true, true);

        Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier>(flowHelper.GetClassifier());
        FlowMonitor::FlowStatsContainer stats = flowMonitor->GetFlowStats();

        // Open a file for writing
        std::ofstream outputFile(file_name_prefix + "-MTU.txt", std::ios::app);
        outputFile << "TCP_rate: " << tcpRateMbps << "Mbps" << std::endl;
        outputFile << "UDP_rate: " << udpRateMbps << "Mbps" << std::endl;
        outputFile << "tcp_buffer_size: " << (1 << bufferSize) << "bytes" << std::endl;
        outputFile << "router_buffer_size: " << (1 << routerBufferSize) << "bytes" << std::endl;
        outputFile << "MTU: " << mtu_bytes << "bytes" << std::endl;
        outputFile << "Nagle Algorithm: " << tcpNoDelay << std::endl;
        for (auto it = stats.begin(); it != stats.end(); ++it) 
        {
            Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow(it->first);
            outputFile << "Flow ID: " << it->first << std::endl;
            outputFile << "Source IP: " << t.sourceAddress << "  Destination IP: " << t.destinationAddress << std::endl;
            outputFile << "Throughput: " << it->second.txBytes * 8.0 / time_to_stop_data / 1000 / 1000 << " Mbps" << std::endl;
            outputFile << "Transmission Delay: " << it->second.delaySum / it->second.txPackets << std::endl;
            outputFile << "Packet Loss Rate: " << (1.0 - (it->second.rxPackets * 1.0) / it->second.txPackets) << std::endl;
            if(it->second.txPackets <= 1)
            {
                outputFile << "Jitter: " <<  0  << std::endl;
            }
            else
            {
                outputFile << "Jitter: " << it->second.jitterSum / (it->second.txPackets - 1) << std::endl;
            }
        }
        outputFile << std::endl;
        outputFile.close();
    }
    
    // Cleanup and destroy the simulation
    tcpSinkApps.Stop (Seconds (10.0));
    udpSinkApps.Stop (Seconds(10.0));
    Simulator::Destroy ();
    
    return 0;
}
