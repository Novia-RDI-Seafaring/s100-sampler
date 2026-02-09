#pragma once

#include <string>

enum class ReferenceCategoryCode
{
    Warning_Cancellation,
    Warning_Reference,
    In_Force
};

/*! 
 * \class ReferenceCategory
 * \brief Represents an S-124 reference category. Not used in the implementation.
 *
 * Provides conversion from int or string to enum representation,
 * and access to a human-readable string.
 */
class ReferenceCategory
{
private:
    ReferenceCategoryCode ReferenceCategoryValue;
    static const char *const referenceCategoryLiterals[3];

public:
    /*!
     * \brief Constructor from integer code.
     */
    ReferenceCategory(int v);
    
     /*!
     * \brief Returns the human readable literal
     */
    std::string getReferenceCategoryLabel();
};