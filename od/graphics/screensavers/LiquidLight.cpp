#include <od/graphics/screensavers/LiquidLight.h>
#include <od/graphics/FrameBuffer.h>
#include <od/graphics/constants.h>
#include <hal/constants.h>
#include <cmath>
#include <cstdlib>

namespace od
{

  // ============================================================
  //  LiquidLight — 煙が植物繊維のように変容する
  //  リキッドライト風モノトーン濃淡表現
  //
  //  3 層構造:
  //    layer1 (低周波): 煙の大きなうねり
  //    layer2 (中周波+ドメインワープ): 繊維状のディテール
  //    layer3 (高周波): 湿った質感の細かい撹拌
  //
  //  セル位置の濃度変調で丸みを帯びた塊が浮かぶ。
  //  コントラスト強調でメリハリのあるノイズ感。
  // ============================================================

  // ---- Perlin noise ----

  float LiquidLight::fade(float t)
  {
    return t * t * t * (t * (t * 6.0f - 15.0f) + 10.0f);
  }

  float LiquidLight::lerp(float a, float b, float t)
  {
    return a + t * (b - a);
  }

  float LiquidLight::grad(int hash, float x, float y)
  {
    int h = hash & 3;
    float u = (h < 2) ? x : y;
    float v = (h < 2) ? y : x;
    return (((h & 1) == 0) ? u : -u) + (((h & 2) == 0) ? v : -v);
  }

  float LiquidLight::noise(float x, float y)
  {
    int xi = (int)floorf(x) & 255;
    int yi = (int)floorf(y) & 255;
    float xf = x - floorf(x);
    float yf = y - floorf(y);
    float u = fade(xf);
    float v = fade(yf);

    int aa = mPerm[mPerm[xi] + yi];
    int ab = mPerm[mPerm[xi] + yi + 1];
    int ba = mPerm[mPerm[xi + 1] + yi];
    int bb = mPerm[mPerm[xi + 1] + yi + 1];

    return lerp(
        lerp(grad(aa, xf, yf), grad(ba, xf - 1, yf), u),
        lerp(grad(ab, xf, yf - 1), grad(bb, xf - 1, yf - 1), u),
        v);
  }

  float LiquidLight::fbm(float x, float y, int octaves)
  {
    float value = 0.0f;
    float amp = 0.5f;
    float freq = 1.0f;
    for (int i = 0; i < octaves; i++)
    {
      value += amp * noise(x * freq, y * freq);
      freq *= 2.3f;
      amp *= 0.45f;
    }
    return value;
  }

  // ---- constructor / destructor ----

  LiquidLight::LiquidLight()
  {
    static int seed[256] = {
        151, 160, 137, 91, 90, 15, 131, 13, 201, 95, 96, 53, 194, 233, 7, 225,
        140, 36, 103, 30, 69, 142, 8, 99, 37, 240, 21, 10, 23, 190, 6, 148,
        247, 120, 234, 75, 0, 26, 197, 62, 94, 252, 219, 203, 117, 35, 11, 32,
        57, 177, 33, 88, 237, 149, 56, 87, 174, 20, 125, 136, 171, 168, 68, 175,
        74, 165, 71, 134, 139, 48, 27, 166, 77, 146, 158, 231, 83, 111, 229, 122,
        60, 211, 133, 230, 220, 105, 92, 41, 55, 46, 245, 40, 244, 102, 143, 54,
        65, 25, 63, 161, 1, 216, 80, 73, 209, 76, 132, 187, 208, 89, 18, 169,
        200, 196, 135, 130, 116, 188, 159, 86, 164, 100, 109, 198, 173, 186, 3, 64,
        52, 217, 226, 250, 124, 123, 5, 202, 38, 147, 118, 126, 255, 82, 85, 212,
        207, 206, 59, 227, 47, 16, 58, 17, 182, 189, 28, 42, 223, 183, 170, 213,
        119, 248, 152, 2, 44, 154, 163, 70, 221, 153, 101, 155, 167, 43, 172, 9,
        129, 22, 39, 253, 19, 98, 108, 110, 79, 113, 224, 232, 178, 185, 112, 104,
        218, 246, 97, 228, 251, 34, 242, 193, 238, 210, 144, 12, 191, 179, 162, 241,
        81, 51, 145, 235, 249, 14, 239, 107, 49, 192, 214, 31, 181, 199, 106, 157,
        184, 84, 204, 176, 115, 121, 50, 45, 127, 4, 150, 254, 138, 236, 205, 93,
        222, 114, 67, 29, 24, 72, 243, 141, 128, 195, 78, 66, 215, 61, 156, 180};
    for (int i = 0; i < 256; i++)
    {
      mPerm[i] = seed[i];
      mPerm[i + 256] = seed[i];
    }
    reset();
  }

  LiquidLight::~LiquidLight() {}

  void LiquidLight::reset()
  {
    mTime = 0.0f;
    for (int i = 0; i < BUBBLE_N; i++)
    {
      mBubbles[i].x = (float)(rand() % 128);
      mBubbles[i].y = (float)(rand() % 64);
      mBubbles[i].speed = 2.0f + (float)(rand() % 30) * 0.2f;
      mBubbles[i].size = (rand() % 3) + 1;  // 1〜3ドット
      mBubbles[i].phase = (float)(rand() % 628) / 100.0f;
    }

    for (int i = 0; i < CELL_N; i++)
    {
      mCells[i].x = 20.0f + (float)(rand() % 216);
      mCells[i].y = 5.0f + (float)(rand() % 54);
      mCells[i].vx = ((float)(rand() % 100) - 50.0f) * 0.02f;
      mCells[i].vy = ((float)(rand() % 100) - 50.0f) * 0.02f;
      mCells[i].radius = 10.0f + (float)(rand() % 60) * 0.3f;
      mCells[i].intensity = 0.3f + (float)(rand() % 100) / 150.0f;
      mCells[i].phase = (float)(rand() % 628) / 100.0f;
    }
  }

  void LiquidLight::draw(FrameBuffer &m, FrameBuffer &s)
  {
    float dt = GRAPHICS_REFRESH_PERIOD;
    if (dt > 0.02f) dt = 0.018f;
    mTime += dt;

    // ---- 気泡のアップデート (サブ画面) ----
    for (int i = 0; i < BUBBLE_N; i++)
    {
      Bubble_t &b = mBubbles[i];
      // ゆっくり上昇
      b.y -= b.speed * dt * 8.0f;
      // Perlin noise で横方向にふわふわ
      float drift = noise(b.x * 0.05f + mTime * 0.1f, b.y * 0.05f + mTime * 0.08f + b.phase);
      b.x += drift * 0.3f;
      // 画面端でループ
      if (b.y < -2.0f)
      {
        b.y = 66.0f;
        b.x = (float)(rand() % 128);
        b.speed = 2.0f + (float)(rand() % 30) * 0.2f;
        b.phase = (float)(rand() % 628) / 100.0f;
      }
      if (b.x < 0.0f) b.x = 0.0f;
      if (b.x > 127.0f) b.x = 127.0f;
    }

    // ---- 突風エフェクト ----
    // Perlin noise で「時々強く吹く風」を生成
    static float gustPhase = 0.0f;
    gustPhase += dt * 0.08f;
    float gustRaw = noise(gustPhase * 0.5f + 1000.0f, gustPhase * 0.3f + 2000.0f);
    // gustRaw は -1..1 → 0 が無風、1 が最大突風
    float gust = gustRaw * gustRaw * gustRaw; // 3乗で急峻な立ち上がり
    if (gust < 0.0f) gust = 0.0f;
    float gustStrength = 0.0f;          // 突風オフ

    // ---- セルの緩やかな漂流 ----
    for (int i = 0; i < CELL_N; i++)
    {
      Cell_t &c = mCells[i];
      c.x += c.vx * dt * 30.0f;
      c.y += c.vy * dt * 30.0f;
      float nx = c.x * 0.015f + mTime * 0.03f;
      float ny = c.y * 0.015f + mTime * 0.025f;
      c.vx += noise(nx, ny + 3.0f) * (0.25f + gustStrength * 0.4f) * dt;
      c.vy += noise(nx + 3.0f, ny) * (0.25f + gustStrength * 0.4f) * dt;
      c.vx *= 0.998f;
      c.vy *= 0.998f;
      if (c.x < 2.0f)  { c.x = 2.0f;  c.vx = -c.vx * 0.7f; }
      if (c.x > 254.0f){ c.x = 254.0f; c.vx = -c.vx * 0.7f; }
      if (c.y < 1.0f)  { c.y = 1.0f;  c.vy = -c.vy * 0.7f; }
      if (c.y > 63.0f) { c.y = 63.0f; c.vy = -c.vy * 0.7f; }
    }

    // ============================================================
    //  描画
    //
    //  layer1: 煙の大きな塊 (低周波 FBM)
    //  layer2: 繊維状のディテール（中周波 + ドメインワープ）
    //  layer3: 湿った micro テクスチャ
    //
    //  3 つの層とセル濃度をブレンドした後、
    //  sigmoid 調のコントラスト強調でメリハリをつける。
    // ============================================================

    // 突風で時間オフセットが一時的に加速（ベース3倍＝煙のスピード2倍）
    float gustSpeed = 9.0f + gustStrength * 1.5f;
    float t = mTime * gustSpeed;

    for (int y = 0; y < 64; y++)
    {
      for (int x = 0; x < 256; x++)
      {
        // ---- 層1: 煙の大きなうねり（ドメインワープ入り） ----
        float warp1x = noise(x * 0.03f + t * 0.08f, y * 0.03f + t * 0.07f) * 1.5f;
        float warp1y = noise(x * 0.03f + t * 0.08f + 5.0f, y * 0.03f + t * 0.07f + 5.0f) * 1.5f;
        float p1x = x * 0.01f + warp1x + t * 0.035f;
        float p1y = y * 0.01f + warp1y + t * 0.028f;
        float layer1 = fbm(p1x, p1y, 4);

        // ---- 層2: 繊維状ディテール（2重ドメインワープ → 溶け合う） ----
        float warp2a_x = noise(x * 0.04f + t * 0.10f, y * 0.04f + t * 0.08f) * 2.0f;
        float warp2a_y = noise(x * 0.04f + t * 0.10f + 11.0f, y * 0.04f + t * 0.08f + 11.0f) * 2.0f;
        // 二段階目のワープ（歪んだ座標をもう一度歪める）
        float warp2b_x = noise((x + warp2a_x) * 0.04f + t * 0.10f + 7.0f,
                               (y + warp2a_y) * 0.04f + t * 0.08f + 7.0f) * 1.5f;
        float warp2b_y = noise((x + warp2a_x) * 0.04f + t * 0.10f + 23.0f,
                               (y + warp2a_y) * 0.04f + t * 0.08f + 23.0f) * 1.5f;
        float p2x = x * 0.05f + warp2a_x + warp2b_x + t * 0.06f;
        float p2y = y * 0.05f + warp2a_y + warp2b_y + t * 0.05f;
        float layer2 = fbm(p2x, p2y, 3);

        // ---- 層3: 湿った細かいテクスチャ ----
        float warp3x = noise(x * 0.08f + t * 0.08f, y * 0.08f + t * 0.07f) * 1.2f;
        float warp3y = noise(x * 0.08f + t * 0.08f + 17.0f, y * 0.08f + t * 0.07f + 17.0f) * 1.2f;
        float p3x = x * 0.1f + warp3x + t * 0.09f;
        float p3y = y * 0.1f + warp3y + t * 0.08f;
        float layer3 = noise(p3x, p3y) * 0.5f + 0.5f * noise(p3x * 0.5f, p3y * 0.5f);

        // ---- ブレンド（セル弱め + ノイズの流れ） ----
        // セルは薄く乗せて丸みを残す
        float cellDensity = 0.0f;
        for (int i = 0; i < CELL_N; i++)
        {
          Cell_t &c = mCells[i];
          float dx = (float)x - c.x;
          float dy = (float)y - c.y;
          float dist2 = dx * dx + dy * dy;
          float r2 = c.radius * c.radius;
          cellDensity += c.intensity * r2 / (dist2 + r2 * 0.5f);
        }
        if (cellDensity > 3.0f) cellDensity = 3.0f;

        float value = layer1 * 0.5f + layer2 * 0.9f + layer3 * 0.1f + cellDensity * 0.1f;

        // ノーマライズ + コントラスト（溶け合い重視で弱め）
        value = (value + 1.0f) * 0.5f;     // -1..1 → 0..1
        value = value * 1.3f - 0.15f;      // コントラスト（溶け合い用）
        if (value < 0.0f) value = 0.0f;
        if (value > 1.0f) value = 1.0f;

        // ガンマ補間（中間トーンを広げて繊維感を強調）
        value = value * value;

        int gray = (int)(value * 15.0f + 0.5f);
        if (gray < 0) gray = 0;
        if (gray > 15) gray = 15;

        m.pixel((Color)gray, x, y);
      }
    }

    // ---- サブ画面 (128x64) — 気泡がぷかぷか浮かぶ ----
    // 濃淡表示ができないので白い dot のみ
    for (int i = 0; i < BUBBLE_N; i++)
    {
      Bubble_t &b = mBubbles[i];
      int bx = (int)b.x;
      int by = 63 - (int)b.y;  // Y反転
      int sz = (int)b.size;

      if (bx >= 0 && bx < 128 && by >= 0 && by < 64)
      {
        // 大きさに応じて dot を打つ
        // サイズ1: 1dot, サイズ2: 3dots (L字), サイズ3: 5dots (十字)
        s.pixel(WHITE, bx, by);
        if (sz >= 2)
        {
          if (bx + 1 < 128) s.pixel(WHITE, bx + 1, by);
          if (by + 1 < 64)  s.pixel(WHITE, bx, by + 1);
        }
        if (sz >= 3)
        {
          if (bx + 1 < 128 && by + 1 < 64) s.pixel(WHITE, bx + 1, by + 1);
          if (bx > 0)                      s.pixel(WHITE, bx - 1, by);
          if (by > 0)                      s.pixel(WHITE, bx, by - 1);
        }
      }
    }
  }

} /* namespace od */
