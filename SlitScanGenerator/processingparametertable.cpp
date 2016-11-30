#include "processingparametertable.h"

ProcessingParameterTable::ProcessingParameterTable(QObject *parent)
    : QAbstractTableModel(parent)
{
}

QVariant ProcessingParameterTable::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation==Qt::Horizontal) {
        if (role==Qt::DisplayRole) {
            if (section==0) return tr("mode");
            if (section==1) return tr("x");
            if (section==2) return tr("y");
        }
    } else {
        return section+1;
    }
    return QVariant();
}


int ProcessingParameterTable::rowCount(const QModelIndex &parent) const
{
    return m_data.size();
}

int ProcessingParameterTable::columnCount(const QModelIndex &parent) const
{
    return 3;
}

QVariant ProcessingParameterTable::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    auto mode=m_data.value(index.row(), ProcessingItem()).mode;
    int x=m_data.value(index.row(), ProcessingItem()).location;
    if (role==Qt::DisplayRole || role==Qt::EditRole) {
        if (index.column()==0) {
            if (mode==Mode::XZ) return "XZ";
            if (mode==Mode::ZY) return "ZY";
        } else if (index.column()==1) {
            if (mode==Mode::ZY) return std::max(0, x);
        } else if (index.column()==2) {
            if (mode==Mode::XZ) return std::max(0, x);
        }
    }
    return QVariant();
}

bool ProcessingParameterTable::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (data(index, role) != value) {
        auto mode=m_data.value(index.row(), ProcessingItem()).mode;
        bool ok=false;
        if (role==Qt::DisplayRole || role==Qt::EditRole) {
            if (index.column()==0) {
                if (value.toString().toLower()=="xz" || value.toString().toLower()=="zx") {
                    m_data[index.row()].mode=Mode::XZ;
                    ok=true;
                }
                if (value.toString().toLower()=="yz" || value.toString().toLower()=="zy") {
                    m_data[index.row()].mode=Mode::ZY;
                    ok=true;
                }
            } else if (index.column()==1) {
                if (mode==Mode::ZY) {
                    m_data[index.row()].location=value.toInt();
                    ok=true;
                }
            } else if (index.column()==2) {
                if (mode==Mode::XZ) {
                    m_data[index.row()].location=value.toInt();
                    ok=true;
                }
            }
        }

        if (ok) {
            emit dataChanged(index, index, QVector<int>() << role);
            return true;
        }
    }
    return false;
}

Qt::ItemFlags ProcessingParameterTable::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    return Qt::ItemIsEditable|Qt::ItemIsSelectable|Qt::ItemIsEnabled;
}

bool ProcessingParameterTable::insertRows(int row, int count, const QModelIndex &parent)
{
    beginInsertRows(parent, row, row + count - 1);
    for (int i=0; i<count; i++) {
        m_data.insert(row, ProcessingItem());
    }
    endInsertRows();
    return true;
}

bool ProcessingParameterTable::addItem(const ProcessingParameterTable::ProcessingItem &item)
{
    int row=m_data.size();
    beginInsertRows(QModelIndex(), row, row );
    m_data.append(item);
    endInsertRows();
    return true;
}

bool ProcessingParameterTable::removeRows(int row, int count, const QModelIndex &parent)
{
    beginRemoveRows(parent, row, row + count - 1);
    for (int i=row+count-1; i>=row; i--) {
        m_data.removeAt(i);
    }
    endRemoveRows();
    return true;
}
