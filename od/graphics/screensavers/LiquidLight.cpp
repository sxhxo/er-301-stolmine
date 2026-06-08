#include <od/graphics/screensavers/LiquidLight.h>
#include <od/graphics/FrameBuffer.h>
#include <od/graphics/constants.h>
#include <hal/constants.h>
#include <cmath>
#include <cstdlib>

namespace od
{

  // ============================================================
  //  LiquidLight — 最小限バージョン（クラッシュ原因切り分け用）
  //  重いPerlin計算を全て削除し、シンプルな画面のみ
  // ============================================================

  // ---- constructor / destructor ----

  LiquidLight::LiquidLight()
  {
    mTime = 0.0f;
  }

  LiquidLight::~LiquidLight() {}

  void LiquidLight::reset()
  {
    mTime = 0.0f;
  }

  void LiquidLight::draw(FrameBuffer &m, FrameBuffer &s)
  {
    float dt = GRAPHICS_REFRESH_PERIOD;
    if (dt > 0.02f) dt = 0.018f;
    mTime += dt;

    // メイン画面：時間経過で明るさが変わる単色塗り（パフォーマンス確認用）
    float brightness = (sinf(mTime * 0.5f) + 1.0f) * 7.5f;
    if (brightness < 0.0f) brightness = 0.0f;
    if (brightness > 15.0f) brightness = 15.0f;
    int gray = (int)brightness;
    m.fill((Color)gray);

    // サブ画面：白で塗りつぶし
    s.fill(WHITE);
  }

} /* namespace od */
