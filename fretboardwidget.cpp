#include "fretboardwidget.h"
#include <QPainter>

FretboardWidget::FretboardWidget(QWidget *parent) : QWidget(parent) {
    setMinimumHeight(320);
    initShapes();
}

void FretboardWidget::initShapes() {
    // Strings ordered: Low E, A, D, G, B, High E
    // Power chords limited to 3 notes, strictly on lower E, A, and D strings.

    // G Family
    m_shapes["G"]   = { {"3","2","0","0","3","3"}, {"3","5","5","x","x","x"} };
    m_shapes["GM"]  = m_shapes["G"];
    m_shapes["Gm"]  = { {"3","5","5","3","3","3"}, {"3","5","5","x","x","x"} };
    m_shapes["G#"]  = { {"4","6","6","5","4","4"}, {"4","6","6","x","x","x"} };
    m_shapes["G#M"] = m_shapes["G#"];
    m_shapes["G#m"] = { {"4","6","6","4","4","4"}, {"4","6","6","x","x","x"} };
    m_shapes["G7"]  = { {"3","2","0","0","0","1"}, {"3","5","5","x","x","x"} };
    m_shapes["Gm7"] = { {"3","5","3","3","3","3"}, {"3","5","5","x","x","x"} };

    // D Family
    m_shapes["D"]   = { {"x","x","0","2","3","2"}, {"x","5","7","7","x","x"} };
    m_shapes["DM"]  = m_shapes["D"];
    m_shapes["Dm"]  = { {"x","x","0","2","3","1"}, {"x","5","7","7","x","x"} };
    m_shapes["D#"]  = { {"x","6","8","8","8","6"}, {"x","6","8","8","x","x"} };
    m_shapes["D#M"] = m_shapes["D#"];
    m_shapes["D#m"] = { {"x","6","8","8","7","6"}, {"x","6","8","8","x","x"} };
    m_shapes["D7"]  = { {"x","x","0","2","1","2"}, {"x","5","7","7","x","x"} };
    m_shapes["Dm7"] = { {"x","x","0","2","1","1"}, {"x","5","7","7","x","x"} };

    // A Family
    m_shapes["A"]   = { {"x","0","2","2","2","0"}, {"5","7","7","7","x","x"} };
    m_shapes["AM"]  = m_shapes["A"];
    m_shapes["Am"]  = { {"x","0","2","2","1","0"}, {"5","7","7","7","x","x"} };
    m_shapes["A#"]  = { {"x","1","3","3","3","1"}, {"6","8","8","8","x","x"} };
    m_shapes["A#M"] = m_shapes["A#"];
    m_shapes["A#m"] = { {"x","1","3","3","2","1"}, {"6","8","8","8","x","x"} };
    m_shapes["A7"]  = { {"x","0","2","0","2","0"}, {"5","7","7","7","x","x"} };
    m_shapes["Am7"] = { {"x","0","2","0","1","0"}, {"5","7","7","7","x","x"} };

    // E Family
    m_shapes["E"]   = { {"0","2","2","1","0","0"}, {"0","2","2","2","x","x"} };
    m_shapes["EM"]  = m_shapes["E"];
    m_shapes["Em"]  = { {"0","2","2","0","0","0"}, {"0","2","2","2","x","x"} };
    m_shapes["F"]   = { {"1","3","3","2","1","1"}, {"1","3","3","3","x","x"} };
    m_shapes["FM"]  = m_shapes["F"];
    m_shapes["Fm"]  = { {"1","3","3","1","1","1"}, {"1","3","3","3","x","x"} };
    m_shapes["F#"]  = { {"2","4","4","3","2","2"}, {"2","4","4","4","x","x"} };
    m_shapes["F#M"] = m_shapes["F#"];
    m_shapes["F#m"] = { {"2","4","4","2","2","2"}, {"2","4","4","4","x","x"} };
    m_shapes["F7"]  = { {"1","3","1","2","1","1"}, {"1","3","3","3","x","x"} };
    m_shapes["Fm7"] = { {"1","3","1","1","1","1"}, {"1","3","3","3","x","x"} };
    m_shapes["E7"]  = { {"0","2","0","1","0","0"}, {"0","2","2","2","x","x"} };
    m_shapes["Em7"] = { {"0","2","0","0","0","0"}, {"0","2","2","2","x","x"} };

    // C Family
    m_shapes["C"]   = { {"x","3","2","0","1","0"}, {"x","3","5","5","x","x"} };
    m_shapes["CM"]  = m_shapes["C"];
    m_shapes["Cm"]  = { {"x","3","5","5","4","3"}, {"x","3","5","5","x","x"} };
    m_shapes["C#"]  = { {"x","4","6","6","6","4"}, {"x","4","6","6","x","x"} };
    m_shapes["C#M"] = m_shapes["C#"];
    m_shapes["C#m"] = { {"x","4","6","6","5","4"}, {"x","4","6","6","x","x"} };
    m_shapes["C7"]  = { {"x","3","2","3","1","0"}, {"x","3","5","5","x","x"} };
    m_shapes["Cm7"] = { {"x","3","5","3","4","3"}, {"x","3","5","5","x","x"} };

    // B Family
    m_shapes["B"]   = { {"x","2","4","4","4","2"}, {"2","4","4","4","x","x"} };
    m_shapes["BM"]  = m_shapes["B"];
    m_shapes["Bm"]  = { {"x","2","4","4","3","2"}, {"2","4","4","4","x","x"} };
    m_shapes["B7"]  = { {"x","2","1","2","0","2"}, {"7","9","9","9","x","x"} };
    m_shapes["Bm7"] = { {"x","2","4","2","3","2"}, {"7","9","9","9","x","x"} };
}

void FretboardWidget::setChord(const QString &currentChord, const QString &nextChord) {
    m_currentChord = currentChord.trimmed();
    m_nextChord = nextChord.trimmed();
    update();
}

void FretboardWidget::setViewMode(bool isTab) {
    m_viewTabs = isTab;
    update();
}

void FretboardWidget::setAccentColor(const QColor &color) {
    m_accentColor = color;
    update();
}

void FretboardWidget::setFingeringMode(bool isPower) {
    m_powerMode = isPower;
    update();
}

QVector<QString> FretboardWidget::getActiveShape(const QString &chord) {
    if (chord.isEmpty()) return m_shapes["G"].stdShape;

    QString cleanChord = chord.trimmed();
    // Support legacy "MAJ" / "MIN" strings if passed, but preserve case sensitivity for 'm'
    QString lookupKey = cleanChord;
    if (lookupKey.endsWith("maj", Qt::CaseInsensitive)) {
        lookupKey.chop(3);
        lookupKey = lookupKey.toUpper();
    }

    if (m_shapes.contains(lookupKey)) {
        Shape s = m_shapes.value(lookupKey);
        return m_powerMode ? s.powerShape : s.stdShape;
    }

    // Fallback search by root prefix
    QString upperClean = cleanChord.toUpper();
    upperClean.remove("MAJ").remove("MIN");
    for (auto it = m_shapes.constBegin(); it != m_shapes.constEnd(); ++it) {
        if (upperClean.startsWith(it.key())) {
            return m_powerMode ? it.value().powerShape : it.value().stdShape;
        }
    }

    return m_shapes["G"].stdShape;
}

void FretboardWidget::drawFretboard(QPainter &painter, const QRect &bounds, const QString &chordName, bool isMini) {
    QVector<QString> shape = getActiveShape(chordName);

    int numFrets = 8; // Increased from 6 to 8 frets
    int startX = bounds.left() + (isMini ? 15 : 40);
    int endX = bounds.right() - (isMini ? 15 : 40);
    int fretW = (endX - startX) / numFrets;

    int topY = bounds.top() + (isMini ? 15 : 24);
    int bottomY = bounds.bottom() - (isMini ? 20 : 25);
    int stringSpacing = (bottomY - topY) / 5;

    if (isMini) {
        painter.setBrush(QColor(18, 18, 22));
        painter.setPen(QPen(QColor(161, 161, 170), 1));
        painter.drawRoundedRect(bounds, 6, 6);

        painter.setFont(QFont("Monospace", 8, QFont::Bold));
        painter.setPen(m_accentColor);
        painter.drawText(bounds.adjusted(6, 2, -6, 0), Qt::AlignTop | Qt::AlignLeft, "NEXT: " + chordName);
    } else {
        painter.setFont(QFont("Monospace", 14, QFont::Bold));
        painter.setPen(m_accentColor);

        QString chordTitle = chordName.isEmpty() ? "CHORD: --" : "CHORD: " + chordName;
        painter.drawText(QRect(startX, bounds.top() - 2, endX - startX, 22), Qt::AlignLeft | Qt::AlignVCenter, chordTitle);
    }

    // Draw Vertical Frets and Fret Numbers (1 to 8)
    painter.setFont(QFont("Monospace", isMini ? 7 : 10, QFont::Bold));
    for (int f = 0; f <= numFrets; ++f) {
        int fx = startX + (f * fretW);
        painter.setPen(QPen(f == 0 ? QColor(212, 212, 216) : QColor(82, 82, 91), f == 0 ? (isMini ? 2 : 4) : 1.5));
        painter.drawLine(fx, topY, fx, bottomY);

        if (f < numFrets) {
            int numX = fx + (fretW / 2);
            painter.setPen(QColor(161, 161, 170));
            painter.drawText(QRect(numX - 10, bottomY + 2, 20, 18), Qt::AlignCenter, QString::number(f + 1));
        }
    }

    // Draw Horizontal Strings and Active Fingering Dots
    for (int s = 0; s < 6; ++s) {
        int sy = topY + (s * stringSpacing);
        painter.setPen(QPen(QColor(161, 161, 170), isMini ? 1 : (1 + s * 0.4)));
        painter.drawLine(startX, sy, endX, sy);

        QString fretStr = shape[5 - s];
        if (fretStr != "x" && fretStr != "0") {
            int fNum = fretStr.toInt();
            if (fNum <= numFrets) {
                int mx = startX + ((fNum - 1) * fretW) + (fretW / 2);
                int dotRadius = isMini ? 4 : 10;

                painter.setBrush(isMini ? QColor(16, 185, 129) : m_accentColor);
                painter.setPen(Qt::NoPen);
                painter.drawEllipse(QPoint(mx, sy), dotRadius, dotRadius);
            }
        }
    }
}


void FretboardWidget::paintEvent(QPaintEvent *) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(rect(), QColor(9, 9, 11));

    int w = width();
    int h = height();

    // Main Center Fretboard
    int mainHeight = h * 0.8;
    int mainTopY = h * 0.15;
    QRect mainBounds(0, mainTopY, w, mainHeight);

    drawFretboard(painter, mainBounds, m_currentChord, false);

    // Top-Right Mini Fretboard (Next Chord)
    int miniW = w / 4;
    int miniH = 80;
    QRect miniBounds(w - miniW - 12, 8, miniW, miniH);

    if (!m_nextChord.isEmpty()) {
        drawFretboard(painter, miniBounds, m_nextChord, true);
    }
}
