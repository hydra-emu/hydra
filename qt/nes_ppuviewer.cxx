#include "nes_ppuviewer.hxx"

NES_PPUViewer::NES_PPUViewer(bool& open, QWidget* parent) : open_(open), QWidget(parent) {}

NES_PPUViewer::~NES_PPUViewer()
{
    open_ = false;
}