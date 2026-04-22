# The Use of Digital Image Processing for Detecting, Counting, and Locating Wormholes in Images.
**Course:** 261453 Digital Image Processing  
**Student:** Bancha Tongkhort    
___
## Abstract 
This report focuses on studying and understanding to apply digital image processing techniques for detecting, counting, and locating wormholes from the given images, using various techniques including Grayscale, Convolution, Median Blur, Thresholding, Morphological Operations, and Connected Component Labeling.  
___
## Dataset
- **Data/WormHole_1H.tif** - An eggplant image with only one wormhole.  

![WormHole_1H](.github/assets/WormHole_1H.png")

- **Data/WormHole_2H.tif** - An eggplant image with only two wormholes.

![WormHole_2H](.github/assets/WormHole_2H.png")

## Project Structure
```bash
IMG_hw3/
├── Data/
│   ├── WormHole_1H.tif
│   └── WormHole_2H.tif
├── include/
│   ├── CCL.h
│   ├── Image.h
│   ├── Preprocessing.h
│   ├── TIFFReader.h
│   ├── TIFFWriter.h
│   └── WormHole.h
├── src/
│   ├── CCL.cpp
│   ├── Image.cpp
│   ├── main.cpp
│   ├── Preprocessing.cpp
│   ├── TIFFReader.cpp
│   └── TIFFWriter.cpp
├── output/
└── Makefile
```
## System Architecture
