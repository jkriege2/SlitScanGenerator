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
        p.set_z_step(p.get_z_step()*tScaling);
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
            if (section==colMode) return tr("Mode");
            if (section==colPos) return tr("Pos.");
            if (section==colAngleMode) return tr("Angle Mode");
            if (section==colSlitWidth) return tr("Slit Width");
            if (section==colComposition) return tr("Composition");
            if (section==colZStep) return tr("Z-Step");
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
    return 8;
}

QVariant ProcessingParameterTable::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    const auto mode=m_data.value(index.row(), ProcessingTask::ProcessingItem()).mode;
    const auto addBefore=m_data.value(index.row(), ProcessingTask::ProcessingItem()).addBefore;
    const auto addAfter=m_data.value(index.row(), ProcessingTask::ProcessingItem()).addAfter;
    const auto angle=m_data.value(index.row(), ProcessingTask::ProcessingItem()).angle;
    const auto angleMode=m_data.value(index.row(), ProcessingTask::ProcessingItem()).filteredAngleMode();
    const int x=m_data.value(index.row(), ProcessingTask::ProcessingItem()).location_x;
    const int y=m_data.value(index.row(), ProcessingTask::ProcessingItem()).location_y;
    const int slit_width=m_data.value(index.row(), ProcessingTask::ProcessingItem()).get_slit_width();
    const int z_step=m_data.value(index.row(), ProcessingTask::ProcessingItem()).get_z_step();
    if (role==Qt::DisplayRole || role==Qt::EditRole) {
        if (index.column()==colMode) {
            if (mode==ProcessingTask::Mode::XZ) return tr("XZ");
            if (mode==ProcessingTask::Mode::ZY) return tr("ZY");
        } else if (index.column()==colPos) {
            if (mode==ProcessingTask::Mode::XZ) return y;
            if (mode==ProcessingTask::Mode::ZY) return x;
        } else if (index.column()==colAngleMode) {
            if (angleMode==ProcessingTask::AngleMode::AngleNone) return "---";
            else if (angleMode==ProcessingTask::AngleMode::AngleRoll) return tr("roll")+" ("+QString::number(angle, 'f', 1)+QLatin1Char('\xB0')+")";
            else if (angleMode==ProcessingTask::AngleMode::AnglePitch) return tr("pitch")+" ("+QString::number(angle, 'f', 1)+QLatin1Char('\xB0')+")";
        } else if (index.column()==colSlitWidth) {
            return slit_width;
        } else if (index.column()==colZStep) {
            return z_step;
        } else if (index.column()==colComposition) {
            QString comp=QString::fromUtf8("\u2592\u2592\u2592");
            if (addBefore==ProcessingTask::AddBeforeAfterMode::ToLower) comp=QString::fromUtf8("\u25B2")+comp;
            else if (addBefore==ProcessingTask::AddBeforeAfterMode::ToHigher) comp=QString::fromUtf8("\u25BC")+comp;
            if (addAfter==ProcessingTask::AddBeforeAfterMode::ToLower) comp=comp+QString::fromUtf8("\u25B2");
            else if (addAfter==ProcessingTask::AddBeforeAfterMode::ToHigher) comp=comp+QString::fromUtf8("\u25BC");
            return comp;
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
