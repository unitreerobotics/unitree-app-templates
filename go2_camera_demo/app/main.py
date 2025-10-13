from fastapi import FastAPI, Response, Query
from fastapi.responses import FileResponse
from fastapi.staticfiles import StaticFiles
from fastapi.middleware.cors import CORSMiddleware
import os
import sys
import io
from typing import Optional

# keep original SDK imports
from unitree_sdk2py.core.channel import ChannelFactoryInitialize
from unitree_sdk2py.go2.video.video_client import VideoClient

# Try to import Pillow for image processing; fall back if unavailable
try:
    from PIL import Image
    PIL_AVAILABLE = True
except Exception:
    PIL_AVAILABLE = False

app = FastAPI()

# CORS
app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],
    allow_credentials=True,
    allow_methods=["GET", "POST", "OPTIONS"],
    allow_headers=["*"],
)

# initialize SDK client once (same logic as before)
if len(sys.argv) > 1:
    ChannelFactoryInitialize(0, sys.argv[1])
else:
    try:
        ChannelFactoryInitialize(0)
    except Exception:
        pass

client = VideoClient()
client.SetTimeout(3.0)
try:
    client.Init()
except Exception:
    # initialization may fail in non-robot environment; keep app running
    pass

# UI directory
UI_DIR = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', 'ui'))

# Mount UI assets under /static (index served at root)
if os.path.isdir(UI_DIR):
    app.mount('/static', StaticFiles(directory=UI_DIR), name='static')

@app.get('/')
def index():
    index_path = os.path.join(UI_DIR, 'index.html')
    if os.path.exists(index_path):
        return FileResponse(index_path, media_type='text/html')
    return Response('index not found', status_code=404, media_type='text/plain')

@app.get('/api/image')
def get_image(w: Optional[int] = Query(None), q: int = Query(70, ge=10, le=95)):
    """Return latest camera image as image/jpeg bytes.
    Query params:
      - w: desired max width in pixels (will preserve aspect ratio)
      - q: JPEG quality (10-95)
    """
    try:
        code, data = client.GetImageSample()
    except Exception as e:
        return Response(f'camera error: {e}', status_code=500, media_type='text/plain')

    if code != 0 or not data:
        return Response('no image' + str(code), status_code=500, media_type='text/plain')

    img_bytes = bytes(data)

    # If Pillow is available and either width or quality requires processing, do it
    if PIL_AVAILABLE and (w is not None or (q and q < 95)):
        try:
            src = io.BytesIO(img_bytes)
            with Image.open(src) as im:
                # convert to RGB if needed (some cameras may produce b/w or RGBA)
                if im.mode not in ('RGB', 'L'):
                    im = im.convert('RGB')

                # resize by width while keeping aspect ratio
                if w:
                    try:
                        w = int(w)
                        max_w = max(1, min(1920, w))
                        if im.width > max_w:
                            new_h = int(im.height * (max_w / im.width))
                            im = im.resize((max_w, new_h), Image.LANCZOS)
                    except Exception:
                        pass

                out_buf = io.BytesIO()
                # quality: use optimize and progressive for smaller mobile-friendly size
                im.save(out_buf, format='JPEG', quality=int(q), optimize=True, progressive=True)
                out_buf.seek(0)
                return Response(content=out_buf.read(), media_type='image/jpeg')
        except Exception as e:
            # fall back to original bytes if processing fails
            try:
                return Response(content=img_bytes, media_type='image/jpeg')
            except Exception:
                return Response(f'processing error: {e}', status_code=500, media_type='text/plain')

    # If no processing required or Pillow not available, return original bytes
    return Response(content=img_bytes, media_type='image/jpeg')

if __name__ == '__main__':
    import uvicorn
    # use module path if running as script
    uvicorn.run('main:app', host='0.0.0.0', port=80, log_level='info')