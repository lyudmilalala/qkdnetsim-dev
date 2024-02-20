#ifndef QKD_TAG_H
#define QKD_TAG_H

#include "ns3/tag.h"
#include "ns3/packet.h"
#include <iostream>
#include <string>

namespace ns3 {

class QKDTag : public Tag
{
public:
  static TypeId GetTypeId (void);
  virtual TypeId GetInstanceTypeId (void) const;
  virtual uint32_t GetSerializedSize (void) const;
  virtual void Serialize (TagBuffer i) const;
  virtual void Deserialize (TagBuffer i);
  virtual void Print (std::ostream &os) const;

  void SetStringValue (const std::string &value);
  std::string GetStringValue (void) const;

private:
  std::string m_stringValue;
};

} // namespace ns3

#endif /* MY_TAG_H */