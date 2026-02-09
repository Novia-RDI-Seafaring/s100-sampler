// References.hpp

#ifndef REFERENCES_HPP
#define REFERENCES_HPP

#include <string>
#include <vector>
#include "ComplexAttributes.hpp"
#include "ReferenceCategory.hpp"

class References
{
private:
    bool noMessageOnHand;
    ReferenceCategory *referenceCategory;
    std::vector<MessageSeriesIdentifier> messageSeriersIdentifier;

public:
    References();
    void loadMSI(MessageSeriesIdentifier &);
    void setNoMessageOnHand(bool &);
    void setReferenceCategory(int &);
};

#endif // REFERENCES_HPP
