#pragma once

#include <od/graphics/ScreenSaver.h>
#include <cstdint>

namespace od
{

  class LiquidLight : public ScreenSaver
  {
  public:
    LiquidLight();
    virtual ~LiquidLight();

    virtual void reset();
    virtual void draw(FrameBuffer &mainFrameBuffer,
                      FrameBuffer &subFrameBuffer);

  private:
    // Perlin noise
    static float fade(float t);
    static float lerp(float a, float b, float t);
    static float grad(int hash, float x, float y);
    float noise(float x, float y);
    float fbm(float x, float y, int octaves);

    int mPerm[512];

    float mTime;

    // サブ画面用の気泡
    static const int BUBBLE_N = 20;
    struct Bubble_t
    {
      float x, y;
      float speed;
      float size;   // 1 or 2
      float phase;
    };
    Bubble_t mBubbles[BUBBLE_N];

    // セル中心点（油の塊）
    static const int CELL_N = 12;
    struct Cell_t
    {
      float x, y;
      float vx, vy;
      float radius;    // 影響半径
      float intensity; // 明るさ
      float phase;     // 脈動位相
    };
    Cell_t mCells[CELL_N];
  };

} /* namespace od */
