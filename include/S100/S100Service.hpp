#pragma once

#include <QString>
#include <QVector>
#include <QUrl>
#include <QObject>

/*!
 * \struct S100Service
 * \brief A data struct that holds one instance of S100Service. Mostly used for the MSR module
 *
 * \param providerOrgId QString provider id
 * \param orgId QString organization id
 * \param name QString some name
 * \param dataProductType S-100 data product
 * \param endpoint QUrl path to endpoint
 * \param active boolean activity
 */
struct S100Service
{
    QString providerOrgId;
    QString orgId;
    QString name;
    QString dataProductType;
    QUrl endpoint;
    bool active = true;

    bool operator==(const S100Service &other) const
    {
        return orgId == other.orgId &&
               name == other.name &&
               dataProductType == other.dataProductType &&
               endpoint == other.endpoint;
    }
};
Q_DECLARE_METATYPE(S100Service)
Q_DECLARE_METATYPE(QVector<S100Service>)

/*!
 * \struct S100Provider
 * \brief A data struct that holds one instance of a S100Provider. Link to their MSR and and name.
 * Something that is hard coded until some super meta MSR database is created.
 *
 */
struct S100Provider
{
    QString orgId;
    QString name;
    QUrl endpoint;
    int secomVersion;
};
Q_DECLARE_METATYPE(S100Provider)
Q_DECLARE_METATYPE(QVector<S100Provider>)
