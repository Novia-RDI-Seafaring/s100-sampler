// NAVWARNPart.cpp

#include "NAVWARNPart.hpp"

NAVWARNPart::NAVWARNPart() {}

void NAVWARNPart::loadFDR(FixedDateRange &FDR)
{
    fixedDateRanges.push_back(FDR);
}

void NAVWARNPart::loadWI(WarningInformation &WI)
{
    warningInformation = WI;
}

void NAVWARNPart::loadGeometry(Geometry *geo)
{
    geometries.push_back(geo);
}
void NAVWARNPart::printPart()
{

    // std::cout << "Printing NAVWARNPart information: " << '\n';
    // std::cout << "Feature name: " << featureName.name << '\n';
    // std::cout << "Feature reference: " << featureReference.name << '\n';
    // std::cout << "Date start: " << fixedDateRange.dateStart.toString() << '\n';
    std::cout << "Warning information: " << warningInformation.information.text << '\n';
};
void NAVWARNPart::printGeometry()
{
    std::cout << "Printing Geometry information: " << '\n';
    // geometry->print();
}

const std::vector<Geometry *> NAVWARNPart::getGeometries()
{
    return geometries;
}

const std::vector<FeatureName> NAVWARNPart::getFeatureNames()
{
    return featureNames;
}

const std::vector<FeatureReference> NAVWARNPart::getFeatureReferences()
{
    return featureReferences;
}

const std::vector<FixedDateRange> NAVWARNPart::getfixedDateRanges()
{
    return fixedDateRanges;
}

const WarningInformation NAVWARNPart::getWarningInformation()
{
    return warningInformation;
}

DirectPosition *NAVWARNPart::getFirstPosition() const
{
    return geometries[0]->getPositions()[0];
}