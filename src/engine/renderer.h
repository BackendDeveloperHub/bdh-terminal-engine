// src/engine/renderer.h
#ifndef BDH_RENDERER_H
#define BDH_RENDERER_H

#include "engine/screen.h"
#include "ui/panes.h"
#include "engine/cursor.h"

// அனைத்து விண்டோக்களையும் திரையில் ரெண்டர் செய்யும் பிரதான பங்க்ஷன்
void renderer_draw_all(VirtualScreen *scr, void *sessions_ptr, int count);

#endif // BDH_RENDERER_H
