#include "S100DataLayer.hpp"
#include "S100FeatureWrapper.hpp"
#include "datadialog.h"
#include "QDateTime"
#include <QObject>
#include <QMessageBox>

class S100DataDisplay : public QObject
{
    Q_OBJECT

public:
    S100DataDisplay(std::unique_ptr<S100DataLayer> layer,
                    std::unique_ptr<DataDialog> dialog,
                    const QString &title,
                    const QString &buttonLabel);
    ~S100DataDisplay();


    /*!
     * \brief Reloads the data display by downloading new data
     * \return A vector of features that are undrawn. To be used in the chart overlay
     */
    std::vector<S100FeatureWrapper *> fullReload();

    DataDialog *getDialog() const
    {
        assert(dialog && "Accessing null dialog");
        return dialog.get();
    }
    S100DataLayer *getDataLayer() const
    {
        assert(dataLayer && "Accessing null dataLayer");
        return dataLayer.get();
    }

    /*!
     * \brief Scans the local data and returns what is found.
     * \return A vector of features that are undrawn. To be used in the chart overlay
     */
    std::vector<S100FeatureWrapper *> scanAndDraw();

signals:
    /*!
     * \brief Emitted when the user selects an item in the tree.
     * \param Pointer to the geographic position object from the selected item.
     */
    void dataItemClicked(DirectPosition *pos);

    /*!
     * \brief Emitted when the user clicks the update/pull button.
     */
    void updateButtonClicked();

private slots:
    void onUpdateButtonClicked();
    void onDataItemClicked(DirectPosition *pos);
    void onDataError(const ServiceError&);


protected:
    std::unique_ptr<S100DataLayer> dataLayer; // QT class. Should be raw.
    std::unique_ptr<DataDialog> dialog;
};
