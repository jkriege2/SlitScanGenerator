#ifndef PROCESSINGPARAMETERTABLE_H
#define PROCESSINGPARAMETERTABLE_H

#include <QAbstractTableModel>
#include "processingtask.h"

class ProcessingParameterTable : public QAbstractTableModel {
        Q_OBJECT
    public:
        explicit ProcessingParameterTable(QObject *parent = 0);

        void clear();

        // Header:
        QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

        ProcessingTask::ProcessingItem getItem(const QModelIndex &idx) const;

        // Basic functionality:
        int rowCount(const QModelIndex &parent = QModelIndex()) const override;
        int columnCount(const QModelIndex &parent = QModelIndex()) const override;

        QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

        Qt::ItemFlags flags(const QModelIndex& index) const override;

        // Add data:
        bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
        bool addItem(const ProcessingTask::ProcessingItem& item) ;

        // Remove data:
        bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;

        inline const QVector<ProcessingTask::ProcessingItem>& dataVector() const { return m_data; }
    public slots:
        void save(ProcessingTask &task, double xyScaling=1.0, double tScaling=1.0) const;
        void load(const ProcessingTask &task);
    private:
        QVector<ProcessingTask::ProcessingItem> m_data;
};

#endif // PROCESSINGPARAMETERTABLE_H
