
#include "mainwindow.h"
#include <QApplication>
#include <QStyleFactory>
#include <QDir>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <streambuf>
#include <QDebug>
#include <QCoreApplication>
#include <QDir>


#pragma once
#include <iostream>
#include <sstream>
#include <streambuf>
#include <QDebug>

class QDebugStream : public std::streambuf {
public:
    QDebugStream(std::ostream &stream) : m_stream(stream) {
        // Save the old buffer and redirect the stream to this instance
        m_old_buf = m_stream.rdbuf(this);
    }

    ~QDebugStream() {
        // Restore the original buffer upon destruction
        m_stream.rdbuf(m_old_buf);
    }

protected:
    // This function is called when characters are written to the stream
    virtual int_type overflow(int_type v) override {
        if (v == traits_type::eof()) {
            return traits_type::not_eof(v);
        }

        if (v == '\n') {
            // Found a newline, flush the accumulated line to qDebug
            qDebug().noquote() << QString::fromStdString(m_buffer);
            m_buffer.clear();
        } else {
            m_buffer.push_back(v);
        }
        return v;
    }

    // Handles bulk block transfers (e.g., std::endl or large strings)
    virtual std::streamsize xsputn(const char *p, std::streamsize n) override {
        std::string str(p, n);
        size_t pos = 0;

        while ((pos = str.find('\n')) != std::string::npos) {
            m_buffer.append(str.substr(0, pos));
            qDebug().noquote() << QString::fromStdString(m_buffer);
            m_buffer.clear();
            str.erase(0, pos + 1);
        }

        m_buffer.append(str);
        return n;
    }

private:
    std::ostream &m_stream;
    std::streambuf *m_old_buf;
    std::string m_buffer;
};


// Call this once at startup, before any chord analysis
static void setupVampPluginPath()
{
    // 1. Check if VAMP_PATH is already set by the user or system
    const char* existingPath = std::getenv("VAMP_PATH");

    // If it exists and is not an empty string, respect it and exit early
    if (existingPath && existingPath[0] != '\0') {
        qDebug() << "VAMP_PATH already set to:" << existingPath;
        return;
    }

    // 2. Fall back to your local application plugins directory
    QString appDir = QCoreApplication::applicationDirPath();
    QString pluginsDir = QDir(appDir).filePath("plugins");

#ifdef Q_OS_WIN
    // Windows: Use _putenv_s (automatically fails if already exists, but we checked anyway)
    _putenv_s("VAMP_PATH", pluginsDir.toLocal8Bit().constData());
#else
    // Linux / macOS: Set overwrite flag (third argument) to 0 as a secondary safety
    setenv("VAMP_PATH", pluginsDir.toLocal8Bit().constData(), 0);
#endif

    qDebug() << "VAMP_PATH fallback set to:" << pluginsDir;
}


int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    QDebugStream redirectCerr(std::cerr);
    app.setApplicationVersion(APP_VERSION);
    // Set VAMP_PATH so Chordino / nnls-chroma plugins are found next to the binary
    setupVampPluginPath();

    // Apply dark studio theme
    app.setStyle(QStyleFactory::create("Fusion"));
    QPalette darkPalette;
    darkPalette.setColor(QPalette::Window, QColor(9, 9, 11));
    darkPalette.setColor(QPalette::WindowText, QColor(244, 244, 245));
    darkPalette.setColor(QPalette::Base, QColor(24, 24, 27));
    darkPalette.setColor(QPalette::AlternateBase, QColor(39, 39, 42));
    darkPalette.setColor(QPalette::ToolTipBase, Qt::white);
    darkPalette.setColor(QPalette::ToolTipText, Qt::white);
    darkPalette.setColor(QPalette::Text, QColor(244, 244, 245));
    darkPalette.setColor(QPalette::Button, QColor(24, 24, 27));
    darkPalette.setColor(QPalette::ButtonText, QColor(244, 244, 245));
    darkPalette.setColor(QPalette::BrightText, Qt::red);
    darkPalette.setColor(QPalette::Link, QColor(217, 119, 6));
    darkPalette.setColor(QPalette::Highlight, QColor(217, 119, 6));
    darkPalette.setColor(QPalette::HighlightedText, QColor(9, 9, 11));
    app.setPalette(darkPalette);

    MainWindow w;
    w.resize(1280, 800);
    w.show();

    int ir = app.exec();
    return ir;
}

