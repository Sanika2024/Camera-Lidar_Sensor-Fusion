import sys
from pathlib import Path
from PIL import Image

if len(sys.argv) < 3:
    print("Usage: python3 make_gif.py <frames_folder> <output_gif> [fps]")
    sys.exit(1)

frames_folder = Path(sys.argv[1])
output_gif = Path(sys.argv[2])

fps = int(sys.argv[3]) if len(sys.argv) > 3 else 10

files = sorted(
    list(frames_folder.glob("*.png")) +
    list(frames_folder.glob("*.jpg")) +
    list(frames_folder.glob("*.jpeg"))
)

if not files:
    print("No images found.")
    sys.exit(1)

images = [Image.open(f).convert("RGB") for f in files]

duration = int(1000 / fps)

images[0].save(
    output_gif,
    save_all=True,
    append_images=images[1:],
    duration=duration,
    loop=0
)

print(f"Created GIF: {output_gif}")
print(f"Frames: {len(images)}")
print(f"FPS: {fps}")
