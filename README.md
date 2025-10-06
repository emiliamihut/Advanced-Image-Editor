# Image Manipulation Application

An image editor for PGM/PPM (P2, P3, P5, P6) that loads images, selects areas, crops, rotates by 90/180/270 degrees, applies filters (EDGE, SHARPEN, BLUR, GAUSSIAN_BLUR), builds histograms, performs histogram equalization, and saves results. Operations are modular with explicit memory management for pixel matrices and selections [attached_file:1].

## Features

- Load and save ASCII/binary formats (P2/P3/P5/P6) including headers and pixel matrices [attached_file:1].
- Selection: select_area(x1,y1,x2,y2) with bounds validation and normalized coordinate order; select_all for full image [attached_file:1].
- Crop: keep only the selected region, reallocate pixel matrix, update dimensions and selection [attached_file:1].
- Rotate: 90/180/270 degrees for whole image or square selection, with angle normalization (mod 360) [attached_file:1].
- Filters: EDGE, SHARPEN, BLUR, GAUSSIAN_BLUR via convolution kernel and divisor, skipping borders and clamping output [attached_file:1].
- Histogram: grayscale only, bins as power of two, bin size = (max_val+1)/bins, star-bar scaled to requested width [attached_file:1].
- Equalize: grayscale contrast via cumulative histogram mapping new = (cum_freq/total_pixels)*255 [attached_file:1].

## Data Structures

- pixel: r,g,b for color or l for grayscale intensity [attached_file:1].
- image: pixels matrix, width, height, max_val, format, selection coordinates x1,y1,x2,y2 [attached_file:1].

## Commands

- LOAD: parse format, dimensions, max pixel value; read pixels (ASCII/binary); grayscale mapped to l, color to r,g,b [attached_file:1].
- SELECT: select_area(x1,y1,x2,y2) validates bounds and normalizes coordinates; select_all covers entire image [attached_file:1].
- CROP: allocate new matrix for selection, free old matrix, update image size and selection [attached_file:1].
- ROTATE: 0/90/180/270; selection must be square for partial rotate; index remaps for each angle; rotate_full/rotate_select variants [attached_file:1].
- APPLY: EDGE | SHARPEN | BLUR | GAUSSIAN_BLUR; build kernel/divisor, skip borders, normalize and clamp [attached_file:1].
- HISTOGRAM: grayscale only; bins = power of 2; bin size = (max_val+1)/bins; star-bar scaled by requested width [attached_file:1].
- EQUALIZE: compute histogram and CDF; map value = (cum_freq/total_pixels)*255 [attached_file:1].
- SAVE: write chosen ASCII/binary format with header and pixel matrix [attached_file:1].

## I/O and Robustness

- Input buffer is cleared after each command using a getchar loop to discard leftovers [attached_file:1].
- Parameters parsed via sscanf to validate counts and detect extra characters; invalid inputs are handled gracefully [attached_file:1].
- Main loop reads commands, dispatches operations, and guards incomplete/invalid cases [attached_file:1].

## Usage

1. Run the program and enter commands (e.g., LOAD, SELECT, CROP, ROTATE 90, APPLY BLUR, HISTOGRAM, EQUALIZE, SAVE, EXIT) [attached_file:1].
2. For LOAD/SAVE, specify the format and path; operations like SELECT and ROTATE accept parameters as described above [attached_file:1].
3. After each command, input leftovers are cleared to prevent blocking on subsequent reads [attached_file:1].

## Implementation Notes

- Angles: negative angles adjusted by +360, then modulo 360 [attached_file:1].
- Rotation mappings:
  - 90°: new[i][j] = old[new_w - j - 1][i]
  - 180°: new[i][j] = old[new_h - i - 1][new_w - j - 1]
  - 270°: new[i][j] = old[j][new_h - i - 1]
- Filters: convolution with kernel and divisor, clamp outputs to valid range [attached_file:1].

## Formats

- ASCII: P2 (grayscale), P3 (color) [attached_file:1].
- Binary: P5 (grayscale), P6 (color) [attached_file:1].

## Requirements

- Standard C toolchain to build and run the console application [attached_file:1].
