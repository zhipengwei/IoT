#include <iostream>
#include <fstream>

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/bridge-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"

#include "ns3/point-to-point-module.h"
#include "ns3/net-device.h"

#include "ns3/flow-monitor-helper.h"

#include "/home/wzp/workspace/ns-allinone-3.26/ns-3.26/scratch/configOnOff.h"
#include <vector>

using namespace ns3;
using namespace std;
NS_LOG_COMPONENT_DEFINE ("CsmaBridgeExample");

// This is the application defined in another class, this definition will allow us to hook the congestion window.
class MyApp : public Application 
{
public:

  MyApp ();
  virtual ~MyApp();

  void Setup (Ptr<Socket> socket, Address address, uint32_t packetSize, uint32_t nPackets, DataRate dataRate);

private:
  virtual void StartApplication (void);
  virtual void StopApplication (void);

  void ScheduleTx (void);
  void SendPacket (void);

  Ptr<Socket>     m_socket;
  Address         m_peer;
  uint32_t        m_packetSize;
  uint32_t        m_nPackets;
  DataRate        m_dataRate;
  EventId         m_sendEvent;
  bool            m_running;
  uint32_t        m_packetsSent;
};

MyApp::MyApp ()
  : m_socket (0), 
    m_peer (), 
    m_packetSize (0), 
    m_nPackets (0), 
    m_dataRate (0), 
    m_sendEvent (), 
    m_running (false), 
    m_packetsSent (0)
{
}

MyApp::~MyApp()
{
  m_socket = 0;
}

void
MyApp::Setup (Ptr<Socket> socket, Address address, uint32_t packetSize, uint32_t nPackets, DataRate dataRate)
{
  m_socket = socket;
  m_peer = address;
  m_packetSize = packetSize;
  m_nPackets = nPackets;
  m_dataRate = dataRate;
}

void
MyApp::StartApplication (void)
{
  m_running = true;
  m_packetsSent = 0;
  m_socket->Bind ();
  m_socket->Connect (m_peer);
  SendPacket ();
}

void 
MyApp::StopApplication (void)
{
  m_running = false;

  if (m_sendEvent.IsRunning ())
    {
      Simulator::Cancel (m_sendEvent);
    }

  if (m_socket)
    {
      m_socket->Close ();
    }
}

void 
MyApp::SendPacket (void)
{
  Ptr<Packet> packet = Create<Packet> (m_packetSize);
  m_socket->Send (packet);

  if (++m_packetsSent < m_nPackets)
    {
      ScheduleTx ();
    }
}

void 
MyApp::ScheduleTx (void)
{
  if (m_running)
    {
      Time tNext (Seconds (m_packetSize * 8 / static_cast<double> (m_dataRate.GetBitRate ())));
      m_sendEvent = Simulator::Schedule (tNext, &MyApp::SendPacket, this);
    }
}

//static void
//CwndChange (uint32_t oldCwnd, uint32_t newCwnd)
//{
//  NS_LOG_UNCOND ( index << Simulator::Now ().GetSeconds () << "\t" << newCwnd);
//}

static void
CwndChange (std::string context, uint32_t oldCwnd, uint32_t newCwnd)
{
  NS_LOG_UNCOND (context << "\t" << Simulator::Now ().GetSeconds () << "\t" << newCwnd);
}

//static void
//RxDrop (Ptr<const Packet> p)
//{
//  NS_LOG_UNCOND ("RxDrop at " << Simulator::Now ().GetSeconds ());
//}

// The following are queue related tracing functions.
// Packet drop event
static void 
AsciiDropEvent (std::string path, Ptr<const Packet> packet)
{
  NS_LOG_UNCOND ("PacketDrop:\t" << Simulator::Now ().GetSeconds () << "\t" << *packet);
//  cout << "aaa" << endl;
//  *os << "d " << Simulator::Now ().GetSeconds () << " ";
//  *os << path << " " << *packet << std::endl;
}
// Enqueue event
static void 
AsciiEnqueueEvent (std::string path, Ptr<const Packet> packet)
{
  NS_LOG_UNCOND ("PacketArrival\t" << Simulator::Now ().GetSeconds () << "\t" << *packet);
 // *os << "+ " << Simulator::Now ().GetSeconds () << " ";
 // *os << path << " " << *packet << std::endl;
}

static void 
AsciiPacketsInQueue (std::string path, uint32_t oldValue, uint32_t newValue) 
{
  NS_LOG_UNCOND ("PacketsInQueue\t" << Simulator::Now ().GetSeconds () << "\t" << newValue);
 // *os << "+ " << Simulator::Now ().GetSeconds () << " ";
 // *os << path << " " << *packet << std::endl;
}

const string ConvertToStringValue (int val, string name) {
	string tmp;
	if (name.compare (string("constant")) == 0) {
		tmp.assign("ns3::ConstantRandomVariable[Constant=");
	}
	else {
		tmp.assign("ns3::ExponentialRandomVariable[Mean=");
	}
	tmp.append(to_string(val));
	tmp.append("]");
	return tmp;
}

int 
main (int argc, char *argv[])
{
  //
  // Users may find it convenient to turn on explicit debugging
  // for selected modules; the below lines suggest how to do this
  //
#if 0 
  LogComponentEnable ("CsmaBridgeExample", LOG_LEVEL_INFO);
#endif

  //
  // Allow the user to override any of the defaults and the above Bind() at
  // run-time, via command-line arguments
  //
  CommandLine cmd;
  cmd.Parse (argc, argv);

  //
  // Explicitly create the nodes required by the topology (shown above).
  //

  int numberOfTerminals = NUMBER_OF_TERMINALS;
  NS_LOG_INFO ("Create nodes.");
  NodeContainer terminals;
  terminals.Create (numberOfTerminals);

  // 
  NS_LOG_INFO ("Create server node.");
  NodeContainer servers;
  servers.Create (1);

  NodeContainer csmaSwitch;
  csmaSwitch.Create (1);

  NS_LOG_INFO ("Build Topology");
  CsmaHelper csma;
  csma.SetChannelAttribute ("DataRate", DataRateValue (EXPERIMENT_CONFIG_SENDER_LINK_DATA_RATE));
  csma.SetChannelAttribute ("Delay", TimeValue (MilliSeconds (EXPERIMENT_CONFIG_SENDER_LINK_DELAY)));

  CsmaHelper csmaServer;
  // UintegerValue, holds an unsigned integer type.
  csmaServer.SetQueue("ns3::DropTailQueue", "MaxBytes", UintegerValue(EXPERIMENT_CONFIG_BUFFER_SIZE_BYTES), "Mode", EnumValue (DropTailQueue::QUEUE_MODE_BYTES));
  csmaServer.SetChannelAttribute ("DataRate", DataRateValue (EXPERIMENT_CONFIG_SERVER_LINK_DATA_RATE));
  csmaServer.SetChannelAttribute ("Delay", TimeValue (MilliSeconds (EXPERIMENT_CONFIG_SERVER_LINK_DELAY)));

  // Create the csma links, from each terminal to the switch
  NetDeviceContainer terminalDevices;
  NetDeviceContainer switchDevices;

  for (int i = 0; i < numberOfTerminals; i++)
    {
      NetDeviceContainer link = csma.Install (NodeContainer (terminals.Get (i), csmaSwitch));
      terminalDevices.Add (link.Get (0));
      switchDevices.Add (link.Get (1));
    }

  // Create point to point link, from the server to the bridge
  NetDeviceContainer serverDevices;
  NetDeviceContainer linkServer = csmaServer.Install (NodeContainer (servers.Get (0), csmaSwitch));
  serverDevices.Add (linkServer.Get (0));
  switchDevices.Add (linkServer.Get (1));
  ostringstream oss;
  oss << "/NodeList/" << csmaSwitch.Get (0) -> GetId () << "/DeviceList/" << linkServer.Get (1)->GetIfIndex() << "/$ns3::CsmaNetDevice/TxQueue/Enqueue";
  //oss << "/NodeList/" << csmaSwitch.Get (0) -> GetId () << "/DeviceList/4" << "/$ns3::CsmaNetDevice/TxQueue/Enqueue";
  //oss << "/NodeList/" << servers.Get (0)->GetId () << "/$ns3::CsmaNetDevice/TxQueue/Enqueue";
  //oss << "/NodeList/" << "5" << "/DeviceList/" << "4" << "/$ns3::CsmaNetDevice/TxQueue/Enqueue";
  //cout << oss.str() << endl;
  Config::Connect (oss.str(), MakeCallback (&AsciiEnqueueEvent));
  //servers.Get (0)->TraceConnect ("Enqueue", oss.str(), MakeCallback (&AsciiEnqueueEvent));

  oss.str("");
  oss.clear();
  //oss << "/NodeList/" << servers.Get (0)->GetId () << "/DeviceList/" << "4" << "/$ns3::CsmaNetDevice/TxQueue/Drop";
//  oss << "/NodeList/" << "5" << "/DeviceList/" << "4" << "/$ns3::CsmaNetDevice/TxQueue/Drop";
  oss << "/NodeList/" << csmaSwitch.Get (0) -> GetId () << "/DeviceList/" << linkServer.Get (1)->GetIfIndex() << "/$ns3::CsmaNetDevice/TxQueue/Drop";
  //cout << oss.str() << endl;
  Config::Connect (oss.str(), MakeCallback (&AsciiDropEvent));

  oss.str("");
  oss.clear();
  //oss << "/NodeList/" << servers.Get (0)->GetId () << "/DeviceList/" << "4" << "/$ns3::CsmaNetDevice/TxQueue/Drop";
//  oss << "/NodeList/" << "5" << "/DeviceList/" << "4" << "/$ns3::CsmaNetDevice/TxQueue/Drop";
  oss << "/NodeList/" << csmaSwitch.Get (0) -> GetId () << "/DeviceList/" << linkServer.Get (1)->GetIfIndex() << "/$ns3::CsmaNetDevice/TxQueue/PacketsInQueue";
  //cout << oss.str() << endl;
  Config::Connect (oss.str(), MakeCallback (&AsciiPacketsInQueue));

  // Create the bridge netdevice, which will do the packet switching
  Ptr<Node> switchNode = csmaSwitch.Get (0);
  BridgeHelper bridge;
  bridge.Install (switchNode, switchDevices);

  // Add internet stack to the terminals
  InternetStackHelper internet;
  internet.Install (terminals);
  internet.Install (servers);
  // We've got the "hardware" in place.  Now we need to add IP addresses.
  NS_LOG_INFO ("Assign IP Addresses.");
  Ipv4AddressHelper ipv4;
  ipv4.SetBase ("10.1.32.0", "255.255.224.0");
  ipv4.Assign (terminalDevices);
  ipv4.Assign (serverDevices);

  Ipv4InterfaceContainer serverIpv4; 
  serverIpv4.Add(ipv4.Assign (serverDevices));

  // Create a sink application on the server node to receive these applications. 
  // We don't need to modify this.
   uint16_t port = 50000;
   Address sinkLocalAddress (InetSocketAddress (serverIpv4.GetAddress(0), port));
   PacketSinkHelper sinkHelper ("ns3::TcpSocketFactory", sinkLocalAddress);
   ApplicationContainer sinkApp = sinkHelper.Install (servers.Get (0));
   sinkApp.Start (Seconds (1.0));
   sinkApp.Stop (Seconds (10.0));

   //normally wouldn't need a loop here but the server IP address is different
   //on each p2p subnet

   vector<Ptr<Socket> > SocketVector(numberOfTerminals);
   for (vector<Ptr<Socket> >::iterator it = SocketVector.begin(); it < SocketVector.end(); it++) {
     int nodeIndex = it - SocketVector.begin();
     *it = Socket::CreateSocket (terminals.Get (nodeIndex), TcpSocketFactory::GetTypeId ());
     ostringstream oss;
     oss << "/NodeList/" << terminals.Get (nodeIndex)->GetId () << "/$ns3::TcpL4Protocol/SocketList/0/CongestionWindow";
     //cout << oss.str() << endl;
     (*it)->TraceConnect ("CongestionWindow", oss.str(), MakeCallback (&CwndChange));
     //(*it)->TraceConnectWithoutContext ("CongestionWindow", MakeCallback (&CwndChange));
   }

   ApplicationContainer clientApps;
   //vector<Ptr<OnOffApplication> > ApplicationVector(numberOfTerminals);
// Create OnOff applications to send TCP to the hub, one on each spoke node.
   OnOffHelper onOffHelper ("ns3::TcpSocketFactory", Address ());
   onOffHelper.SetAttribute ("OnTime", StringValue (ConvertToStringValue(EXPERIMENT_SENDER_ONTIME_CONSTANT, "constant")));
   onOffHelper.SetAttribute ("OffTime", StringValue (ConvertToStringValue(EXPERIMENT_SENDER_ONTIME_CONSTANT, "exponential")));
   onOffHelper.SetConstantRate (DataRate (160000));

   for(uint32_t i=0; i<terminals.GetN (); ++i)
   {
      AddressValue sinkAddress
         (InetSocketAddress (serverIpv4.GetAddress (0), port));
      onOffHelper.SetAttribute ("Remote", sinkAddress);

      clientApps.Add (onOffHelper.Install (terminals.Get (i)));
      // void Setup (Ptr<Socket> socket, Address address, uint32_t packetSize, uint32_t nPackets, DataRate dataRate);
   }
   clientApps.Start (Seconds (EXPERIMENT_CONFIG_START_TIME));
   clientApps.Stop (Seconds (EXPERIMENT_CONFIG_STOP_TIME));


  NS_LOG_INFO ("Configure Tracing.");

  //
  // Configure tracing of all enqueue, dequeue, and NetDevice receive events.
  // Trace output will be sent to the file "csma-bridge.tr"
  //
  AsciiTraceHelper ascii;
  csma.EnableAsciiAll (ascii.CreateFileStream ("csma-bridge.tr"));

  //
  // Also configure some tcpdump traces; each interface will be traced.
  // The output files will be named:
  //     csma-bridge-<nodeId>-<interfaceId>.pcap
  // and can be read by the "tcpdump -r" command (use "-tt" option to
  // display timestamps correctly)
  //
  csma.EnablePcapAll ("csma-bridge", false);

  //
  // Now, do the actual simulation.
  //
//  Ptr<FlowMonitor> flowMonitor;
//  FlowMonitorHelper flowHelper;
//  //flowMonitor = flowHelper.InstallAll();
//  flowMonitor = flowHelper.Install(servers.Get (0));


  NS_LOG_INFO ("Run Simulation.");
  Simulator::Run ();

//  flowMonitor->SerializeToXmlFile("NameOfFile", false, false);
  Simulator::Destroy ();
  NS_LOG_INFO ("Done.");
}
