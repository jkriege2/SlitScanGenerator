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

void ProcessingParameterTable::load(QString filename, QString* videoFile)
{
    beginResetModel();
    m_data.clear();
    // QFileDialog::getOpenFileName(this, tr("Open Configuration File ..."), "", tr("INI-File (*.ini)"));
    QSettings setall(filename, QSettings::IniFormat);
    int cnt=setall.value("count", 0).toInt();
    if (videoFile) {
        *videoFile=setall.value("input_file", "").toString();
        if (videoFile->size()>0) {
            *videoFile=QFileInfo(filename).absoluteDir().absoluteFilePath(*videoFile);
        }
    }
    for (int j=0; j<cnt; j++) {
        ProcessingTask::ProcessingItem pi;
        pi.location_x=setall.value(QString("item%1/location_x").arg(j,3,10,QChar('0')), 0).toInt();
        pi.location_y=setall.value(QString("item%1/location_y").arg(j,3,10,QChar('0')), 0).toInt();
        pi.angle=setall.value(QString("item%1/angle").arg(j,3,10,QChar('0')), 0).toDouble();
        pi.angleMode=static_cast<ProcessingTask::AngleMode>(setall.value(QString("item%1/angle_mode").arg(j,3,10,QChar('0')), 0).toInt());
        if (setall.value(QString("item%1/mode").arg(j,3,10,QChar('0')), "").toString()=="ZY") {
            pi.mode=ProcessingTask::Mode::ZY;
            m_data.push_back(pi);
        } else if (setall.value(QString("item%1/mode").arg(j,3,10,QChar('0')), "").toString()=="XZ") {
            pi.mode=ProcessingTask::Mode::XZ;
            m_data.push_back(pi);
        }
    }
    endResetModel();
}

void ProcessingParameterTable::save(QString filename, QString videoFile) const
{
    // QFileDialog::getOpenFileName(this, tr("Open Configuration File ..."), "", tr("INI-File (*.ini)"));
    QSettings setall(filename, QSettings::IniFormat);
    setall.setValue("count", m_data.size());
    if (videoFile.size()>0) setall.value("input_file", QFileInfo(filename).absoluteDir().relativeFilePath(videoFile));

    for (int j=0; j<m_data.size(); j++) {
        ProcessingTask::ProcessingItem pi=m_data[j];
        setall.setValue(QString("item%1/location_x").arg(j,3,10,QChar('0')), pi.location_x);
        setall.setValue(QString("item%1/location_y").arg(j,3,10,QChar('0')), pi.location_y);
        setall.setValue(QString("item%1/angle").arg(j,3,10,QChar('0')), pi.angle);
        setall.setValue(QString("item%1/angle_mode").arg(j,3,10,QChar('0')), static_cast<int>(pi.angleMode));
        if (pi.mode==ProcessingTask::Mode::ZY) {
            setall.setValue(QString("item%1/mode").arg(j,3,10,QChar('0')), "ZY");
        } else if (pi.mode==ProcessingTask::Mode::XZ) {
            setall.setValue(QString("item%1/mode").arg(j,3,10,QChar('0')), "XZ");
        }
    }
}

QVariant ProcessingParameterTable::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation==Qt::Horizontal) {
        if (role==Qt::DisplayRole) {
            if (section==0) return tr("mode");
            if (section==1) return tr("x");
            if (section==2) return tr("y");
            if (section==3) return tr("angle mode");
            if (section==4) return tr("angle");
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
            if (mode==ProcessingTask::Mode::XZ) return "XZ";
            if (mode==ProcessingTask::Mode::ZY) return "ZY";
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
                else return angle;
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
