# k230_v4l2_drm - Python bindings for V4L2 DRM capture

Python bindings for the v4l2-drm library, enabling efficient video capture
on K230 platforms with optional DRM display support.

## Features

- Zero-copy frame access (mmap/DMABUF)
- DRM display output for real-time preview
- Multi-camera support
- Rotation and flip control
- Crop support
- NumPy array integration

## Installation

The module is built as part of the `python-k230` package. Build with:

```bash
make python-k230-rebuild
```

## Quick Start

### Single Camera with Display

```python
from k230_v4l2_drm import Camera

# Create camera with DRM display
camera = Camera(device=0, width=1920, height=1080, format="NV12", display=True)
camera.start()

# Video appears on screen automatically
# Press Ctrl+C to stop
```

### Single Camera for Processing (No Display)

```python
from k230_v4l2_drm import Camera
import numpy as np

# Create camera without display
camera = Camera(device=0, width=640, height=480, format="NV12", display=False)
camera.start()
camera.start_streaming()

# Read frames
for i in range(100):
    if camera.read_frame(timeout_ms=1000):
        frame = camera.get_frame()  # numpy array
        print(f"Frame shape: {frame.shape}")
        # Process frame here...
        camera.release_frame()

camera.stop_streaming()
```

### Multi-Camera Setup

```python
from k230_v4l2_drm import V4L2DRM, Context

v4l2 = V4L2DRM()

# Camera 0
ctx0 = Context()
ctx0.device = 0
ctx0.width = 1920
ctx0.height = 1080
ctx0.video_format_str = "NV12"
ctx0.display = True
v4l2.add_context(ctx0)

# Camera 1
ctx1 = Context()
ctx1.device = 1
ctx1.width = 1280
ctx1.height = 720
ctx1.video_format_str = "NV12"
ctx1.display = True
ctx1.offset_x = 1920  # Display on right side
v4l2.add_context(ctx1)

v4l2.setup(use_display=True)
v4l2.run()  # Blocking call
```

## API Reference

### Camera Class

Convenience class for single-camera use.

```python
Camera(device, width, height, format="NV12", display=True)
```

**Parameters:**
- `device`: Video device number (0 for /dev/video0)
- `width`: Frame width
- `height`: Frame height
- `format`: Pixel format string ("NV12", "NV21", "NV16", "NV61", "YUYV", etc.)
- `display`: Enable DRM display output

**Methods:**
- `start()`: Initialize and setup camera
- `start_streaming()`: Start video stream
- `stop_streaming()`: Stop video stream
- `read_frame(timeout_ms)`: Wait for a frame
- `release_frame()`: Release current frame
- `get_frame()`: Get frame as numpy array with shape
- `get_frame_data()`: Get raw frame bytes
- `get_frame_count()`: Get captured frame count
- `set_rotation(rot)`: Set rotation (0, 90, 180, 270)
- `set_hflip(flip)`: Set horizontal flip (-1=disable, 0=off, 1=on)
- `set_vflip(flip)`: Set vertical flip (-1=disable, 0=off, 1=on)
- `set_offset(x, y)`: Set display offset
- `set_crop(w, h, ox, oy)`: Set crop parameters

### V4L2DRM Class

Low-level class for multi-camera and advanced use cases.

```python
V4L2DRM()
```

**Methods:**
- `add_context(ctx)`: Add a Context
- `clear_contexts()`: Remove all contexts
- `setup(use_display=True)`: Initialize all cameras
- `start(index)`: Start streaming for context
- `stop(index)`: Stop streaming for context
- `stop_all()`: Stop all contexts
- `dump_frame(index, timeout_ms)`: Get a frame
- `dump_release(index)`: Release frame
- `run(handler)`: Run streaming loop with callback
- `get_context(index)`: Get Context by index
- `get_context_count()`: Get number of contexts
- `is_running()`: Check if running
- `cleanup_global()`: Static method to cleanup display

### Context Class

Configuration for a single camera.

**Properties:**
- `device`: Video device number
- `width`, `height`: Resolution
- `video_format`: V4L2 pixel format (integer)
- `video_format_str`: Pixel format string
- `display_format`: DRM format (integer)
- `display_format_str`: DRM format string
- `display`: Enable DRM display
- `offset_x`, `offset_y`: Display offset
- `rotation`: Rotation (0-3)
- `hflip`, `vflip`: Flip settings
- `buffer_num`: Number of buffers
- `frame_count`: Captured frame count
- `video_fd`: Video file descriptor

### Rotation Enum

```python
from k230_v4l2_drm import Rotation

Rotation.ROTATION_0      # No rotation
Rotation.ROTATION_90     # 90 degrees clockwise
Rotation.ROTATION_180    # 180 degrees
Rotation.ROTATION_270    # 270 degrees clockwise
Rotation.REFLECT_X       # Horizontal reflection
Rotation.REFLECT_Y       # Vertical reflection
```

## Supported Formats

| Format | V4L2 Constant | DRM Constant |
|--------|---------------|--------------|
| NV12   | V4L2_PIX_FMT_NV12 | DRM_FORMAT_NV12 |
| NV21   | V4L2_PIX_FMT_NV21 | DRM_FORMAT_NV21 |
| NV16   | V4L2_PIX_FMT_NV16 | DRM_FORMAT_NV16 |
| NV61   | V4L2_PIX_FMT_NV61 | DRM_FORMAT_NV61 |
| BGR24  | V4L2_PIX_FMT_BGR24 | DRM_FORMAT_BGR888 |
| RGB24  | V4L2_PIX_FMT_RGB24 | DRM_FORMAT_RGB888 |
| YUYV   | V4L2_PIX_FMT_YUYV | DRM_FORMAT_YUYV |

## Frame Array Shapes

For different formats, `get_frame()` returns:

- **NV12/NV21**: `(height * 3/2, width)` - Y plane + interleaved UV
- **RGB24/BGR24**: `(height, width, 3)` - RGB triplets
- **YUYV**: `(height, width, 2)` - YUYV packed

Use `get_frame_data()` for raw 1D byte array.

## Notes

1. For display mode, call `run()` which handles the display loop
2. For non-display mode, manually call `read_frame()`/`release_frame()`
3. Always release frames after processing to avoid buffer starvation
4. Use `cleanup_global()` when completely done to release DRM resources

## License

MIT
