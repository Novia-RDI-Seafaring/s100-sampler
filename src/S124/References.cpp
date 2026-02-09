// References.cpp

#include "References.hpp"

References::References(){};

void References::loadMSI(MessageSeriesIdentifier &MSI)
{
    messageSeriersIdentifier.push_back(MSI);
}

void References::setNoMessageOnHand(bool &boolean)
{
    noMessageOnHand = boolean;
}

void References::setReferenceCategory(int &code)
{
    referenceCategory = new ReferenceCategory(code);
}
