#include "KTileManager.h"

KTileManager::KTileManager()
    : m_tileSheet(nullptr), m_tileWidth(0), m_tileHeight(0), m_colsPerRow(0)
{
}

KTileManager::~KTileManager()
{
    if (m_tileSheet != nullptr) {
        delete m_tileSheet;
        m_tileSheet = nullptr;
    }
}

bool KTileManager::LoadTileSheet(const WCHAR* filename, const int tileWidth, const int tileHeight)
{
    if (m_tileSheet != nullptr) {
        delete m_tileSheet;
        m_tileSheet = nullptr;
    }

    m_tileSheet = new Image(filename);
    if (m_tileSheet->GetLastStatus() != Ok) {
        delete m_tileSheet;
        m_tileSheet = nullptr;
        return false;
    }

    m_tileWidth = tileWidth;
    m_tileHeight = tileHeight;
    m_colsPerRow = m_tileSheet->GetWidth() / m_tileWidth;

    return true;
}

void KTileManager::DrawTile(const HDC hdc, const int x, const int y, const int row, const int col, const bool mirror, const double scale)
{
    if (m_tileSheet == nullptr) return;

    Graphics graphics(hdc);
    graphics.SetInterpolationMode(InterpolationModeNearestNeighbor);

    const int srcX = col * m_tileWidth;
    const int srcY = row * m_tileHeight;

    const int destW = static_cast<int>(m_tileWidth * scale);
    const int destH = static_cast<int>(m_tileHeight * scale);

    const Rect srcRect(srcX, srcY, m_tileWidth, m_tileHeight);

    if (mirror) {
        Matrix oldTransform;
        graphics.GetTransform(&oldTransform);

        Matrix mirrorMatrix;
        mirrorMatrix.Translate(static_cast<REAL>(x + destW), static_cast<REAL>(y));
        mirrorMatrix.Scale(-static_cast<REAL>(scale*2), static_cast<REAL>(scale*2));
        graphics.SetTransform(&mirrorMatrix);

        graphics.DrawImage(m_tileSheet, 0, 0,
            srcRect.X, srcRect.Y, srcRect.Width, srcRect.Height, UnitPixel);

        graphics.SetTransform(&oldTransform);
    }
    else {
        const Rect destRect(x, y, destW, destH);
        graphics.DrawImage(m_tileSheet, destRect,
            srcRect.X, srcRect.Y, srcRect.Width, srcRect.Height, UnitPixel);
    }
}
