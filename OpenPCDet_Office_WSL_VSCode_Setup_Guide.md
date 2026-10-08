# OpenPCDet Office Setup --- Windows + WSL2 + Ubuntu + VS Code

A from-scratch, offline-friendly checklist for setting up
OpenPCDet/PointPillars on a Windows office computer.

> **Important:** Do not blindly copy Python/PyTorch/CUDA versions from
> another computer. OpenPCDet contains compiled CUDA/C++ extensions.
> First identify the office GPU, Windows NVIDIA driver, Ubuntu version,
> Python version, and CUDA environment; then choose compatible
> PyTorch/spconv/toolkit versions.

## 1. What we are building

``` text
Windows
 ├─ NVIDIA Windows driver
 ├─ WSL 2
 │   └─ Ubuntu
 │       ├─ Linux build tools
 │       ├─ Python virtual environment
 │       ├─ PyTorch + CUDA
 │       ├─ spconv
 │       └─ OpenPCDet
 └─ VS Code + WSL extension
        └─ OpenPCDet project
```

Target:

``` text
Windows → WSL2 → Ubuntu → GPU visible → PyTorch CUDA
→ spconv → OpenPCDet CUDA extensions → PointPillars inference
```

## 2. Office-computer rules

-   Use Administrator privileges only when Windows requires them.
-   If company policy blocks WSL, virtualization, downloads, or driver
    changes, contact IT. Do not bypass policy.
-   Do **not** install a Linux NVIDIA display driver inside WSL.
-   Do **not** uninstall existing company software.
-   Do **not** randomly mix CUDA/PyTorch/spconv versions.
-   Do **not** upload company LiDAR, video, annotations, proprietary
    weights, credentials, or internal code to GitHub.
-   Do not run `wsl --unregister Ubuntu` casually; it can delete the
    Ubuntu distribution.
-   If one layer fails, fix that layer before continuing.

------------------------------------------------------------------------

# 3. CHECKPOINT 1 --- Windows

Open **PowerShell**.

Run:

``` powershell
winver
```

``` powershell
nvidia-smi
```

``` powershell
wsl --status
```

``` powershell
wsl --list --verbose
```

``` powershell
wsl --version
```

### Expected

For a suitable modern Windows machine, `winver` should show Windows 11
or a sufficiently recent Windows 10.

`nvidia-smi` should show an NVIDIA GPU, driver version and a CUDA
version supported by the driver.

`wsl --list --verbose` may show:

``` text
NAME      STATE      VERSION
Ubuntu    Stopped    2
```

### If `nvidia-smi` is not recognized

Open:

**Device Manager → Display adapters**

If an NVIDIA GPU exists but `nvidia-smi` fails, ask IT to verify the
NVIDIA driver. Do not install a random CUDA toolkit/driver.

### If `wsl` is not recognized

WSL is probably not installed. Continue to Section 4.

------------------------------------------------------------------------

# 4. CHECKPOINT 2 --- Install WSL2 + Ubuntu

Microsoft's recommended simple installation is:

**PowerShell as Administrator:**

``` powershell
wsl --install
```

Restart Windows if requested.

After restart:

``` powershell
wsl --status
```

``` powershell
wsl --list --verbose
```

The default installation normally installs Ubuntu and sets WSL2 as the
default.

### If `wsl --install` does not install Ubuntu

Run:

``` powershell
wsl --list --online
```

Then:

``` powershell
wsl --install -d Ubuntu
```

If normal installation hangs at 0.0%:

``` powershell
wsl --install --web-download -d Ubuntu
```

### If installation requires Administrator access

Stop and contact IT.

### If Windows is too old

Microsoft's simple command requires Windows 10 version 2004 / Build
19041 or later, or Windows 11. For an older system, ask IT to update
Windows or follow Microsoft's manual WSL installation procedure.

------------------------------------------------------------------------

# 5. CHECKPOINT 3 --- Confirm WSL2

Run:

``` powershell
wsl --list --verbose
```

Want:

``` text
Ubuntu    Stopped    2
```

If Ubuntu is version 1:

``` powershell
wsl --set-version Ubuntu 2
```

Then:

``` powershell
wsl --list --verbose
```

Optional:

``` powershell
wsl --set-default-version 2
```

------------------------------------------------------------------------

# 6. CHECKPOINT 4 --- First Ubuntu launch

Open:

**Start → Ubuntu**

Create a Linux username and password.

When typing the Linux password, nothing appears on screen. That is
normal.

Then run:

``` bash
cat /etc/os-release
```

``` bash
uname -a
```

``` bash
python3 --version
```

``` bash
git --version
```

### Expected

`cat /etc/os-release` should show Ubuntu and a `VERSION_ID`, such as
22.04 or 24.04.

Do not select CUDA/PyTorch versions just from the Ubuntu version.

------------------------------------------------------------------------

# 7. Update Ubuntu and install basic tools

Inside Ubuntu:

``` bash
sudo apt update
```

``` bash
sudo apt upgrade -y
```

``` bash
sudo apt install -y git wget curl build-essential cmake unzip pkg-config python3-venv python3-pip
```

Verify:

``` bash
git --version
```

``` bash
cmake --version
```

``` bash
python3 --version
```

### If `apt update` fails

Test:

``` bash
ping -c 3 google.com
```

If networking/DNS fails, the office firewall/proxy may be involved.
Contact IT instead of changing proxy settings randomly.

------------------------------------------------------------------------

# 8. CHECKPOINT 5 --- GPU inside WSL

First update WSL from **Windows PowerShell**:

``` powershell
wsl --update
```

Then:

``` powershell
wsl --shutdown
```

Start Ubuntu again.

Inside Ubuntu:

``` bash
nvidia-smi
```

### Expected

The NVIDIA GPU should be visible.

### Critical branch

**Windows `nvidia-smi` works + Ubuntu `nvidia-smi` works → continue.**

**Windows works + Ubuntu fails → STOP.**

Do not install:

``` bash
sudo apt install nvidia-driver-xxx
```

NVIDIA's CUDA-on-WSL documentation states that the Windows NVIDIA driver
supplies GPU support for WSL and that a separate Linux NVIDIA display
driver must not be installed inside WSL.

If WSL cannot see the GPU, ask IT to verify WSL2, virtualization, the
Windows NVIDIA driver and corporate restrictions.

------------------------------------------------------------------------

# 9. CHECKPOINT 6 --- VS Code

Install VS Code on **Windows**.

In VS Code install Microsoft's:

**WSL** extension.

Then:

``` text
Ctrl + Shift + P
→ WSL: Connect to WSL
```

A WSL-connected VS Code window should open.

Bottom-left should say something like:

``` text
WSL: Ubuntu
```

Open:

**Terminal → New Terminal**

Expected:

``` bash
username@computer:~$
```

Not:

``` text
PS C:\Users\...
```

### Rule

Use PowerShell for Windows/WSL administration:

``` powershell
wsl --status
wsl --list --verbose
wsl --update
wsl --shutdown
```

Use the VS Code WSL terminal for Linux/Python/OpenPCDet work.

------------------------------------------------------------------------

# 10. CHECKPOINT 7 --- Create the project location

In the VS Code WSL terminal:

``` bash
mkdir -p ~/projects
cd ~/projects
pwd
```

Expected:

``` text
/home/<username>/projects
```

Keep the active OpenPCDet source tree inside the Linux filesystem rather
than under `/mnt/c/...`.

Windows drives are still accessible through `/mnt/c`, `/mnt/d`, etc.

------------------------------------------------------------------------

# 11. CHECKPOINT 8 --- Python environment

Check:

``` bash
python3 --version
```

``` bash
python3.10 --version
```

If Python 3.10 exists:

``` bash
python3.10 -m venv ~/pcdet-env
```

Activate:

``` bash
source ~/pcdet-env/bin/activate
```

Expected prompt:

``` text
(pcdet-env) username@computer:~$
```

Verify:

``` bash
which python
```

Expected:

``` text
/home/<username>/pcdet-env/bin/python
```

Then:

``` bash
python --version
```

Upgrade packaging tools:

``` bash
python -m pip install --upgrade pip setuptools wheel
```

### If Python 3.10 does not exist

Do not randomly install a version. Record:

``` bash
python3 --version
cat /etc/os-release
```

Then choose the Python version together with the compatible
PyTorch/OpenPCDet environment.

------------------------------------------------------------------------

# 12. CHECKPOINT 9 --- Collect compatibility information

With the environment active, run:

``` bash
cat /etc/os-release
```

``` bash
nvidia-smi
```

``` bash
python --version
```

``` bash
nvcc --version
```

``` bash
which nvcc
```

### Important distinction

`nvidia-smi` reports information about the NVIDIA driver and the CUDA
level supported by the driver.

`nvcc --version` reports the CUDA toolkit/compiler installed inside
Linux.

They are not the same thing.

If `nvcc` is missing, do not automatically assume the GPU setup is
broken.

------------------------------------------------------------------------

# 13. CUDA toolkit rule

Do not install a CUDA toolkit just because `nvcc` is missing.

First determine the PyTorch/OpenPCDet/spconv combination required for
the office GPU.

If CUDA compilation requires `nvcc`, install a CUDA Toolkit version
compatible with that environment using NVIDIA's WSL-specific
instructions.

Avoid packages that attempt to install a Linux NVIDIA driver.

Afterwards:

``` bash
nvcc --version
```

should show a CUDA compilation-toolkit version.

------------------------------------------------------------------------

# 14. CHECKPOINT 10 --- Install PyTorch

Choose the PyTorch build only after checking:

``` text
GPU
NVIDIA driver
Ubuntu
Python
CUDA toolkit/compiler if needed
OpenPCDet
spconv
```

The general command has the form:

``` bash
pip install torch torchvision --index-url https://download.pytorch.org/whl/<CUDA_BUILD>
```

Use the exact command from the selected PyTorch build; do not literally
type `<CUDA_BUILD>`.

Then test:

``` bash
python -c "import torch; print('PyTorch:', torch.__version__); print('PyTorch CUDA:', torch.version.cuda); print('CUDA available:', torch.cuda.is_available()); print('GPU:', torch.cuda.get_device_name(0) if torch.cuda.is_available() else 'NOT AVAILABLE')"
```

### Success

You need:

``` text
CUDA available: True
GPU: <office GPU name>
```

### If `CUDA available: False`

STOP.

Run:

``` bash
nvidia-smi
```

and:

``` bash
python -c "import torch; print(torch.__version__); print(torch.version.cuda)"
```

If `nvidia-smi` works but PyTorch CUDA is false, you likely installed
the wrong/CPU PyTorch build or have a compatibility issue. Fix PyTorch
before OpenPCDet.

------------------------------------------------------------------------

# 15. CHECKPOINT 11 --- Clone OpenPCDet

With `(pcdet-env)` active:

``` bash
cd ~/projects
```

``` bash
git clone https://github.com/open-mmlab/OpenPCDet.git
```

``` bash
cd OpenPCDet
```

Verify:

``` bash
git status
```

``` bash
git log --oneline -1
```

------------------------------------------------------------------------

# 16. Inspect requirements before installing

Run:

``` bash
head -50 requirements.txt
```

``` bash
head -100 setup.py
```

Then install:

``` bash
pip install -r requirements.txt
```

If there is a dependency conflict, save the complete error. Do not
randomly uninstall packages.

------------------------------------------------------------------------

# 17. CHECKPOINT 12 --- Install spconv

OpenPCDet uses sparse convolution.

The spconv package must match the selected CUDA/PyTorch environment.

For example, a CUDA 12.4 environment may use a `spconv-cu124` package,
but do not install that package unless the actual selected environment
is compatible with it.

After installation:

``` bash
python -c "import spconv; print('spconv:', spconv.__version__)"
```

Expected:

``` text
spconv: <version>
```

------------------------------------------------------------------------

# 18. CHECKPOINT 13 --- Compile OpenPCDet

From:

``` bash
cd ~/projects/OpenPCDet
```

run:

``` bash
python setup.py develop
```

This is important because OpenPCDet contains custom C++/CUDA operations.

### If compilation fails

Do not immediately reinstall everything.

Run:

``` bash
python --version
```

``` bash
python -c "import torch; print(torch.__version__); print(torch.version.cuda); print(torch.cuda.is_available())"
```

``` bash
nvcc --version
```

``` bash
nvidia-smi
```

``` bash
python -c "import spconv; print(spconv.__version__)"
```

Then inspect the **first meaningful error** in the build output.

Common meanings:

-   `nvcc: command not found` → CUDA compiler/toolkit/PATH issue.
-   `unsupported gpu architecture` → CUDA toolkit/compiler may not
    support the GPU architecture.
-   `CUDA_HOME is not set` → CUDA toolkit path/configuration issue.
-   C++ compiler error → inspect `gcc --version` and `g++ --version`.
-   PyTorch/CUDA mismatch → compare `torch.version.cuda` with the
    selected toolkit/environment.
-   spconv import/build issue → compatibility problem.

------------------------------------------------------------------------

# 19. CHECKPOINT 14 --- Verify OpenPCDet

Run:

``` bash
python -c "import pcdet; print('OpenPCDet import: OK')"
```

Then:

``` bash
python -c "from pcdet.models import build_network; print('OpenPCDet model import: OK')"
```

Expected:

``` text
OpenPCDet import: OK
```

and:

``` text
OpenPCDet model import: OK
```

------------------------------------------------------------------------

# 20. CHECKPOINT 15 --- Verify CUDA extensions

Run:

``` bash
python -c "from pcdet.ops.iou3d_nms import iou3d_nms_cuda; print('iou3d_nms CUDA: OK')"
```

Then:

``` bash
python -c "from pcdet.ops.roiaware_pool3d import roiaware_pool3d_cuda; print('roiaware_pool3d CUDA: OK')"
```

Expected:

``` text
iou3d_nms CUDA: OK
roiaware_pool3d CUDA: OK
```

If these fail, the OpenPCDet custom CUDA build is not healthy yet.

------------------------------------------------------------------------

# 21. One-shot environment check

Create:

``` bash
cd ~/projects/OpenPCDet
nano check_pcdet_env.py
```

Paste:

``` python
import sys
import torch

print("=" * 60)
print("OpenPCDet Environment Check")
print("=" * 60)

print("Python:", sys.version)
print("PyTorch:", torch.__version__)
print("PyTorch CUDA:", torch.version.cuda)
print("CUDA available:", torch.cuda.is_available())

if torch.cuda.is_available():
    print("GPU:", torch.cuda.get_device_name(0))
    print("GPU count:", torch.cuda.device_count())
else:
    print("GPU: NOT AVAILABLE")

try:
    import spconv
    print("spconv:", spconv.__version__)
except Exception as e:
    print("spconv: FAILED")
    print(e)

try:
    import pcdet
    print("PCDet: IMPORT OK")
except Exception as e:
    print("PCDet: FAILED")
    print(e)

try:
    from pcdet.ops.iou3d_nms import iou3d_nms_cuda
    print("iou3d_nms CUDA: OK")
except Exception as e:
    print("iou3d_nms CUDA: FAILED")
    print(e)

try:
    from pcdet.ops.roiaware_pool3d import roiaware_pool3d_cuda
    print("roiaware_pool3d CUDA: OK")
except Exception as e:
    print("roiaware_pool3d CUDA: FAILED")
    print(e)

print("=" * 60)
```

Run:

``` bash
python check_pcdet_env.py
```

Ideal structure:

``` text
Python: ...
PyTorch: ...
PyTorch CUDA: ...
CUDA available: True
GPU: <office GPU>
GPU count: 1
spconv: ...
PCDet: IMPORT OK
iou3d_nms CUDA: OK
roiaware_pool3d CUDA: OK
```

------------------------------------------------------------------------

# 22. CHECKPOINT 16 --- Existing weights and data

The weights and data are already downloaded, so **do not download them
again**.

If they are on Windows:

``` text
C:\Users\<WindowsUser>\Downloads
```

they are accessible in WSL through:

``` text
/mnt/c/Users/<WindowsUser>/Downloads
```

Find them carefully.

Do not move them until you identify:

1.  dataset
2.  model architecture
3.  checkpoint
4.  matching configuration

Keep large data/checkpoints outside the Git repository where possible:

``` text
/home/<username>/
├── datasets/
├── models/
├── pcdet-env/
└── projects/
    └── OpenPCDet/
```

------------------------------------------------------------------------

# 23. Model/configuration matching

A checkpoint must match its model architecture.

Example:

``` text
PointPillars checkpoint
        ↓
PointPillars configuration
```

Do not use a PointPillars checkpoint with a SECOND/PV-RCNN
configuration.

Find PointPillars configs:

``` bash
cd ~/projects/OpenPCDet
find tools/cfgs -iname "*pillar*"
```

Identify the exact checkpoint/config pair before running inference.

------------------------------------------------------------------------

# 24. First goal: inference, not training

Do not begin by training a new model.

First target:

``` text
Known-compatible dataset
        ↓
Pretrained PointPillars
        ↓
OpenPCDet
        ↓
GPU inference
        ↓
3D detections
```

Only after that should you adapt the pipeline to your own `.pcd` data.

A custom `.pcd` file cannot automatically be used just by renaming it.
OpenPCDet may require a dataset loader/conversion step.

------------------------------------------------------------------------

# 25. Daily startup

After everything is installed, you do NOT reinstall it every day.

In VS Code WSL terminal:

``` bash
source ~/pcdet-env/bin/activate
```

``` bash
cd ~/projects/OpenPCDet
```

Check:

``` bash
nvidia-smi
```

Then run the required OpenPCDet command.

------------------------------------------------------------------------

# 26. Common problems

## A. WSL not installed

``` powershell
wsl --status
```

If missing:

``` powershell
wsl --install
```

Administrator access may be required.

## B. Ubuntu missing

``` powershell
wsl --list --online
```

then:

``` powershell
wsl --install -d Ubuntu
```

## C. Ubuntu is WSL1

``` powershell
wsl --set-version Ubuntu 2
```

## D. Windows GPU works, WSL GPU fails

``` powershell
wsl --shutdown
```

Start Ubuntu and retry:

``` bash
nvidia-smi
```

If still broken, contact IT. Do not install a Linux NVIDIA driver.

## E. VS Code terminal is PowerShell

Use:

``` text
Ctrl + Shift + P
→ WSL: Connect to WSL
```

Then open a new terminal.

## F. Python environment not active

``` bash
source ~/pcdet-env/bin/activate
```

## G. `python` is not the virtual environment

``` bash
which python
```

Expected:

``` text
/home/<username>/pcdet-env/bin/python
```

## H. `nvcc` missing

``` bash
which nvcc
```

A missing `nvcc` does not automatically mean GPU support is broken.
Determine whether compilation actually requires the toolkit and install
a compatible WSL CUDA toolkit if needed.

## I. PyTorch CUDA is false

``` bash
nvidia-smi
```

``` bash
python -c "import torch; print(torch.__version__); print(torch.version.cuda); print(torch.cuda.is_available())"
```

Fix PyTorch before OpenPCDet.

## J. OpenPCDet compilation fails

Check:

``` bash
python --version
```

``` bash
python -c "import torch; print(torch.__version__); print(torch.version.cuda)"
```

``` bash
nvcc --version
```

``` bash
python -c "import spconv; print(spconv.__version__)"
```

Fix the first compatibility error.

## K. `apt`/`pip` cannot download

Test:

``` bash
ping -c 3 google.com
```

Corporate proxy/firewall restrictions may be responsible.

## L. Do not unregister Ubuntu casually

Never use:

``` powershell
wsl --unregister Ubuntu
```

as routine troubleshooting. It can delete the Ubuntu filesystem.

------------------------------------------------------------------------

# 27. Diagnostic snapshot

Create a private log:

``` bash
mkdir -p ~/environment_logs
```

Then:

``` bash
{
echo "===== DATE ====="
date
echo "===== OS ====="
cat /etc/os-release
echo "===== KERNEL ====="
uname -a
echo "===== PYTHON ====="
python --version
which python
echo "===== NVIDIA ====="
nvidia-smi
echo "===== NVCC ====="
nvcc --version
echo "===== GIT ====="
git --version
echo "===== CMAKE ====="
cmake --version
echo "===== PYTORCH ====="
python -c "import torch; print(torch.__version__); print(torch.version.cuda); print(torch.cuda.is_available()); print(torch.cuda.get_device_name(0) if torch.cuda.is_available() else 'N/A')"
echo "===== SPCONV ====="
python -c "import spconv; print(spconv.__version__)"
echo "===== PCDET ====="
python -c "import pcdet; print(pcdet.__version__)"
} > ~/environment_logs/office_environment.txt 2>&1
```

Read:

``` bash
cat ~/environment_logs/office_environment.txt
```

Do not commit this log to GitHub if it contains company-sensitive
machine information.

------------------------------------------------------------------------

# 28. Recommended GitHub structure

``` text
OpenPCDet-Office-Setup/
├── README.md
├── docs/
│   └── OpenPCDet_Office_WSL_VSCode_Setup.md
├── scripts/
│   └── check_pcdet_env.py
├── configs/
│   └── README.md
└── .gitignore
```

Suggested `.gitignore`:

``` gitignore
__pycache__/
*.py[cod]
*.egg-info/

.venv/
venv/
pcdet-env/

data/
datasets/
*.pcd
*.bin
*.bag
*.pcap

*.pth
*.pt
*.ckpt

output/
outputs/
logs/
runs/

.vscode/
.DS_Store
Thumbs.db
```

Do not upload company data, proprietary weights, credentials, tokens,
internal code, or internal URLs.

------------------------------------------------------------------------

# 29. Final success checklist

-   [ ] Windows meets WSL2 requirements
-   [ ] WSL2 installed
-   [ ] Ubuntu installed
-   [ ] Ubuntu is WSL2
-   [ ] Windows `nvidia-smi` works
-   [ ] WSL `nvidia-smi` works
-   [ ] No Linux NVIDIA display driver installed in WSL
-   [ ] VS Code installed
-   [ ] WSL extension installed
-   [ ] VS Code connects to Ubuntu
-   [ ] VS Code terminal is Linux/Bash
-   [ ] Python virtual environment created
-   [ ] PyTorch installed
-   [ ] `torch.cuda.is_available()` is `True`
-   [ ] Correct office GPU appears in PyTorch
-   [ ] Compatible CUDA compiler/toolkit installed if compilation
    requires it
-   [ ] spconv imports
-   [ ] OpenPCDet imports
-   [ ] OpenPCDet CUDA extensions import
-   [ ] Dataset identified
-   [ ] Checkpoint identified
-   [ ] Configuration identified
-   [ ] Checkpoint/configuration match
-   [ ] First PointPillars inference completed

------------------------------------------------------------------------

# 30. Milestone order

Do this in exactly this order:

``` text
1. Windows check
        ↓
2. WSL2
        ↓
3. Ubuntu
        ↓
4. GPU visible in WSL
        ↓
5. VS Code + WSL
        ↓
6. Python environment
        ↓
7. Choose compatible PyTorch/CUDA
        ↓
8. PyTorch GPU test
        ↓
9. spconv
        ↓
10. OpenPCDet
        ↓
11. Compile CUDA extensions
        ↓
12. Environment verification
        ↓
13. Existing model + data
        ↓
14. PointPillars inference
        ↓
15. Adapt to custom .pcd data
```

**Never skip ahead to OpenPCDet if the earlier checkpoint is failing.**

------------------------------------------------------------------------

# 31. Official references

Microsoft WSL installation:
https://learn.microsoft.com/en-us/windows/wsl/install

Microsoft WSL basic commands:
https://learn.microsoft.com/en-us/windows/wsl/basic-commands

Microsoft WSL development environment:
https://learn.microsoft.com/en-us/windows/wsl/setup/environment

VS Code WSL: https://code.visualstudio.com/docs/remote/wsl

NVIDIA CUDA on WSL: https://docs.nvidia.com/cuda/wsl-user-guide/

OpenPCDet: https://github.com/open-mmlab/OpenPCDet

------------------------------------------------------------------------

## First commands on a fresh office machine

Start with only these in **Windows PowerShell**:

``` powershell
winver
nvidia-smi
wsl --status
wsl --list --verbose
```

If WSL is not installed and you have Administrator permission:

``` powershell
wsl --install
```

Restart if requested.

Then:

``` powershell
wsl --list --verbose
```

After Ubuntu starts, use the **VS Code WSL terminal** and run:

``` bash
cat /etc/os-release
nvidia-smi
python3 --version
git --version
```

That completes the initial discovery checkpoint.

**Do not guess the package versions before the office GPU/driver/Ubuntu
information is known.**
