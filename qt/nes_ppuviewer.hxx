#pragma once
#ifndef TKP_NES_PPUVIEWER_H
#define TKP_NES_PPUVIEWER_H
#include <QWidget>

class NES_PPUViewer : public QWidget {
    Q_OBJECT
private:
    bool& open_;
public:
    NES_PPUViewer(bool& open, QWidget* parent = nullptr);
    ~NES_PPUViewer();
};
#endif