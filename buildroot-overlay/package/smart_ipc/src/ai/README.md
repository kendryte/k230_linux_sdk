# Where these files came from

Copied from `buildroot-overlay/package/face_detect/src/`, which is Canaan's
Linux port of the rt-smart example at
`canmv_k230/src/rtsmart/examples/ai/face_detection`. Same network
(retina-face on a 0.25-mobilenet backbone), same anchors, same post-processing,
same `face_detection_320.kmodel`.

They are copied rather than linked because buildroot builds each package from
its own rsync'd copy of the package directory, so `../../face_detect/src`
does not exist at build time.

| file | what it is |
|---|---|
| `ai_base.{cc,h}` | thin wrapper over the nncase interpreter: load kmodel, bind input/output tensors, run |
| `face_detection.{cc,h}` | ai2d pre-process, inference, NMS + anchor decode |
| `anchors_320.cc`, `anchors_640.cc` | the prior boxes, one table per input size |
| `utils.{cc,h}` | `padding_resize_one_side()` and friends, used to build the ai2d pipeline |
| `scoped_timing.h` | the per-stage timer the debug levels print |

## Local changes

Two, both in the drawing code, both marked with a comment at the top of
`face_detection.cc`:

1. `draw_result()` and `draw_result_video()` deleted. They scaled the boxes
   with the `SENSOR_WIDTH` / `SENSOR_HEIGHT` macros from `sensor_set.h`, which
   pins the AI capture resolution at compile time. `../ai_face.cpp` draws
   instead, at whatever `--ai-size` asked for, and it also has to turn the
   coordinates for a portrait panel.
2. The now-unneeded `#include "sensor_set.h"` went with them, so that header is
   not copied at all.

Nothing else is touched, so a future `diff -u` against `face_detect/src/` stays
readable.
