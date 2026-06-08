#pragma once

#include <od/graphics/ScreenSaver.h>
#include <cstdint>

namespace od
{

  class Mushroom : public ScreenSaver
  {
  public:
    Mushroom();
    virtual ~Mushroom();

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

    // キノコ — 本数はそのまま、サイズは倍
    static const int MUSHROOM_N = 12;
    struct Mushroom_t
    {
      float x;
      float stalkHeight;
      float capRadius;
      float curvature;
      float wobble;
      float age;
      float growthSpeed;
      float stalkWidth;
      float lean;
    };
    Mushroom_t mMushrooms[MUSHROOM_N];

    // 胞子 — 増量
    static const int SPORE_N = 80;
    struct Spore_t
    {
      float x, y;
      float vx, vy;
      float life;
    };
    Spore_t mSpores[SPORE_N];

    // 蝶々
    static const int BUTTERFLY_N = 4;
    struct Butterfly_t
    {
      float x, y;
      float vx, vy;
      float phase;       // 羽ばたく位相
      float freq;        // 羽ばたき周波数
      float hoverPhase;  // ふわふわ位置
    };
    Butterfly_t mButterflies[BUTTERFLY_N];

    // 地面
    float mGroundY[512]; // 2x 解像度

    float mTime;
    float mElapsed;

    void spawnMushroom(int idx);
    void emitSpore(float x, float y);
    void spawnButterfly(int idx);
  };

} /* namespace od */
