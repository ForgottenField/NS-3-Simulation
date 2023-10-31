#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include <string>

using namespace ns3;
using namespace std;

NS_LOG_COMPONENT_DEFINE ("AtcnCaseStudy");

void tracer_CWnd(uint32_t x_old, uint32_t x_new) {
    cout << "TCP_Cwnd" << "," << Simulator::Now().GetNanoSeconds() << "," << x_new << endl;
}

static const uint32_t totalTxBytes = 2000000;
static uint32_t currentTxBytes = 0;
static const uint32_t writeSize = 1040;
uint8_t writedata[writeSize];

void WriteUntilBufferFull (Ptr<Socket> localSocket, uint32_t txSpace);

void StartFlow (Ptr<Socket> localSocket, Ipv4Address servAddress, uint16_t servPort) {
	localSocket->Connect (InetSocketAddress (servAddress, servPort));
	localSocket->SetSendCallback (MakeCallback (&WriteUntilBufferFull));
	WriteUntilBufferFull (localSocket, localSocket->GetTxAvailable ());
}

void WriteUntilBufferFull (Ptr<Socket> localSocket, uint32_t txSpace) {
	while (currentTxBytes < totalTxBytes && localSocket->GetTxAvailable () > 0) {
		uint32_t left = totalTxBytes - currentTxBytes;
		uint32_t dataOffset = currentTxBytes % writeSize;
		uint32_t toWrite = writeSize - dataOffset;
		toWrite = std::min (toWrite, left);
		toWrite = std::min (toWrite, localSocket->GetTxAvailable ());
		int amountSent = localSocket->Send (&writedata[dataOffset], toWrite, 0);
		if(amountSent < 0) {
			return;
		}
		currentTxBytes += amountSent;
	}

	if (currentTxBytes >= totalTxBytes) {
		localSocket->Close ();
	}
}

int
main (int argc, char *argv[])
{
	  CommandLine cmd (__FILE__);
	  cmd.Parse (argc, argv);

	  NodeContainer nodes;
	  nodes.Create (6);

	  PointToPointHelper p2pHelper;
	  p2pHelper.SetDeviceAttribute ("DataRate", StringValue ("10Mbps"));
	  p2pHelper.SetChannelAttribute ("Delay", StringValue ("5ms"));

	  NetDeviceContainer l_0 = p2pHelper.Install(nodes.Get(0), nodes.Get(1));
	  NetDeviceContainer l_1 = p2pHelper.Install(nodes.Get(1), nodes.Get(2));
	  NetDeviceContainer l_2 = p2pHelper.Install(nodes.Get(2), nodes.Get(3));
	  NetDeviceContainer l_3 = p2pHelper.Install(nodes.Get(3), nodes.Get(4));
	  NetDeviceContainer l_4 = p2pHelper.Install(nodes.Get(4), nodes.Get(5));

	  InternetStackHelper stack;
	  stack.Install (nodes);

	  Ipv4AddressHelper address;
	  address.SetBase ("10.1.1.0", "255.255.255.0");
	  Ipv4InterfaceContainer l_0_intf = address.Assign (l_0);
	  address.SetBase ("10.1.2.0", "255.255.255.0");
	  Ipv4InterfaceContainer l_1_intf = address.Assign (l_1);
	  address.SetBase ("10.1.3.0", "255.255.255.0");
	  Ipv4InterfaceContainer l_2_intf = address.Assign (l_2);
	  address.SetBase ("10.1.4.0", "255.255.255.0");
	  Ipv4InterfaceContainer l_3_intf = address.Assign (l_3);
	  address.SetBase ("10.1.5.0", "255.255.255.0");
	  Ipv4InterfaceContainer l_4_intf = address.Assign (l_4);

	  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

	  uint16_t port = 8080;

	  Ptr<Socket> localSocket = Socket::CreateSocket (nodes.Get (0), TcpSocketFactory::GetTypeId ());
	  localSocket->Bind ();

	  Simulator::ScheduleNow (&StartFlow, localSocket, l_4_intf.GetAddress (1), port);

	  PacketSinkHelper sink ("ns3::TcpSocketFactory",
	                         InetSocketAddress (Ipv4Address::GetAny (), port));
	  ApplicationContainer sinkApps = sink.Install (nodes.Get (5));
	  sinkApps.Start (Seconds (0.0));
	  sinkApps.Stop (Seconds (10.0));

	  Config::ConnectWithoutContext ("/NodeList/0/$ns3::TcpL4Protocol/SocketList/0/CongestionWindow", MakeCallback (&tracer_CWnd));

	  p2pHelper.EnablePcapAll("atcn-cs", true);

	  Simulator::Stop (Seconds (10.0));
	  Simulator::Run ();
	  Simulator::Destroy ();

	  return 0;
}
