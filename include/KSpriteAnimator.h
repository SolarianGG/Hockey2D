#pragma once
#include <map>
#include <vector>

#include "KTileManager.h"
#include "KVector2.h"

struct Animation {
  std::vector<KVector2> sequence;
  double timeBetweenFrames;
  double elapsedTime = 0.0;
  int currentFrame = 0;
};

class KSpriteAnimator {
 private:
  KTileManager* tilemap_ = nullptr;
  std::map<int, Animation> animations_;

 public:
  void SetTileMap(KTileManager* tilemap) { tilemap_ = tilemap; }

  void SetAnimation(const int id, const std::vector<KVector2>& sequence,
                    const double timeBetweenSprite) {
    Animation anim;
    anim.sequence = sequence;
    anim.timeBetweenFrames = timeBetweenSprite;
    animations_[id] = anim;
  }

  void Update(const double deltaTime) {
    for (std::map<int, Animation>::iterator it = animations_.begin();
         it != animations_.end(); ++it) {
      Animation& anim = it->second;
      anim.elapsedTime += deltaTime;
      if (anim.elapsedTime >= anim.timeBetweenFrames) {
        anim.elapsedTime = 0.0;
        anim.currentFrame = (anim.currentFrame + 1) % anim.sequence.size();
      }
    }
  }

  void Draw(const HDC hdc, const int id, const int x, const int y,
            const bool mirror = false, const double scale = 1.0) {
    if (animations_.find(id) == animations_.end() || !tilemap_) return;
    const Animation& anim = animations_[id];
    const KVector2& tile = anim.sequence[anim.currentFrame];
    tilemap_->DrawTile(hdc, x, y, (int)tile.y, (int)tile.x, mirror, scale);
  }

  void ClearAll() { animations_.clear(); }
};
