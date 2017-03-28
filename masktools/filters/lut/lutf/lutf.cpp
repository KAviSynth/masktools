#include "lutf.h"
#include "../functions.h"

using namespace Filtering;

template<bool realtime, class T>
static void frame_c(Byte *dstp, ptrdiff_t dst_pitch, const Byte *srcp, ptrdiff_t src_pitch, const Byte *lutp, Parser::Context *ctx, int width, int height)
{
    T processor("");

    processor.reset();

    for (int j = 0; j < height; j++) {
        for (int i = 0; i < width; i++) {
            processor.add(dstp[i]);
        }
        dstp += dst_pitch;
    }

    dstp -= dst_pitch * height;

    // lutf: treat it as if the first frame was constant, result of avg/min/max, etc...
    if (realtime) {
      const Byte X = processor.finalize();
      // no 2D lut needed, X is constant
      // speedwise (expr = "x ymin - ymax ymin - / range_max *")
      // real lut: 180fps, 
      // realtime, compute_byte(X,srcp[i]) for each pixels: 6.3fps
      // realtime, miniLut: 173 fps
      Byte miniLut[256];
      for(int i=0; i<256;i++)
        miniLut[i] = ctx->compute_byte_xy(X, i);
      for (int j = 0; j < height; j++) {
        for (int i = 0; i < width; i++) {
          dstp[i] = miniLut[srcp[i]];
        }
        srcp += src_pitch;
        dstp += dst_pitch;
      }
    }
    else {
      const Byte *lut = lutp + (processor.finalize() << 8);

      for (int j = 0; j < height; j++) {
        for (int i = 0; i < width; i++) {
          dstp[i] = lut[srcp[i]];
        }
        srcp += src_pitch;
        dstp += dst_pitch;
      }
    }
}

template<bool realtime, int bits_per_pixel, class T>
static void frame16_c(Word *dstp, ptrdiff_t dst_pitch, const Word *srcp, ptrdiff_t src_pitch, const Word *lutp, Parser::Context *ctx, int width, int height)
{
  dst_pitch /= sizeof(uint16_t);
  src_pitch /= sizeof(uint16_t);

  T processor("");

  processor.reset();

  for (int j = 0; j < height; j++) {
    for (int i = 0; i < width; i++) {
      processor.add(dstp[i]);
    }
    dstp += dst_pitch;
  }

  dstp -= dst_pitch * height;

  const Word max_pixel_value = (1 << bits_per_pixel) - 1;

  // lutf: treat it as if the first frame was constant, result of avg/min/max, etc...
  if (realtime) {
    const Word X = processor.finalize();
    // no 2D lut needed, X is constant
    // speedwise (expr = "x ymin - ymax ymin - / range_max *")
    // real lut: 180fps, 
    // realtime, compute_byte(X,srcp[i]) for each pixels: 6.3fps
    // realtime, miniLut: 173 fps
    Word miniLut[65536]; // full 16, anti overflow
    const int real_buf_size = (1 << bits_per_pixel);
    for (int i = 0; i < real_buf_size; i++)
      miniLut[i] = ctx->compute_word_xy<bits_per_pixel>(X, i);
    for (int i = real_buf_size; i < 65536; i++)
      miniLut[i] = max_pixel_value;

    for (int j = 0; j < height; j++) {
      for (int i = 0; i < width; i++) {
        dstp[i] = miniLut[srcp[i]];
      }
      srcp += src_pitch;
      dstp += dst_pitch;
    }
  }
  else {
    const Word *lut = lutp + (min(processor.finalize(),max_pixel_value) << bits_per_pixel);

    for (int j = 0; j < height; j++) {
      for (int i = 0; i < width; i++) {
        dstp[i] = bits_per_pixel == 16 ? lut[srcp[i]] : lut[srcp[i] > max_pixel_value ? max_pixel_value : srcp[i]];
      }
      srcp += src_pitch;
      dstp += dst_pitch;
    }
  }
}


template<class T>
static void frame32_c(Float *dstp, ptrdiff_t dst_pitch, const Float *srcp, ptrdiff_t src_pitch, Parser::Context *ctx, int width, int height)
{
  dst_pitch /= sizeof(Float);
  src_pitch /= sizeof(Float);

  T processor("");

  processor.reset();

  for (int j = 0; j < height; j++) {
    for (int i = 0; i < width; i++) {
      processor.add(dstp[i]);
    }
    dstp += dst_pitch;
  }

  dstp -= dst_pitch * height;

  // lutf: treat it as if the first frame was constant, result of avg/min/max, etc...
  const Float X = processor.finalize();

  // always full realtime
  for (int j = 0; j < height; j++) {
    for (int i = 0; i < width; i++) {
      dstp[i] = ctx->compute_float(X, srcp[i], -1.0, -1.0, 32);
    }
    srcp += src_pitch;
    dstp += dst_pitch;
  }
}

namespace Filtering { namespace MaskTools { namespace Filters { namespace Lut { namespace Frame {

Processor *processors_array[NUM_MODES] = MPROCESSOR_SINGLE(frame_c, false);
//Processor *processors_array[NUM_MODES] = { &frame_c< false, Nonizer >, &frame_c< false, Averager<int> >, &frame_c< false, Minimizer >, &frame_c< false, Maximizer >, &frame_c< false, Deviater<int> >, &frame_c< false, Rangizer >, &frame_c< false, Medianizer >, &frame_c< false, MedianizerBetter<4> >, &frame_c< false, MedianizerBetter<6> >, &frame_c< false, MedianizerBetter<2> >, };
Processor *processorsCtx_array[NUM_MODES] = MPROCESSOR_SINGLE(frame_c, true);

Processor16 *processors10_array[NUM_MODES] = MPROCESSOR16_SINGLE(frame16_c, false, 10);
Processor16 *processors10Ctx_array[NUM_MODES] = MPROCESSOR16_SINGLE(frame16_c, true, 10);
Processor16 *processors12_array[NUM_MODES] = MPROCESSOR16_SINGLE(frame16_c, false, 12);
Processor16 *processors12Ctx_array[NUM_MODES] = MPROCESSOR16_SINGLE(frame16_c, true, 12);
Processor16 *processors14_array[NUM_MODES] = MPROCESSOR16_SINGLE(frame16_c, false, 14);
Processor16 *processors14Ctx_array[NUM_MODES] = MPROCESSOR16_SINGLE(frame16_c, true, 14);
Processor16 *processors16_array[NUM_MODES] = MPROCESSOR16_SINGLE(frame16_c, false, 16);
Processor16 *processors16Ctx_array[NUM_MODES] = MPROCESSOR16_SINGLE(frame16_c, true, 16);

Processor32 *processors32Ctx_array[NUM_MODES] = MPROCESSOR32_SINGLE(frame32_c);

} } } } }

