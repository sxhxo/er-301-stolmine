#include <od/graphics/screensavers/Mushroom.h>
#include <od/graphics/FrameBuffer.h>
#include <od/graphics/constants.h>
#include <hal/constants.h>
#include <cmath>
#include <cstdlib>

namespace od
{

  // ============================================================
  //  Mushroom v2 — 逆さまキノコ、高解像度、太い茎、
  //                胞子多め、蝶々付き
  //
  //  メイン画面 256x64 は上下反転して表示：
  //    天井 = 地面, 床 = 空
  //    キノコは天井からぶら下がる形
  // ============================================================

  // ---- Perlin noise ----

  float Mushroom::fade(float t)
  {
    return t * t * t * (t * (t * 6.0f - 15.0f) + 10.0f);
  }

  float Mushroom::lerp(float a, float b, float t)
  {
    return a + t * (b - a);
  }

  float Mushroom::grad(int hash, float x, float y)
  {
    int h = hash & 3;
    float u = (h < 2) ? x : y;
    float v = (h < 2) ? y : x;
    return (((h & 1) == 0) ? u : -u) + (((h & 2) == 0) ? v : -v);
  }

  float Mushroom::noise(float x, float y)
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

  float Mushroom::fbm(float x, float y, int octaves)
  {
    float value = 0.0f;
    float amp = 0.5f;
    float freq = 1.0f;
    for (int i = 0; i < octaves; i++)
    {
      value += amp * noise(x * freq, y * freq);
      freq *= 2.1f;
      amp *= 0.48f;
    }
    return value;
  }

  // ---- constructor / destructor ----

  Mushroom::Mushroom()
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

  Mushroom::~Mushroom() {}

  void Mushroom::reset()
  {
    mTime = 0.0f;
    mElapsed = 0.0f;

    // 地面ライン（天井側、上下反転に対応）
    for (int x = 0; x < 512; x++)
    {
      float nx = x * 0.025f;
      float ground = fbm(nx, 0.0f, 3);
      float h = 0.5f + ground * 0.15f;
      // 反転: groundY = y座標 0 が天井 → 地面は天井から 6〜14 ピクセル下
      mGroundY[x] = 6.0f + h * 14.0f;
    }

    for (int i = 0; i < MUSHROOM_N; i++) mMushrooms[i].age = -1.0f;
    for (int i = 0; i < SPORE_N; i++)    mSpores[i].life = 0.0f;

    for (int i = 0; i < 4; i++)
    {
      spawnMushroom(i);
      mMushrooms[i].age = (float)rand() / (float)RAND_MAX * 0.3f;
    }

    for (int i = 0; i < BUTTERFLY_N; i++) spawnButterfly(i);
  }

  void Mushroom::spawnMushroom(int idx)
  {
    mMushrooms[idx].x = 20.0f + (float)(rand() % 216);
    mMushrooms[idx].age = 0.0f;
    mMushrooms[idx].growthSpeed = 0.15f + (float)(rand() % 100) / 500.0f;
    // 高解像度: 茎を長く・太く
    float h = 20.0f + (float)(rand() % 40) * 0.6f;
    mMushrooms[idx].stalkHeight = h;
    mMushrooms[idx].capRadius = 5.0f + h * 0.35f + (float)(rand() % 10) * 0.4f;
    mMushrooms[idx].curvature = 0.3f + (float)(rand() % 100) / 200.0f;
    mMushrooms[idx].stalkWidth = 4.0f + (float)(rand() % 8) * 0.5f; // 倍
    mMushrooms[idx].wobble = (float)(rand() % 628) / 100.0f;
    mMushrooms[idx].lean = ((float)(rand() % 100) - 50.0f) / 100.0f;
  }

  void Mushroom::emitSpore(float x, float y)
  {
    for (int i = 0; i < SPORE_N; i++)
    {
      if (mSpores[i].life > 0.01f) continue;
      mSpores[i].x = x + ((float)(rand() % 100) - 50.0f) * 0.1f;
      mSpores[i].y = y;
      mSpores[i].vx = ((float)(rand() % 100) - 50.0f) * 0.03f;
      mSpores[i].vy = ((float)(rand() % 50) + 30.0f) * 0.03f; // 下向き(反転)
      mSpores[i].life = 0.3f + (float)(rand() % 100) / 150.0f;
      break;
    }
  }

  void Mushroom::spawnButterfly(int idx)
  {
    mButterflies[idx].x = (float)(rand() % 256);
    mButterflies[idx].y = 20.0f + (float)(rand() % 40);
    mButterflies[idx].vx = 0.5f + (float)(rand() % 100) * 0.01f;
    if (rand() % 2) mButterflies[idx].vx = -mButterflies[idx].vx;
    mButterflies[idx].vy = 0.0f;
    mButterflies[idx].phase = (float)(rand() % 628) / 100.0f;
    mButterflies[idx].freq = 3.0f + (float)(rand() % 50) * 0.1f;
    mButterflies[idx].hoverPhase = (float)(rand() % 628) / 100.0f;
  }

  void Mushroom::draw(FrameBuffer &m, FrameBuffer &s)
  {
    float dt = GRAPHICS_REFRESH_PERIOD;
    if (dt > 0.02f) dt = 0.018f;
    mTime += dt;
    mElapsed += dt;

    // ---- 地面ライン更新（高解像度 512px で計算 → 256px へダウンサンプル） ----
    float nt = mTime * 0.04f;
    for (int x = 0; x < 512; x++)
    {
      float nx = x * 0.025f;
      float ground = fbm(nx + nt * 0.1f, nt * 0.05f, 3);
      float h = 0.5f + ground * 0.15f;
      mGroundY[x] = 6.0f + h * 14.0f;
    }

    // ---- キノコ更新 ----
    int alive = 0;
    for (int i = 0; i < MUSHROOM_N; i++)
    {
      if (mMushrooms[i].age < 0.0f) continue;
      mMushrooms[i].age += dt * mMushrooms[i].growthSpeed;
      if (mMushrooms[i].age > 2.5f) { spawnMushroom(i); continue; }
      if (mMushrooms[i].age >= 0.0f && mMushrooms[i].age <= 1.5f) alive++;
    }
    if (alive < 3)
    {
      for (int i = 0; i < MUSHROOM_N; i++)
      {
        if (mMushrooms[i].age < 0.0f || mMushrooms[i].age > 2.0f)
        {
          spawnMushroom(i); alive++;
          if (alive >= 3) break;
        }
      }
    }

    // ---- 背景: 暗い星空風テクスチャ（Perlin noise） ----
    // 画面下（反転後は下＝空）に薄いグラデーション
    for (int y = 32; y < 64; y++)
    {
      for (int x = 0; x < 256; x += 2)
      {
        float nx = x * 0.04f;
        float ny = y * 0.04f;
        float val = fbm(nx + mTime * 0.02f, ny + mTime * 0.01f, 2);
        int g = (int)((val + 0.6f) * 2.5f);
        if (g < 0) g = 0; if (g > 4) g = 4;
        m.pixel((Color)g, x, y);
        if (x+1 < 256) m.pixel((Color)g, x+1, y);
      }
    }

    // ---- 地面（天井）の描画 ----
    for (int x = 0; x < 256; x++)
    {
      // サブピクセル補間: 2x 解像度の groundY を線形補間
      int ix = x * 2;
      float gy = mGroundY[ix];
      int gyi = (int)(gy + 0.5f);
      if (gyi >= 0 && gyi < 64)
      {
        // 地面の上（天井寄り）を土で塗る
        for (int dy = 0; dy < gyi; dy++)
        {
          float soilNx = x * 0.08f;
          float soilNy = dy * 0.08f;
          float soil = fbm(soilNx + mTime * 0.02f, soilNy + mTime * 0.015f, 2);
          int g = (int)((soil + 0.8f) * 2.0f);
          if (g < 1) g = 1; if (g > 5) g = 5;
          m.pixel((Color)g, x, dy);
        }
        // 輝線
        m.pixel(GRAY10, x, gyi);
      }
    }

    // ---- キノコ描画（逆さま: 天井からぶら下がる） ----
    for (int i = 0; i < MUSHROOM_N; i++)
    {
      Mushroom_t &shroom = mMushrooms[i];
      if (shroom.age < 0.0f) continue;

      float growthFactor;
      if (shroom.age < 1.0f) growthFactor = shroom.age;
      else if (shroom.age < 1.8f) growthFactor = 1.0f;
      else growthFactor = 1.0f - (shroom.age - 1.8f) / 0.7f * 0.7f;

      float currentHeight = shroom.stalkHeight * growthFactor;
      float currentCap = shroom.capRadius * (0.2f + 0.8f * growthFactor);
      float currentStalkW = shroom.stalkWidth * (0.3f + 0.7f * growthFactor);

      // 揺れ
      float sway = 2.5f * sinf(mTime * 1.2f + shroom.wobble) * growthFactor;
      float tipSway = 4.0f * sinf(mTime * 0.8f + shroom.wobble + 1.0f) * growthFactor;

      // 傾き
      float leanAngle = shroom.lean * (0.2f + 0.8f * growthFactor);
      if (shroom.age > 1.8f) leanAngle += ((shroom.age - 1.8f) / 0.7f) * shroom.lean * 0.5f;

      // 地面の高さ（天井側）→ サブピクセル補間
      int gx = (int)(shroom.x + 0.5f);
      if (gx < 0) gx = 0; if (gx > 255) gx = 255;
      int ix = gx * 2;
      float groundY = mGroundY[ix];
      // ↓ 隣の点も混ぜて補間
      if (ix + 1 < 512) groundY = (groundY + mGroundY[ix + 1]) * 0.5f;

      // ---- 茎（逆さま: 下向きに伸びる） ----
      int stalkSegments = (int)(currentHeight / 1.5f);
      if (stalkSegments < 1) stalkSegments = 1;

      for (int seg = 0; seg < stalkSegments; seg++)
      {
        float t = (float)seg / (float)stalkSegments;
        float nextT = (float)(seg + 1) / (float)stalkSegments;

        float curve = sinf(t * 3.14159f) * leanAngle * currentHeight * 0.5f;
        float nextCurve = sinf(nextT * 3.14159f) * leanAngle * currentHeight * 0.5f;

        float x0 = shroom.x + sway * t + curve;
        float x1 = shroom.x + sway * nextT + nextCurve;

        // 逆さま: +y 方向（下）に伸びる
        float y0 = groundY + t * currentHeight;
        float y1 = groundY + nextT * currentHeight;

        float w = currentStalkW * (1.0f - t * 0.4f);

        int stalkCol = (int)(5.0f + 5.0f * t);
        if (stalkCol > 12) stalkCol = 12;

        int ix0 = (int)(x0 + 0.5f);
        int iy0 = (int)(y0 + 0.5f);
        int ix1 = (int)(x1 + 0.5f);
        int iy1 = (int)(y1 + 0.5f);

        if (iy0 > 63) iy0 = 63; if (iy1 > 63) iy1 = 63;
        if (iy0 < 0)  iy0 = 0;  if (iy1 < 0)  iy1 = 0;

        m.line((Color)stalkCol, ix0, iy0, ix1, iy1);

        // 太い茎: 両側に線を追加
        int wpx = (int)(w + 0.5f);
        if (wpx > 0)
        {
          for (int side = 1; side <= wpx; side++)
          {
            int sideCol = stalkCol - side;
            if (sideCol < 2) sideCol = 2;
            m.line((Color)sideCol, ix0 + side, iy0, ix1 + side, iy1);
            m.line((Color)sideCol, ix0 - side, iy0, ix1 - side, iy1);
          }
        }
      }

      // ---- 傘（逆さま: 茎の先端は下、傘はさらに下） ----
      float capCenterX = shroom.x + sway + sinf(3.14159f) * leanAngle * currentHeight * 0.5f + tipSway * 0.5f;
      float capCenterY = groundY + currentHeight;

      int cxi = (int)(capCenterX + 0.5f);
      int cyi = (int)(capCenterY + 0.5f);

      int capR = (int)(currentCap + 0.5f);
      float curvatureFactor = shroom.curvature * (0.8f + 0.2f * growthFactor);

      if (capR >= 2 && cyi < 64)
      {
        // 傘の輪郭（下向き半円）
        for (int a = -capR; a <= capR; a++)
        {
          int px = cxi + a;
          if (px < 0 || px >= 256) continue;

          float ratio = (float)a / (float)capR;
          float capH = sqrtf(1.0f - ratio * ratio) * capR * curvatureFactor;
          // 逆さま: 傘のてっぺんはさらに下（y+）
          int py = cyi + (int)(capH * 0.5f);
          if (py >= 64) py = 63;

          float ageBright = shroom.age < 1.0f ? shroom.age : (shroom.age < 1.8f ? 1.0f : 1.0f - (shroom.age - 1.8f) * 1.5f);
          if (ageBright < 0.1f) ageBright = 0.1f;

          int capCol = (int)(6.0f + 8.0f * ageBright);
          if (capCol > 15) capCol = 15; if (capCol < 3) capCol = 3;

          int grad = (int)((1.0f - fabsf(ratio)) * 4.0f);
          int col = capCol - grad;
          if (col < 2) col = 2;

          m.pixel((Color)col, px, py);
          // 傘の下辺を2ピクセル
          if (py + 1 < 64)
            m.pixel((Color)(col > 3 ? col - 2 : 1), px, py + 1);
        }

        // てっぺんハイライト
        if (cxi >= 0 && cxi < 256)
        {
          int tipY = cyi + (int)(capR * curvatureFactor * 0.5f);
          if (tipY < 64) m.pixel(WHITE, cxi, tipY);
        }

        // 傘の斑点（明るい点）
        if (growthFactor > 0.5f)
        {
          int spots = capR;
          for (int s = 0; s < spots; s++)
          {
            int sx = cxi + (rand() % (capR * 2 + 1)) - capR;
            int sy = cyi + (rand() % (int)(capR * curvatureFactor * 0.5f));
            if (sx >= 0 && sx < 256 && sy >= 0 && sy < 64)
              m.pixel(GRAY12, sx, sy);
          }
        }
      }

      // ---- 胞子放出 ----
      if (shroom.age > 0.7f && shroom.age < 1.8f && capR > 2)
      {
        if ((rand() % 100) < 6)
        {
          float sporeX = capCenterX + ((float)(rand() % 100) - 50.0f) * 0.15f;
          float sporeY = capCenterY + 3.0f; // 傘の下（逆さま）
          emitSpore(sporeX, sporeY);
        }
      }
    }

    // ---- 胞子の更新・描画（逆さま: 重力は下向き） ----
    for (int i = 0; i < SPORE_N; i++)
    {
      Spore_t &spore = mSpores[i];
      if (spore.life <= 0.0f) continue;

      spore.x += spore.vx * dt * 20.0f;
      spore.y += spore.vy * dt * 20.0f;
      spore.vy += 0.15f * dt * 20.0f; // 重力（下）
      spore.life -= dt * 0.4f;

      float wind = 0.8f * sinf(mTime * 1.2f + (float)i * 0.5f);
      spore.vx += wind * dt;

      if (spore.y > 66.0f || spore.x < -5.0f || spore.x > 260.0f || spore.life <= 0.0f)
      {
        spore.life = 0.0f; continue;
      }

      int sx = (int)(spore.x + 0.5f);
      int sy = (int)(spore.y + 0.5f);
      if (sx >= 0 && sx < 256 && sy >= 0 && sy < 64)
      {
        int col = (int)(spore.life * 12.0f) + 1;
        if (col > 12) col = 12;
        m.pixel((Color)col, sx, sy);
        // ボケ効果
        if (sx+1 < 256) m.pixel(GRAY4, sx+1, sy);
        if (sy+1 < 64)  m.pixel(GRAY3, sx, sy+1);
      }
    }

    // ---- 蝶々 ----
    for (int i = 0; i < BUTTERFLY_N; i++)
    {
      Butterfly_t &b = mButterflies[i];

      b.phase += dt * b.freq;
      b.hoverPhase += dt * 1.5f;

      // 左右にふわふわ
      b.x += b.vx * dt * 30.0f;
      b.vy = 2.0f * sinf(b.hoverPhase);
      b.y += b.vy * dt * 20.0f;

      // 壁で反射
      if (b.x < 5.0f) { b.x = 5.0f; b.vx = -b.vx; }
      if (b.x > 250.0f) { b.x = 250.0f; b.vx = -b.vx; }
      if (b.y < 10.0f) { b.y = 10.0f; }
      if (b.y > 58.0f) { b.y = 58.0f; }

      int bx = (int)(b.x + 0.5f);
      int by = (int)(b.y + 0.5f);
      if (bx < 0 || bx >= 256 || by < 0 || by >= 64) continue;

      // 羽ばたき: 位相で羽の開き具合
      float wingAngle = fabsf(sinf(b.phase));
      int wingSpan = (int)(3.0f * wingAngle + 0.5f);
      if (wingSpan < 1) wingSpan = 1;

      // 蝶々のボディ
      m.pixel(WHITE, bx, by);

      // 羽（左）
      for (int w = 1; w <= wingSpan; w++)
      {
        int wy = by + (w % 2 == 0 ? -1 : 1);
        if (bx - w >= 0 && wy >= 0 && wy < 64)
          m.pixel(GRAY12, bx - w, wy);
      }

      // 羽（右）
      for (int w = 1; w <= wingSpan; w++)
      {
        int wy = by + (w % 2 == 0 ? 1 : -1);
        if (bx + w < 256 && wy >= 0 && wy < 64)
          m.pixel(GRAY12, bx + w, wy);
      }

      // たまに方向転換
      if ((rand() % 200) == 0)
      {
        b.vx = -b.vx;
        b.freq = 3.0f + (float)(rand() % 50) * 0.1f;
      }
    }

    // ---- サブ画面 (128x64) — 逆さまシルエット ----
    // 地面ライン
    for (int x = 0; x < 128; x++)
    {
      int gx = x * 4;
      float gy = (mGroundY[gx] + mGroundY[gx + 1]) * 0.5f;
      int gyi = (int)(gy * 0.5f + 0.5f);
      if (gyi >= 0 && gyi < 64) s.pixel(GRAY4, x, gyi);
    }

    // キノコシルエット
    for (int i = 0; i < MUSHROOM_N; i++)
    {
      Mushroom_t &shroom = mMushrooms[i];
      if (shroom.age < 0.0f) continue;

      float gf = shroom.age < 1.0f ? shroom.age : (shroom.age > 1.8f ? 1.0f - (shroom.age - 1.8f) * 0.7f : 1.0f);
      if (gf < 0.1f) continue;

      int gxi = (int)(shroom.x + 0.5f);
      if (gxi < 0) gxi = 0; if (gxi > 255) gxi = 255;

      float gy = mGroundY[gxi * 2];
      int sx = (int)(shroom.x * 0.5f);
      int sy = (int)((gy + shroom.stalkHeight * gf) * 0.5f);
      int sc = (int)(shroom.capRadius * 0.5f * gf);

      if (sx >= 0 && sx < 128 && sy >= 0 && sy < 64)
      {
        if (sc > 1) s.fillCircle(GRAY6, sx, sy, sc);
      }
    }
  }

} /* namespace od */
