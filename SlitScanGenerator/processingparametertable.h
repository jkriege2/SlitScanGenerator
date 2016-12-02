#ifndef PROCESSINGPARAMETERTABLE_H
#define PROCESSINGPARAMETERTABLE_H

#include <QAbstractTableModel>

class ProcessingParameterTable : public QAbstractTableModel
{
        Q_OBJECT

    public:
        enum class Mode {
            XZ,
            ZY
        };

        struct ProcessingItem {
            inline ProcessingItem(): mode(Mode::XZ), location(-1) {}
            Mode mode;
            int location;
        };

        explicit ProcessingParameterTable(QObject *parent = 0);

        void clear();

        // Header:
        QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

        ProcessingItem getItem(const QModelIndex &idx) const;

        // Basic functionality:
        int rowCount(const QModelIndex &parent = QModelIndex()) const override;
        int columnCount(const QModelIndex &parent = QModelIndex()) const override;

        QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

        // Editable:
        bool setData(const QModelIndex &index, const QVariant &value,
                     int role = Qt::EditRole) override;

        Qt::ItemFlags flags(const QModelIndex& index) const override;

        // Add data:
        bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
        bool addItem(const ProcessingItem& item) ;

        // Remove data:
        bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;

        const QVector<ProcessingItem>& data() const { return m_data; }
    public slots:
        void load(QString filename=QString(), QString *videoFile=nullptr);
        void save(QString filename, QString videoFile) const;
    private:
        QVector<ProcessingItem> m_data;
};

#endif // PROCESSINGPARAMETERTABLE_H
