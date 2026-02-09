// NAVWARNPreamble.cpp

#include "NAVWARNPreamble.hpp"

NAVWARNPreamble::NAVWARNPreamble() {}

void NAVWARNPreamble::loadMSI(MessageSeriesIdentifier MSI)
{
    messageSeriesIdentifier = MSI;
}

void NAVWARNPreamble::loadGA(GeneralArea GA)
{
    generalAreas.push_back(GA);
}

void NAVWARNPreamble::loadNAVWARNType(int code)
{
    navwarnTypeGeneral = new NavwarnTypeGeneral(code);
}

void NAVWARNPreamble::loadNAVWARNType(const std::string &other)
{
    navwarnTypeGeneral = new NavwarnTypeGeneral(other);
}

void NAVWARNPreamble::loadIntService(bool boolean)
{
    intService = boolean;
}

void NAVWARNPreamble::loadCancellationDate(const std::optional<DateTime> &date)
{
    cancellationDate = date;
}

void NAVWARNPreamble::loadPublicationTime(const std::optional<DateTime> &newDateTime)
{
    publicationTime = newDateTime;
}
void NAVWARNPreamble::printPreamble()
{

    std::cout << "Printing Navigational Warning Preamble information: " << '\n';

    messageSeriesIdentifier.printStruct();
    std::cout << "Int service: " << intService << '\n';
    std::cout << "Navigational Warning type: " << navwarnTypeGeneral->getNavwarnTypeGeneralLabel() << '\n';
    std::cout << "Publication time: " << publicationTime->toString() << '\n';
    std::cout << "Printing Navigational Warning Parts: " << '\n';

    for (NAVWARNPart *&part : NAVWARNPreambleContent)
    {
        part->printPart();
    }
};
std::string NAVWARNPreamble::getPreambleData() // Only works for point, redo for general and surfaces
{
    std::string wholeData;
    wholeData += messageSeriesIdentifier.warningType->getWarningTypeLabel() + " at " + generalAreas[0].location.text + '\n';
    wholeData += "Navigational Warning type: " + navwarnTypeGeneral->getNavwarnTypeGeneralLabel() + '\n';
    wholeData += "Publication time: " + publicationTime->toString() + '\n' + '\n';

    for (auto navwarnPart : NAVWARNPreambleContent)
    {

        wholeData += navwarnPart->getWarningInformation().information.text;
        wholeData += " at: " + navwarnPart->getGeometries()[0]->getPositions()[0]->printPosition() + '\n';
        wholeData += '\n';
    }

    return wholeData;
};
void NAVWARNPreamble::appendNewPart(NAVWARNPart *&part)
{
    NAVWARNPreambleContent.push_back(part); // MAKE SURE THIS WORKS
}

void NAVWARNPreamble::appendNewReference(References *&ref)
{
    NAVWARNReferences.push_back(ref);
}

void NAVWARNPreamble::appendNewAreaAffected(NAVWARNAreaAffected *&area)
{
    NAVWARNAreas.push_back(area);
}

void NAVWARNPreamble::printGeometry()
{
    NAVWARNPreambleContent[0]->printGeometry();
}
NAVWARNPart *NAVWARNPreamble::returnPart()
{
    return NAVWARNPreambleContent[0];
}

const std::optional<std::vector<AffectedChartPublications>> &NAVWARNPreamble::getAffectedChartPublications() const
{
    return affectedChartPublications;
}

const std::vector<GeneralArea> &NAVWARNPreamble::getGeneralAreas() const
{
    return generalAreas;
}

const std::optional<std::vector<Locality>> &NAVWARNPreamble::getLocalities() const
{
    return localities;
}

const MessageSeriesIdentifier &NAVWARNPreamble::getMessageSeriesIdentifier() const
{
    return messageSeriesIdentifier;
}

const std::optional<std::vector<NAVWARNtitle>> &NAVWARNPreamble::getNavwarnTitles() const
{
    return navwarnTitles;
}

const std::optional<DateTime> &NAVWARNPreamble::getCancellationDate() const
{
    return cancellationDate;
}

bool NAVWARNPreamble::getIntService() const
{
    return intService;
}

NavwarnTypeGeneral *NAVWARNPreamble::getNavwarnTypeGeneral() const
{
    return navwarnTypeGeneral;
}

const std::optional<DateTime> &NAVWARNPreamble::getPublicationTime() const
{
    return publicationTime;
}

const std::vector<NAVWARNPart *> &NAVWARNPreamble::getNAVWARNPreambleContent() const
{
    return NAVWARNPreambleContent;
}

const std::vector<References *> &NAVWARNPreamble::getNAVWARNReferences() const
{
    return NAVWARNReferences;
}

DirectPosition *NAVWARNPreamble::getFirstPosition() const
{
    return NAVWARNPreambleContent[0]->getFirstPosition();
}

void NAVWARNPreamble::loadGMLID(std::string newID)
{
    gmlid = newID;
}

const std::string &NAVWARNPreamble::getGMLID() const
{
    return gmlid;
}