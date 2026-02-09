// NAVWARNWrapper.cpp

#include "NAVWARNWrapper.hpp"
#include <qdebug.h>

NAVWARNWrapper::NAVWARNWrapper(NAVWARNPreamble *preamble)
    : S100FeatureWrapper(preamble)
{
    treeItem = buildTreeItem();
}

std::string NAVWARNWrapper::getGMLID() const
{
    return static_cast<NAVWARNPreamble *>(baseData)->getGMLID();
}

bool NAVWARNWrapper::hasGeometry()
{
    const auto &geos = static_cast<NAVWARNPreamble *>(baseData)
                           ->returnPart()
                           ->getGeometries();

    return !geos.empty() && geos.front() != nullptr;
}

bool NAVWARNWrapper::isSurface()
{
    return static_cast<NAVWARNPreamble *>(baseData)->returnPart()->getGeometries()[0]->getPositions().size() > 1;
}

std::vector<DirectPosition *> NAVWARNWrapper::getSurface()
{
    return static_cast<NAVWARNPreamble *>(baseData)->returnPart()->getGeometries()[0]->getPositions();
}

QTreeWidgetItem *NAVWARNWrapper::buildTreeItem()
{
    auto navWarnPreamble = static_cast<NAVWARNPreamble *>(baseData);
    auto affectedChartPublications = navWarnPreamble->getAffectedChartPublications();
    auto generalAreas = navWarnPreamble->getGeneralAreas();
    auto localities = navWarnPreamble->getLocalities();
    auto messageSeriesIdentifier = navWarnPreamble->getMessageSeriesIdentifier();
    auto navwarnTitles = navWarnPreamble->getNavwarnTitles();
    auto cancellationTime = navWarnPreamble->getCancellationDate();
    auto navwarnTypeGeneral = navWarnPreamble->getNavwarnTypeGeneral();
    auto NAVWARNPreambleContent = navWarnPreamble->getNAVWARNPreambleContent();
    auto intService = navWarnPreamble->getIntService();
    auto publicationTime = navWarnPreamble->getPublicationTime();
    auto NAVWARNReferences = navWarnPreamble->getNAVWARNReferences();

    // std::cout << "Generating Simple QTree Item" << '\n';
    QTreeWidgetItem *rootItem = new QTreeWidgetItem();

    rootItem->setData(0, Qt::UserRole, QVariant::fromValue(reinterpret_cast<quintptr>(navWarnPreamble)));

    if (publicationTime)
    {
        rootItem->setText(1, QString::fromStdString(publicationTime->toString()));
    }
    else
    {
        rootItem->setText(1, "");
    }

    if (!generalAreas.empty())
    {
        rootItem->setText(0, QString::fromStdString(generalAreas[0].location.text));
    }

    for (const auto &part : NAVWARNPreambleContent)
    {
        QTreeWidgetItem *navwarnPreambleContentItem = new QTreeWidgetItem(rootItem);
        const auto warningInformation = part->getWarningInformation();

        navwarnPreambleContentItem->setText(0, QString::fromStdString(warningInformation.information.text));

        QString t = QString("<div style='width: 300px;'>%1</div>")
                        .arg(QString::fromStdString(warningInformation.information.text));

        navwarnPreambleContentItem->setToolTip(0, t);

        if (cancellationTime)
        {
            QTreeWidgetItem *cancellationTimeItem = new QTreeWidgetItem(rootItem);
            cancellationTimeItem->setText(0, "Cancellation Date");
            cancellationTimeItem->setText(1, QString::fromStdString(cancellationTime->toString())); // Assuming toString() method exists
        }
    }
    return rootItem;
}
