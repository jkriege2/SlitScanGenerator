#include "processingparametertable.h"
#include <QFileDialog>
#include <QSettings>
#include <QFileInfo>
#include <QDir>

ProcessingParameterTable::ProcessingParameterTable(QObject *parent)
    : QAbstractTableModel(parent)
{
}

void ProcessingParameterTable::clear()
{
    beginResetModel();
    m_data.clear();
    endResetModel();
}

void ProcessingParameterTable::load(const ProcessingTask& task) {
    beginResetModel();
    m_data=task.pis;
    endResetModel();
}


void ProcessingParameterTable::save(ProcessingTask& task, double xyScaling, double tScaling) const {
    task.pis=m_data;
    for (auto& p: task.pis) {
        p.location_x=p.location_x*xyScaling;
        p.location_y=p.location_y*xyScaling;
        switch (p.angleMode) {
        case ProcessingTask::AngleMode::AnglePitch:
            // pitch angle is corrected, so the (in x/y and t differently reduced) preview-dataset yields the same results as when processing the full dataset.
            // this compensates for invxyFactor/tFactor!=1
            p.angle=atan(tan(p.angle/180.0*M_PI)*xyScaling/tScaling)/M_PI*180.0;
            break;
        case ProcessingTask::AngleMode::AngleNone:
        case ProcessingTask::AngleMode::AngleRoll:
            // for simple roll-angles we do not have to correct, because the angle relates x and y, which are modified with the same factor!
            break;
        }
    }
}


QVariant ProcessingParameterTable::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation==Qt::Horizontal) {
        if (role==Qt::DisplayRole) {
            if (section==0) return tr("Mode");
            if (section==1) return tr("X");
            if (section==2) return tr("Y");
            if (section==3) return tr("Angle Mode");
            if (section==4) return tr("Angle");
        }
    } else {
        return section+1;
    }
    return QVariant();
}

ProcessingTask::ProcessingItem ProcessingParameterTable::getItem(const QModelIndex &idx) const
{
    return m_data[idx.row()];
}


int ProcessingParameterTable::rowCount(const QModelIndex &/*parent*/) const
{
    return m_data.size();
}

int ProcessingParameterTable::columnCount(const QModelIndex &/*parent*/) const
{
    return 5;
}

QVariant ProcessingParameterTable::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    auto mode=m_data.value(index.row(), ProcessingTask::ProcessingItem()).mode;
    auto angle=m_data.value(index.row(), ProcessingTask::ProcessingItem()).angle;
    auto angleMode=m_data.value(index.row(), ProcessingTask::ProcessingItem()).filteredAngleMode();
    int x=m_data.value(index.row(), ProcessingTask::ProcessingItem()).location_x;
    int y=m_data.value(index.row(), ProcessingTask::ProcessingItem()).location_y;
    if (role==Qt::DisplayRole || role==Qt::EditRole) {
        if (index.column()==0) {
            if (mode==ProcessingTask::Mode::XZ) return tr("XZ");
            if (mode==ProcessingTask::Mode::ZY) return tr("ZY");
        } else if (index.column()==1) {
            return x;
        } else if (index.column()==2) {
            return y;
        } else if (index.column()==3) {
            if (role==Qt::EditRole) {
                if (angleMode==ProcessingTask::AngleMode::AngleNone) return 0;
                else if (angleMode==ProcessingTask::AngleMode::AngleRoll) return 0;
                else if (angleMode==ProcessingTask::AngleMode::AnglePitch) return 1;
            } else {
                if (angleMode==ProcessingTask::AngleMode::AngleNone) return "---";
                else if (angleMode==ProcessingTask::AngleMode::AngleRoll) return tr("roll");
                else if (angleMode==ProcessingTask::AngleMode::AnglePitch) return tr("pitch");
            }
        } else if (index.column()==4) {
            if (role==Qt::EditRole) {
                if (angleMode==ProcessingTask::AngleMode::AngleNone) return 0.0;
                else return angle;
            } else {
                if (angleMode==ProcessingTask::AngleMode::AngleNone) return "---";
                else return QString::number(angle, 'f', 1)+QLatin1Char('\xB0');
            }
        }
    }
    return QVariant();
}


Qt::ItemFlags ProcessingParameterTable::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    return Qt::ItemIsSelectable|Qt::ItemIsEnabled;
}

bool ProcessingParameterTable::insertRows(int row, int count, const QModelIndex &parent)
{
    beginInsertRows(parent, row, row + count - 1);
    for (int i=0; i<count; i++) {
        m_data.insert(row, ProcessingTask::ProcessingItem());
    }
    endInsertRows();
    return true;
}

bool ProcessingParameterTable::addItem(const ProcessingTask::ProcessingItem &item)
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
