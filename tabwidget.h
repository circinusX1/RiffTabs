#pragma once

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QString>
#include <QVector>
#include "glwaveformwidget.h"

class TabWidget : public QOpenGLWidget, protected QOpenGLFunctions {
    Q_OBJECT
public:
    explicit TabWidget(QWidget *parent = nullptr);
    void setChords(const QVector<ChordMarker> &chords);
    void setCurrentTime(double timeSeconds);
    void setChord(const QString &chord);
    void setTimeWindow(double startTime, double endTime, bool showWholeSong = false);

protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;

private:
    QVector<ChordMarker> m_chords;
    QString m_currentChord;
    double m_currentTime = 0.0;
    double m_windowStart = 0.0;
    double m_windowEnd = 0.0;
    bool m_showWholeSong = true;
};
