#ifndef PROCESSINGTASKTOOLWIDGETS_H
#define PROCESSINGTASKTOOLWIDGETS_H
#include <QComboBox>
#include "processingtask.h"

class InterpolationMethodComboBox : public QComboBox {
    Q_OBJECT

public:
    inline InterpolationMethodComboBox(QWidget* parent = nullptr) : QComboBox(parent) {
        // Populate the combo box with InterpolationMethod items
        addItem(tr("Nearest Neighbor (lowest quality)"), QVariant::fromValue(static_cast<int>(ProcessingTask::InterpolationMethod::NearestNeighbor)));
        addItem(tr("Linear"), QVariant::fromValue(static_cast<int>(ProcessingTask::InterpolationMethod::Linear)));
        addItem(tr("Cubic (highest quality)"), QVariant::fromValue(static_cast<int>(ProcessingTask::InterpolationMethod::Cubic)));
    }

    inline ProcessingTask::InterpolationMethod getCurrentInterpolationMode() const {
        return static_cast<ProcessingTask::InterpolationMethod>(currentData().toInt());
    }

    inline void setCurrentInterpolationMode(ProcessingTask::InterpolationMethod method) {
        int index = findData(QVariant::fromValue(static_cast<int>(method)));
        if (index != -1) {
            setCurrentIndex(index);
        }
    }
};

class AddBeforeAfterModeComboBox : public QComboBox {
    Q_OBJECT

public:
    inline AddBeforeAfterModeComboBox(QWidget* parent = nullptr) : QComboBox(parent) {
        // Populate the combo box with AddBeforeAfterMode items
        addItem(tr("nothing"), QVariant::fromValue(static_cast<int>(ProcessingTask::AddBeforeAfterMode::None)));
        addItem(tr("left/top half image"), QVariant::fromValue(static_cast<int>(ProcessingTask::AddBeforeAfterMode::ToLower)));
        addItem(tr("right/bottom half image"), QVariant::fromValue(static_cast<int>(ProcessingTask::AddBeforeAfterMode::ToHigher)));
    }

    inline ProcessingTask::AddBeforeAfterMode currentMode() const {
        return static_cast<ProcessingTask::AddBeforeAfterMode>(currentData().toInt());
    }

    inline void setCurrentMode(ProcessingTask::AddBeforeAfterMode method) {
        int index = findData(QVariant::fromValue(static_cast<int>(method)));
        if (index != -1) {
            setCurrentIndex(index);
        }
    }
};

class AngleModeComboBox : public QComboBox {
    Q_OBJECT

public:
    inline AngleModeComboBox(QWidget* parent = nullptr) : QComboBox(parent) {
        // Populate the combo box with AngleMode items
        addItem(QIcon(":/icons/pitch.png"), tr("pitch"), QVariant::fromValue(static_cast<int>(ProcessingTask::AngleMode::AnglePitch)));
        addItem(QIcon(":/icons/roll.png"), tr("roll"), QVariant::fromValue(static_cast<int>(ProcessingTask::AngleMode::AngleRoll)));
        setCurrentMode(ProcessingTask::AngleMode::DefaultNotNothing);
    }

    inline ProcessingTask::AngleMode currentMode() const {
        return static_cast<ProcessingTask::AngleMode>(currentData().toInt());
    }

    inline void setCurrentMode(ProcessingTask::AngleMode method) {
        int index = findData(QVariant::fromValue(static_cast<int>(method)));
        if (index != -1) {
            setCurrentIndex(index);
        } else {
            setCurrentMode(ProcessingTask::AngleMode::DefaultNotNothing);
        }
    }
};

class FileFormatComboBox : public QComboBox {
    Q_OBJECT

public:
    inline FileFormatComboBox(QWidget* parent = nullptr) : QComboBox(parent) {
        // Populate the combo box with FileFormat items
        for (int i=0; i<static_cast<int>(ProcessingTask::FileFormat::__COUNT); i++) {
            addItem(ProcessingTask::FileFormat2String(static_cast<ProcessingTask::FileFormat>(i)), QVariant::fromValue(i));
        }
    }

    inline ProcessingTask::FileFormat currentFileFormat() const {
        return static_cast<ProcessingTask::FileFormat>(currentData().toInt());
    }

    inline void setCurrentFileFormat(ProcessingTask::FileFormat method) {
        int index = findData(QVariant::fromValue(static_cast<int>(method)));
        if (index != -1) {
            setCurrentIndex(index);
        }
    }
};

class OutputTargetOptionsComboBox : public QComboBox {
    Q_OBJECT

public:
    inline OutputTargetOptionsComboBox(QWidget* parent = nullptr) : QComboBox(parent) {
        // Populate the combo box with OutputTargetOptions items
        addItem(tr("same directory as input file"), QVariant::fromValue(static_cast<int>(ProcessingTask::OutputTargetOptions::SameDirectoryAsInput)));
        addItem(tr("generate subdirectory for each input file"), QVariant::fromValue(static_cast<int>(ProcessingTask::OutputTargetOptions::SubDirectoryPerInput)));
    }

    inline ProcessingTask::OutputTargetOptions currentOutputTarget() const {
        return static_cast<ProcessingTask::OutputTargetOptions>(currentData().toInt());
    }

    inline void setCurrentOutputTarget(ProcessingTask::OutputTargetOptions method) {
        int index = findData(QVariant::fromValue(static_cast<int>(method)));
        if (index != -1) {
            setCurrentIndex(index);
        }
    }
};

#endif // PROCESSINGTASKTOOLWIDGETS_H
