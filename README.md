# NoMoreCatz
NoMoreCatz is an AI-based Chromium plugin, blocking pictures with cats on it.

## Requirements
- Any Chromium-based browser (Chrom, Edge, Brave, Opera, etc).
- 4 core CPU with AVX support.
- Windows 10 x64.

## Installation
- Download NoMoreCatz achive from [here](https://github.com/T-Troll/NoMoreCatz/releases).
- Unpack archive in any folder by you choice (please - no spaces in path, as well as national chars).
- Open browser.
- Select "Extensions" from browser settings.
- Enable "Developer mode" switch on Extension page.
- Three buttons appears, press "Load Unpacked" one.
- Select folder "plugin" inside the folder you unpack archive and press "Open Folder".
- Extention named "NoMoreCatz" shoild be added into extensions list.
- At the NoMoreCatz extension card, find the field "ID:" followed with the text string. Copy it or write it down.
- Open command line at the folder you unpack archive.
- Issue command `nomorecatz -register <ID string you write down from extenstion card>`
- Done!

## How it works?
Every picture browser download transferred to host application for check.<br>
If cat detected on the picture, plugin block it and replace to dummy picture.<br>

### Under the hood
Depends of building variant, host application uses full TensorFlow 2.3 (you need to download it sepatarely, it's HUGE) or TensorFlowLite to check picture data against trained model.<br>
For Tensorlow, it uses faster-rcnn model trained by Coco, Cat vs Dogs and Pets datasets.<br>
For TensorflowLite, SSD-resnet model, trained by Coco dataset used.
You can changed blocking type into `/plugin/background.js` script - it have variable for it at the beginning. Don't forget to reload extenstion after you modify it.

## Known issues
- Some pictures loaded by uncommon ways (f.e. from CSS) will be analyzed, but can't be blocked now. Will be fixed later.
- AI blocking is about probability, so it can block approximately 90% of images (for full TensorFlow) or even 70% (for TensorflowLite).
- If many images loaded from page, blocking will be delayed die to heavy load. Common blocking time is about 200ms (for Lite) + Chromium delay (depends on picture size, up to 400ms). For full Tensorflow delay can be up to 4 seconds!
- Full Tensorflow version require about 1Gb of RAM per process, so 16+Gb is reccomended. Lite version only uses around 100M per process.
- If full version compiled with GPU support, it also require CUDA 10.1 + CuDNN installed, and Nvidia GPU with at least 2Gb of VRAM.
