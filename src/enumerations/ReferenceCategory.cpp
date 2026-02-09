#include "ReferenceCategory.hpp"

// Constructor
ReferenceCategory::ReferenceCategory(int v)
{
    ReferenceCategoryValue = ReferenceCategoryCode(v - 1); // codes are offset by one
}
std::string ReferenceCategory::getReferenceCategoryLabel()
{
    int labelIndex = static_cast<int>(ReferenceCategoryValue); // make a check for going over 12
    return referenceCategoryLiterals[labelIndex];
}

const char *const ReferenceCategory::referenceCategoryLiterals[3] =
    {
        "Warning Cancellation",
        "Warning Reference",
        "In-Force"};