#pragma once
#include <Windows.h>
#include <gdiplus.h>
using namespace Gdiplus;

class KTileManager
{
private:
    Image* m_tileSheet;
    int m_tileWidth;
    int m_tileHeight;
    int m_colsPerRow;

public:
    KTileManager();
    ~KTileManager();

    bool LoadTileSheet(const WCHAR* filename, int tileWidth, int tileHeight);
    void DrawTile(HDC hdc, int x, int y, int row, int col, bool mirror = false, double scale = 1.0);
};
