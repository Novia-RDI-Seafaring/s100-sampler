#include "ATONWrapper.hpp"

ATONWrapper::ATONWrapper(ATONMember *newmember) : S100FeatureWrapper(newmember)
{
    treeItem = buildTreeItem();

}

 std::string ATONWrapper::getGMLID() const {
    return static_cast<ATONMember*>(baseData)->getGMLID();
 }

QTreeWidgetItem *ATONWrapper::buildTreeItem()
{
    //std::cout << "Generating Simple Aton QTree Item" << '\n';
    auto member = static_cast<ATONMember*>(baseData);

    QTreeWidgetItem *rootItem = new QTreeWidgetItem();

    auto atonChanges = member->getStatus()->changeDetails;
    auto atonTypes = member->getStatus()->changeTypes;
    // auto atonDate = member->

    rootItem->setData(0, Qt::UserRole, QVariant::fromValue(reinterpret_cast<quintptr>(member)));
    rootItem->setText(0, QString::fromStdString(member->getName()));
    rootItem->setText(1, QString::fromStdString(member->getDate().toString("XML")));

    // if (!generalAreas.empty())
    // {
    //     rootItem->setText(0, QString::fromStdString(generalAreas[0].location.text));
    // }

    QTreeWidgetItem *newType = new QTreeWidgetItem(rootItem);

    newType->setText(0, QString::fromStdString(atonTypes));
    newType->setText(1, QString::fromStdString(atonChanges));

    //std::cout << "Generation ok" << '\n';

    return rootItem;
}


const Status* ATONWrapper::getStatus()
{
    return static_cast<ATONMember*>(baseData)->getStatus();
}