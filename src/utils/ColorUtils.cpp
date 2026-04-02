#include "thetaexplorer/ColorUtils.h"

QString ColorUtils::getStyleSheet()
{
    return QString(R"(
QWidget {
    background-color: #161616;
    color: #b2b2b2;
    font-family: "Inter", "Helvetica Neue", "Arial", sans-serif;
    font-size: 13px;
}
QMainWindow {
    background-color: #161616;
}
QScrollArea {
    background-color: #161616;
    border: none;
}
QScrollBar:vertical {
    background: #1d1d1d;
    width: 8px;
    border-radius: 4px;
}
QScrollBar::handle:vertical {
    background: #444444;
    border-radius: 4px;
    min-height: 20px;
}
QScrollBar::handle:vertical:hover {
    background: #666666;
}
QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
    height: 0;
}
QScrollBar:horizontal {
    background: #1d1d1d;
    height: 8px;
    border-radius: 4px;
}
QScrollBar::handle:horizontal {
    background: #444444;
    border-radius: 4px;
}
QScrollBar::handle:horizontal:hover {
    background: #666666;
}
QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal {
    width: 0;
}
QPushButton {
    background-color: #252525;
    color: #b2b2b2;
    border: 1px solid #333333;
    border-radius: 5px;
    padding: 5px 14px;
    font-size: 13px;
}
QPushButton:hover {
    background-color: #2e2e2e;
    border-color: #555555;
}
QPushButton:pressed {
    background-color: #1a1a1a;
}
QPushButton:disabled {
    color: #444444;
    border-color: #2a2a2a;
    background-color: #1d1d1d;
}
QPushButton#downloadBtn {
    background-color: #3a2f6e;
    border-color: #774dcb;
    color: #c8b8f0;
}
QPushButton#downloadBtn:hover {
    background-color: #4a3a88;
    border-color: #8e6ddf;
}
QPushButton#downloadBtn:disabled {
    background-color: #252525;
    border-color: #333333;
    color: #444444;
}
QPushButton#deleteBtn {
    background-color: #3d1f27;
    border-color: #cf6679;
    color: #f0b8c0;
}
QPushButton#deleteBtn:hover {
    background-color: #4d2535;
    border-color: #df7080;
}
QPushButton#deleteBtn:disabled {
    background-color: #252525;
    border-color: #333333;
    color: #444444;
}
QLabel {
    color: #b2b2b2;
    background: transparent;
}
QLabel#dimLabel {
    color: #555555;
    font-size: 12px;
}
QLabel#accentLabel {
    color: #774dcb;
}
QLabel#successLabel {
    color: #4caf7d;
}
QLabel#errorLabel {
    color: #cf6679;
}
QProgressBar {
    background-color: #252525;
    border: 1px solid #333333;
    border-radius: 4px;
    height: 6px;
    text-align: center;
    font-size: 11px;
    color: #777777;
}
QProgressBar::chunk {
    background-color: #774dcb;
    border-radius: 4px;
}
QSplitter::handle {
    background-color: #2a2a2a;
    width: 1px;
    height: 1px;
}
QStatusBar {
    background-color: #111111;
    border-top: 1px solid #222222;
    color: #666666;
    font-size: 12px;
}
QDialog {
    background-color: #1d1d1d;
}
)");
}
