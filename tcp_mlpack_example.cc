// tcp_mlpack_example.cc
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include <mlpack/core.hpp>
#include <mlpack/methods/linear_regression/linear_regression.hpp>

using namespace ns3;
using namespace mlpack;

NS_LOG_COMPONENT_DEFINE("TcpMlpackExample");

// Callback to log received data
void ReceivePacket(Ptr<Socket> socket, Ptr<LinearRegression> model) {
  Ptr<Packet> packet;
  Address from;
  while ((packet = socket->RecvFrom(from))) {
    // Use mlpack model to predict transmission time
    arma::vec input = {static_cast<double>(packet->GetSize())};
    double predictedTime = model->Predict(input);
    NS_LOG_INFO("Received " << packet->GetSize() << " bytes from " << from
                             << " Predicted Transmission Time: " << predictedTime);
  }
}

int main(int argc, char *argv[]) {
  CommandLine cmd;
  cmd.Parse(argc, argv);

  // Create nodes
  NodeContainer nodes;
  nodes.Create(2);

  // Create point-to-point link
  PointToPointHelper pointToPoint;
  pointToPoint.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
  pointToPoint.SetChannelAttribute("Delay", StringValue("2ms"));

  NetDeviceContainer devices;
  devices = pointToPoint.Install(nodes);

  // Install the internet stack on the nodes
  InternetStackHelper stack;
  stack.Install(nodes);

  // Assign IP addresses
  Ipv4AddressHelper address;
  address.SetBase("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer interfaces = address.Assign(devices);

  // Create a TCP server
  Ptr<Socket> serverSocket = Socket::CreateSocket(nodes.Get(1), TcpSocketFactory::GetTypeId());
  serverSocket->Bind(InetSocketAddress(interfaces.GetAddress(1), 8080));
  serverSocket->Listen();

  // Create a TCP client
  Ptr<Socket> clientSocket = Socket::CreateSocket(nodes.Get(0), TcpSocketFactory::GetTypeId());
  Ptr<MyApp> clientApp = CreateObject<MyApp>();
  clientApp->Setup(clientSocket, InetSocketAddress(interfaces.GetAddress(1), 8080), 1024, 1000000, DataRate("1Mbps"));
  nodes.Get(0)->AddApplication(clientApp);
  clientApp->SetStartTime(Seconds(1.0));
  clientApp->SetStopTime(Seconds(10.0));

  // Install a packet sink on the server
  PacketSinkHelper packetSinkHelper("ns3::TcpSocketFactory", InetSocketAddress(Ipv4Address::GetAny(), 8080));
  ApplicationContainer sinkApps = packetSinkHelper.Install(nodes.Get(1));
  sinkApps.Start(Seconds(0.0));
  sinkApps.Stop(Seconds(10.0));
  sinkApps.Get(0)->TraceConnectWithoutContext("Rx", MakeCallback(&ReceivePacket));

  // Train a simple linear regression model using mlpack
  arma::mat X = arma::randn<arma::mat>(1, 1000); // Example input features (size of packets)
  arma::rowvec y = 2 * X.row(0) + 0.5 + 0.01 * arma::randn<arma::rowvec>(1000); // Example output (transmission time)
  LinearRegression model(X, y);

  // Run simulation
  Simulator::Run();
  Simulator::Destroy();

  return 0;
}
