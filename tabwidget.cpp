#include "tabwidget.h"
#include "mainwindow.h"
#include <QPainter>

TabWidget::TabWidget(QWidget *parent) : QOpenGLWidget(parent) {
    setMinimumHeight(110);
}

void TabWidget::setChords(const QVector<ChordMarker> &chords) {
    m_chords = chords;
    update();
}

void TabWidget::setCurrentTime(double timeSeconds) {
    m_currentTime = timeSeconds;
    update();
}

void TabWidget::setChord(const QString &chord) {
    m_currentChord = chord.trimmed();
    update();
}

void TabWidget::setTimeWindow(double startTime, double endTime, bool showWholeSong) {
    m_windowStart = startTime;
    m_windowEnd = endTime;
    m_showWholeSong = showWholeSong;
    update();
}

void TabWidget::initializeGL() {
    initializeOpenGLFunctions();
    glClearColor(0.06f, 0.06f, 0.043f, 1.0f);
}

void TabWidget::resizeGL(int w, int h) {
    glViewport(0, 0, w, h);
}

void TabWidget::paintGL() {
    glClear(GL_COLOR_BUFFER_BIT);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    int w = width();
    int h = height();

    // Background panel
    painter.fillRect(rect(), QColor(9, 9, 11));
    painter.setPen(QPen(QColor(63, 63, 70), 1));
    painter.drawRoundedRect(rect().adjusted(2, 2, -2, -2), 6, 6);

    // Draw 6 guitar strings (e, B, G, D, A, E)
    QStringList stringNames = {"e", "B", "G", "D", "A", "E"};
    int startY = 24; // Spacing offset to accommodate 7px chord name gap above top string
    int spacing = 14;

    painter.setFont(QFont("Monospace", 8, QFont::Bold));

    for (int i = 0; i < 6; ++i) {
        int y = startY + (i * spacing);

        painter.setPen(QColor(161, 161, 170));
        painter.drawText(12, y + 4, stringNames[i] + "|");

        painter.setPen(QPen(QColor(82, 82, 91), 1));
        painter.drawLine(35, y, w - 15, y);
    }

    if (!m_chords.isEmpty()) {
        double totalDuration = m_chords.last().timeSeconds + 5.0;
        if (totalDuration <= 0) totalDuration = 1.0;

        double viewStart = 0.0;
        double viewEnd = totalDuration;

        if (!m_showWholeSong && m_windowEnd > m_windowStart) {
            viewStart = m_windowStart;
            viewEnd = m_windowEnd;
        }

        double viewDuration = viewEnd - viewStart;
        if (viewDuration <= 0) viewDuration = 1.0;

        int usableWidth = w - 60;
        int startX = 45;

        for (int i = 0; i < m_chords.size(); ++i) {
            const auto &marker = m_chords[i];
            double nextTime = (i + 1 < m_chords.size()) ? m_chords[i + 1].timeSeconds : totalDuration;

            if (!m_showWholeSong && (marker.timeSeconds > viewEnd + 2.0 || nextTime < viewStart - 2.0)) {
                continue;
            }

            bool isActive = (m_currentTime >= marker.timeSeconds && m_currentTime < nextTime);

            double normalizedX = (marker.timeSeconds - viewStart) / viewDuration;
            int xPos = startX + static_cast<int>(normalizedX * usableWidth);

            if (xPos < 35 || xPos > w - 20) continue;

            if (isActive) {
                painter.setPen(Qt::NoPen);
                painter.setBrush(QColor(245, 158, 11, 40));
                painter.drawRoundedRect(xPos - 8, 8, 30, h - 16, 4, 4);
            }

            // Draw chord name: 7 pixels spacing above the top string (startY = 24)
            painter.setFont(QFont("Monospace", 8, isActive ? QFont::Bold : QFont::Normal));
            // Chord name is noticeably whiter than the tab font
            painter.setPen(isActive ? QColor(255, 255, 255) : QColor(228, 228, 231));
            painter.drawText(xPos - 4, startY - 7, marker.chordName);

            // Draw tab fret numbers
            painter.setFont(QFont("Monospace", 9, QFont::Bold));
            painter.setPen(isActive ? QColor(52, 211, 153) : QColor(113, 113, 122));

            QVector<QString> shape;
            if (MainWindow::Pmain) {
                shape = MainWindow::Pmain->getActiveShape(marker.chordName);
            }

            for (int s = 0; s < 6; ++s) {
                if (s >= shape.size()) break;
                QString fretStr = shape[s];
                if (fretStr != "x" && fretStr != "0") {
                    int fNum = fretStr.toInt();
                    int tabStringIdx = 5 - s;
                    int noteY = startY + (tabStringIdx * spacing);

                    painter.drawText(xPos, noteY + 4, QString::number(fNum));
                }
            }
        }
    }
}
