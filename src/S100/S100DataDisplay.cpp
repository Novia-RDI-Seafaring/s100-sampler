#include "S100DataDisplay.hpp"

S100DataDisplay::S100DataDisplay(std::unique_ptr<S100DataLayer> layer,
                                 std::unique_ptr<DataDialog> dialog,
                                 const QString &title,
                                 const QString &buttonLabel)
    : dataLayer(std::move(layer)),
      dialog(std::move(dialog))
{
    this->dialog->setTitle(title);
    this->dialog->setPullButtonTitle(buttonLabel);
    this->dialog->setAttribute(Qt::WA_DeleteOnClose, false);

    connect(this->dialog.get(), &DataDialog::updateButtonClicked, this, &S100DataDisplay::onUpdateButtonClicked);
    connect(this->dialog.get(), &DataDialog::dataItemClicked, this, &S100DataDisplay::onDataItemClicked);
    connect(this->dataLayer.get(), &S100DataLayer::generalError, this, &S100DataDisplay::onDataError);

    std::cout << "display ok" << '\n';
}

S100DataDisplay::~S100DataDisplay()
{
    std::cout << "S100DataDisplay destroyed\n";
}

void S100DataDisplay::onUpdateButtonClicked()
{
    emit updateButtonClicked();
}

void S100DataDisplay::onDataItemClicked(DirectPosition *pos)
{

    emit dataItemClicked(pos);
}

void S100DataDisplay::onDataError(const ServiceError &err)
{
    QWidget *parent = dialog ? static_cast<QWidget *>(dialog.get()) : nullptr;

    QString text;

    if (err.service.name == "" && err.response.statusCode == 0)
    {
        text = QStringLiteral("%1").arg(QString::fromStdString(err.mainMessage));
    }
    else
    {
        text = QStringLiteral("%1 (%2)\nHTTP %3\n%4")
                   .arg(err.service.name,
                        err.service.endpoint.toDisplayString(QUrl::RemoveUserInfo | QUrl::FullyDecoded))
                   .arg(err.response.statusCode)
                   .arg(QString::fromStdString(err.mainMessage));
    }

    QMessageBox::critical(parent, tr("Service error"), text);
}

std::vector<S100FeatureWrapper *> S100DataDisplay::fullReload()
{
    dialog->clearTreeWidget();
    dataLayer->fullRefresh();

    auto undrawnList = dataLayer->getUndrawnFeatures();

    for (auto *undrawn : undrawnList)
    {
        try
        {
            dataLayer->markFeatureAsDrawn(undrawn->getGMLID());
            dialog->pushToTreeWidget(undrawn->getQTreeItem());
        }
        catch (const std::exception &ex)
        {
            std::cerr << "Error processing undrawn " << ": " << ex.what() << std::endl;
        }
    }

    dialog->setUpdateTimeLabel(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
    return undrawnList;
}

std::vector<S100FeatureWrapper *> S100DataDisplay::scanAndDraw()
{
    dataLayer->scanAndLoad();

    auto features = dataLayer->getUndrawnFeatures();
    for (auto *feature : features)
    {

        std::cout << "looping over feature " << feature->getGMLID() << '\n';

        dataLayer->markFeatureAsDrawn(feature->getGMLID());
        dialog->pushToTreeWidget(feature->getQTreeItem());
    }

    dialog->setUpdateTimeLabel(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
    return features;
}
