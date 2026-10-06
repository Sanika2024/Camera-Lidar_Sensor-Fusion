# Office Setup Guide — Camera–LiDAR Sensor Fusion

This guide explains how to clone, configure, verify, build, and run the **Camera–LiDAR Sensor Fusion** project on an office Windows laptop using **WSL2 + Ubuntu**.

**GitHub repository:** https://github.com/Sanika2024/Camera-Lidar_Sensor_Fusion

## 1. Project Overview

The pipeline is organized into six stages:

```text
RAW SENSOR DATA
       │
       ├───────────────┐
       ▼               ▼
   STAGE 0           LiDAR
 Raw Camera            │
       │               │
       ▼               ▼
   STAGE 1           STAGE 2
     YOLO          Raw Point Cloud
       │               │
       └───────┬───────┘
               ▼
           STAGE 3
    LiDAR → Camera Image
               │
               ▼
           STAGE 4
       SENSOR FUSION
 Camera + LiDAR Association
               │
               ▼
           STAGE 5
      TARGET VEHICLE
               │
               ▼
        FINAL RESULTS
```

| Stage | Purpose |
|---|---|
| Stage 0 | Raw KITTI camera images |
| Stage 1 | YOLO vehicle detection |
| Stage 2 | Raw LiDAR point cloud |
| Stage 3 | LiDAR-to-camera projection |
| Stage 4 | Camera–LiDAR association for all detected vehicles |
| Stage 5 | Preceding/target vehicle and associated LiDAR visualization |

## 2. GitHub vs Local Files

### Included in GitHub
- C++ source code
- Stage implementations
- CMake configuration
- YOLO configuration/class files
- Python GIF utility
- Documentation
- Preview GIFs

### Download/generated locally
- KITTI dataset
- `dat/yolo/yolov3.weights`
- `build/`
- generated `results/`

The large files are intentionally excluded by `.gitignore`.

## 3. Required Environment

| Component | Requirement |
|---|---|
| OS | Windows + WSL2 |
| Linux | Ubuntu |
| Architecture | x86-64 |
| Compiler | GCC/G++ |
| CMake | 4.x |
| OpenCV | **4.x** |
| Git | Required |
| Python | Python 3 |
| pkg-config | Required |
| GitHub CLI | Optional |

The known working setup uses OpenCV 4.10.0.

**Do not deliberately switch to OpenCV 5.x.** The original Udacity code uses APIs that can be incompatible with OpenCV 5 changes.

CUDA/GPU is not required for the current pipeline.

## 4. Check WSL2

Open PowerShell:

```powershell
wsl --status
wsl --version
wsl -l -v
```

Ubuntu should show `VERSION 2`.

## 5. Enter Ubuntu and Check System

```bash
lsb_release -a
uname -m
uname -a
```

Expected architecture:

```text
x86_64
```

## 6. Install Development Tools

```bash
sudo apt update
sudo apt install -y build-essential cmake git pkg-config python3 python3-pip wget curl unzip
```

Verify:

```bash
g++ --version
gcc --version
cmake --version
git --version
python3 --version
pkg-config --version
```

## 7. Verify OpenCV

```bash
pkg-config --modversion opencv4
```

Expected format:

```text
4.x.x
```

The known working version is:

```text
4.10.0
```

If `opencv4` is not found, stop and fix the OpenCV installation before building. Do not randomly install OpenCV 5.x.

## 8. Clone the Repository

```bash
cd ~
git clone https://github.com/Sanika2024/Camera-Lidar_Sensor_Fusion.git
cd Camera-Lidar_Sensor_Fusion
```

Check:

```bash
ls
```

Expected:

```text
CMakeLists.txt
ENVIRONMENT.md
README.md
dat
images
results_preview
src
stage_code
tools
```

Verify Git:

```bash
git status
git log --oneline -1
```

## 9. Download YOLOv3 Weights

Official YOLOv3 page:

https://pjreddie.com/darknet/yolo/

Direct weights:

https://data.pjreddie.com/files/yolov3.weights

From the project root:

```bash
curl -L "https://data.pjreddie.com/files/yolov3.weights" -o "dat/yolo/yolov3.weights"
```

Or:

```bash
wget https://data.pjreddie.com/files/yolov3.weights -O dat/yolo/yolov3.weights
```

Verify:

```bash
ls -lh dat/yolo/yolov3.weights
```

Expected size is approximately **237 MB**.

Optional SHA-256 check:

```text
523e4e69e1d015393a1b0a441cef1d9c7659e3eb2d7e15f793f060a21b32f297
```

Run:

```bash
sha256sum dat/yolo/yolov3.weights
```

The current pipeline does not require `yolov3-tiny.weights`.

## 10. Download KITTI Dataset

Official KITTI raw-data page:

https://www.cvlibs.net/datasets/kitti/raw_data.php

Official KITTI site:

https://www.cvlibs.net/datasets/kitti/

Download the appropriate KITTI raw sequence/data corresponding to the project.

The project expects:

```text
images/
└── KITTI/
    └── 2011_09_26/
        ├── image_02/
        │   └── data/
        │       ├── 0000000000.png
        │       ├── 0000000001.png
        │       └── ...
        └── velodyne_points/
            └── data/
                ├── 0000000000.bin
                ├── 0000000001.bin
                └── ...
```

The camera files must be directly under:

```text
images/KITTI/2011_09_26/image_02/data/
```

and LiDAR files directly under:

```text
images/KITTI/2011_09_26/velodyne_points/data/
```

Avoid an extra unwanted directory level.

## 11. Verify KITTI

Camera count:

```bash
find images/KITTI/2011_09_26/image_02/data -name "*.png" | wc -l
```

Expected for the current experiment:

```text
78
```

LiDAR count:

```bash
find images/KITTI/2011_09_26/velodyne_points/data -name "*.bin" | wc -l
```

Expected:

```text
78
```

Check filenames:

```bash
ls images/KITTI/2011_09_26/image_02/data | head
ls images/KITTI/2011_09_26/velodyne_points/data | head
```

Expected matching pattern:

```text
0000000000.png ↔ 0000000000.bin
0000000001.png ↔ 0000000001.bin
...
0000000077.png ↔ 0000000077.bin
```

## 12. Build

From the project root:

```bash
rm -rf build
mkdir build
cd build
cmake .. -DCMAKE_POLICY_VERSION_MINIMUM=3.5
cmake --build . -j$(nproc)
```

Verify the executables:

```bash
ls
```

Expected:

```text
Stage00_RawCamera
Stage01_YOLO
Stage02_LiDAR
Stage03_LiDARProjection
Stage04_SensorFusion
Stage05_TargetVehicle
```

## 13. First Test: One Frame

Before processing all 78 frames, test one frame.

In the relevant stage source files, temporarily use:

```cpp
imgStartIndex = 0;
imgEndIndex = 0;
imgStepWidth = 1;
```

Rebuild:

```bash
cd build
cmake --build . -j$(nproc)
```

Then run the stages.

### Stage 0

```bash
./Stage00_RawCamera
```

### Stage 1

```bash
./Stage01_YOLO
```

### Stage 2

```bash
./Stage02_LiDAR
```

### Stage 3

```bash
./Stage03_LiDARProjection
```

### Stage 4

```bash
./Stage04_SensorFusion
```

Stage 4 associates projected LiDAR with **all detected vehicle bounding boxes**. It does not select only the preceding vehicle.

### Stage 5

```bash
./Stage05_TargetVehicle
```

Stage 5 produces the clean target/preceding-vehicle visualization with its associated LiDAR points.

## 14. Generated Results

Outputs are stored under:

```text
results/
└── stages/
    ├── 00_raw_camera/
    ├── 01_yolo_all_vehicles/
    ├── 02_lidar_detection/
    ├── 03_lidar_projection/
    ├── 04_sensor_fusion/
    └── 05_target_vehicle/
```

`results/` is intentionally ignored by Git.

## 15. Second Test: 19 Frames

After frame 0 works, use:

```cpp
imgStartIndex = 0;
imgEndIndex = 18;
imgStepWidth = 1;
```

This processes 19 frames.

Rebuild:

```bash
cd build
cmake --build . -j$(nproc)
```

Then run the stages again.

## 16. Full 78-Frame Run

Once the 19-frame test succeeds, use:

```cpp
imgStartIndex = 0;
imgEndIndex = 77;
imgStepWidth = 1;
```

This processes:

```text
0, 1, 2, ..., 77
```

= **78 frames**.

Rebuild:

```bash
cd build
cmake --build . -j$(nproc)
```

Run in order:

```bash
./Stage00_RawCamera
./Stage01_YOLO
./Stage02_LiDAR
./Stage03_LiDARProjection
./Stage04_SensorFusion
./Stage05_TargetVehicle
```

## 17. Generate GIFs

Use the existing repository utility:

```text
tools/make_gif.py
```

Do not create a separate ad-hoc GIF script.

Example:

```bash
python3 tools/make_gif.py results/stages/01_yolo_all_vehicles/frames results/stages/01_yolo_all_vehicles.gif 5
```

Stage 5 example:

```bash
python3 tools/make_gif.py results/stages/05_target_vehicle/frames results/stages/05_target_vehicle.gif 5
```

## 18. Troubleshooting

### OpenCV not found

```bash
pkg-config --modversion opencv4
```

If it fails, fix OpenCV 4.x before continuing.

### CMake policy error

Use:

```bash
cmake .. -DCMAKE_POLICY_VERSION_MINIMUM=3.5
```

### YOLO weights missing

```bash
ls -lh dat/yolo/yolov3.weights
```

The file must be:

```text
dat/yolo/yolov3.weights
```

### YOLO file is only a few bytes

It may be a Git LFS pointer. Download the actual weights:

```bash
curl -L "https://data.pjreddie.com/files/yolov3.weights" -o "dat/yolo/yolov3.weights"
```

### KITTI image cannot be opened

Check:

```bash
ls images/KITTI/2011_09_26/image_02/data | head
```

### LiDAR data missing

Check:

```bash
ls images/KITTI/2011_09_26/velodyne_points/data | head
```

### Camera/LiDAR counts differ

```bash
find images/KITTI/2011_09_26/image_02/data -name "*.png" | wc -l
find images/KITTI/2011_09_26/velodyne_points/data -name "*.bin" | wc -l
```

Both should be 78 for the current experiment.

### CMake cache problem

Delete and rebuild:

```bash
rm -rf build
mkdir build
cd build
cmake .. -DCMAKE_POLICY_VERSION_MINIMUM=3.5
cmake --build . -j$(nproc)
```

## 19. Recommended Office Setup Sequence

```text
1. Verify WSL2
2. Open Ubuntu
3. Install development tools
4. Verify GCC/G++/CMake/Git/Python
5. Verify OpenCV 4.x
6. Clone GitHub repository
7. Download YOLOv3 weights
8. Download/extract KITTI data
9. Verify 78 camera + 78 LiDAR frames
10. Build with CMake
11. Test frame 0
12. Test frames 0–18
13. Run frames 0–77
14. Generate GIFs/results if required
```

## 20. Files That Must Not Be Pushed

Do not commit:

```text
images/KITTI/
dat/yolo/yolov3.weights
dat/yolo/yolov3-tiny.weights
build/
results/
```

These are already excluded by `.gitignore`.

For source/documentation changes:

```bash
git status
git add .
git commit -m "Describe the change"
git push
```

## 21. Final Verification Checklist

### Environment
- [ ] WSL2 working
- [ ] Ubuntu working
- [ ] GCC/G++ installed
- [ ] CMake 4.x installed
- [ ] Git installed
- [ ] Python 3 installed
- [ ] OpenCV 4.x available through `pkg-config`

### Repository
- [ ] GitHub repository cloned
- [ ] `README.md` available
- [ ] `ENVIRONMENT.md` available
- [ ] `CMakeLists.txt` available
- [ ] `stage_code/` available

### YOLO
- [ ] `coco.names` exists
- [ ] `yolov3.cfg` exists
- [ ] `yolov3.weights` downloaded
- [ ] Weight size approximately 237 MB
- [ ] SHA-256 verified if required

### KITTI
- [ ] Camera data exists
- [ ] LiDAR data exists
- [ ] 78 PNG files available
- [ ] 78 BIN files available
- [ ] Camera/LiDAR frame numbers match

### Build
- [ ] CMake configuration succeeds
- [ ] Compilation succeeds
- [ ] Six stage executables generated

### Execution
- [ ] Stage 0 works
- [ ] Stage 1 works
- [ ] Stage 2 works
- [ ] Stage 3 works
- [ ] Stage 4 works
- [ ] Stage 5 works
- [ ] 19-frame test completed
- [ ] 78-frame run completed

## 22. Final Office Project Structure

```text
Camera-Lidar_Sensor_Fusion/
│
├── .gitignore
├── CMakeLists.txt
├── README.md
├── ENVIRONMENT.md
├── OFFICE_SETUP_GUIDE.md
│
├── dat/
│   └── yolo/
│       ├── coco.names
│       ├── yolov3.cfg
│       ├── yolov3-tiny.cfg
│       └── yolov3.weights
│
├── images/
│   ├── course_code_structure.png
│   └── KITTI/
│       └── 2011_09_26/
│           ├── image_02/
│           │   └── data/
│           └── velodyne_points/
│               └── data/
│
├── results_preview/
├── src/
├── stage_code/
│   ├── 00_raw_camera/
│   ├── 01_yolo_all_vehicles/
│   ├── 02_lidar_detection/
│   ├── 03_lidar_projection/
│   ├── 04_sensor_fusion/
│   └── 05_target_vehicle/
├── tools/
│   └── make_gif.py
├── build/
└── results/
```
