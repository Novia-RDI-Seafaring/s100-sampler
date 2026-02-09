#pragma once

#include <iostream>
#include <vector>
#include "DirectPosition.hpp"


/**
 * \class Geometry
 * \brief Abstract base class for S-100 geometry objects
 */

class Geometry
{
public:
    virtual ~Geometry() = default;
    virtual const std::vector<DirectPosition *> getPositions() const = 0;
    virtual std::string getGeometryType() const = 0;
}
;

/**
 * \class GM_Point
 * \brief S-100 geometric point. 
 * Stored as a vector for some reason.
 */
class GM_Point : public Geometry
{
public:
    GM_Point(double latitude, double longitude);
    GM_Point(DirectPosition *pos);

    const std::vector<DirectPosition *> getPositions() const override;
    std::string getGeometryType() const override;

private:
    DirectPosition *position;
};

/**
 * \class GM_Surface
 * \brief S-100 geometric surface. Patches not implemented, just a list of DirectPositions.
 */
class GM_Surface : public Geometry
{
public:
    int sign = 0;
    GM_Surface(std::vector<DirectPosition *>);
    const std::vector<DirectPosition *> getPositions() const override;
    std::string getGeometryType() const override;

private:
    std::vector<DirectPosition *> positions; // Probably should be patches and each patch has positions
};
