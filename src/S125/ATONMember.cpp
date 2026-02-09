#include "ATONMember.hpp"

ATONMember::ATONMember() : status(nullptr) {}

// Load methods
void ATONMember::loadStatus(Status *newStatus)
{
    status.reset(newStatus); // Takes ownership of the Status pointer
}

// Load methods
void ATONMember::pushStatus(Status *newStatus)
{
    statusList.push_back(newStatus);
}

void ATONMember::loadGeometry(Geometry *newGeometry)
{
    geometries.emplace_back(newGeometry); // Takes ownership of the Geometry pointer
}

// Getter methods
const Status *ATONMember::getStatus() const { return status.get(); }
const std::vector<Status *> ATONMember::getStatusList() const { return statusList; }

const std::vector<Geometry *> &ATONMember::getGeometries() const { return geometries; }
std::string ATONMember::getName()
{
    return featureName;
}

void ATONMember::loadName(std::string newName)
{
    featureName = newName;
}

void ATONMember::loadGMLID(std::string newID)
{
    gmlid = newID;
}

void ATONMember::loadDate(Date date)
{
    datasetDate = date;
}

Date ATONMember::getDate()
{
    return datasetDate;
}
const std::string &ATONMember::getGMLID() const
{
    return gmlid;
}

DirectPosition *ATONMember::getFirstPosition() const
{
    return geometries[0]->getPositions()[0];
}
