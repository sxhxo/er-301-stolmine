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
    float mTime;
  };

} /* namespace od */
