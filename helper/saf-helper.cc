/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "ns3/names.h"
#include "ns3/uinteger.h"

#include "saf-helper.h"

namespace ns3 {

SafApplicationHelper::SafApplicationHelper(uint16_t port, uint32_t numNodes,
                                           uint16_t numDataitems) {
  NS_ASSERT_MSG(numDataitems % numNodes == 0,
                "Data items MUST be divisable by the number of nodes");
  NS_ASSERT_MSG(numDataitems != 0 && numNodes != 0,
                "Data items and number of nodes can not be zero");

  m_factory.SetTypeId(SafApplication::GetTypeId());
  SetAttribute("Port", UintegerValue(port));
  SetAttribute("NumNodes", UintegerValue(numNodes));
  SetAttribute("TotalDataItems", UintegerValue(numDataitems));
}

void SafApplicationHelper::SetAttribute(std::string name,
                                        const AttributeValue &value) {
  m_factory.Set(name, value);
}

ApplicationContainer SafApplicationHelper::Install(Ptr<Node> node) const {
  return ApplicationContainer(InstallPriv(node));
}

ApplicationContainer SafApplicationHelper::Install(std::string nodeName) const {
  Ptr<Node> node = Names::Find<Node>(nodeName);
  return ApplicationContainer(InstallPriv(node));
}

ApplicationContainer SafApplicationHelper::Install(NodeContainer c) const {
  ApplicationContainer apps;
  for (NodeContainer::Iterator i = c.Begin(); i != c.End(); ++i) {
    apps.Add(InstallPriv(*i));
  }

  return apps;
}

Ptr<Application> SafApplicationHelper::InstallPriv(Ptr<Node> node) const {
  Ptr<Application> app = m_factory.Create<SafApplication>();
  node->AddApplication(app);

  return app;
}
} // namespace ns3
