
# VFE - Video Frame Extractor

**Version**: 1.0  
**Author**: sudoXpg  
**License**: []

## Overview

`VFE` (Video Frame Extractor) is a command-line tool built using the FFmpeg library that allows users to extract frames from a video file. It supports custom frame extraction based on the number of frames or a specific time range. The tool also includes options for specifying output directories, enabling verbose mode, and more.

### Features
- Extract all frames or a specified number of frames from any video file.
- Specify a custom time frame to extract frames between specific start and end times.
- Custom output directory creation to store extracted frames.
- Verbose mode for detailed log output.
- Process indication with less detailed output for casual usage.
- Time tracking to display the total time taken for extraction.

## Requirements

To compile and run `VFE`, the following dependencies are required:

- FFmpeg libraries (libavcodec, libavformat, libswscale, libavutil)
- GCC (for compilation)
- Linux-based system (tested on Ubuntu)

## Installation

1. **Install FFmpeg Libraries**:  
   On Ubuntu, run the following commands:
   ```bash
   sudo apt update
   sudo apt install libavcodec-dev libavformat-dev libswscale-dev libavutil-dev
   ```

2. **Clone the Repository**:
   ```bash
   git clone https://github.com/sudoXpg/videoFrameExtractor
   cd vfe
   ```

3. **Compile the Program**:
   Use the following command to compile:
   ```bash
   gcc main.c -lavcodec -lavformat -lswscale -lavutil -o vfe
   ```

4. **Make it Globally Accessible**:
   To use `VFE` from anywhere on your system:
   ```bash
   sudo mv vfe /usr/local/bin/
   ```

## Usage

To use `VFE`, run the following command:

```bash
vfe <input_file> <max_frames|all> [options]
```

### Arguments:

- **`<input_file>`**: Path to the input video file.
- **`<num_frames|all>`**: Specify the number of frames to extract or use 'all' to extract all frames.

### Options:

- `--frames`: Extract frames between the specified start and end time.
- `--dir <dir>`: Specify a directory to save the extracted frames. Creates the directory if it doesn't exist.
- `--verbose`: Enable verbose output for detailed logging.
- `--h`: Show the help message.

### Example Usage:

```bash
vfe input.mp4 all                            # Extract all frames from a video file.
vfe input.mp4 10                             # Extract the first 10 frames from a video file.
vfe input.mp4 all --frames 10 20             # Extract frames from 10 to 20 seconds of the video.
vfe input.mp4 all --dir ./frames             # Specify a directory to save the extracted frames.
```

## Example Output

When running `VFE`, you can expect the following output:

```bash
Frame index: 1, average frame rate: 29.97, resolution[1920x1080]
Frame index: 2, average frame rate: 29.97, resolution[1920x1080]
...
Completed in 12.35sec
```

## Help Screen

To view the help screen, run:

```bash
vfe --h
```

This will output:

```
>>    VFE v1 ~ Video Frame Extractor
---------------------------------------------------------
Usage:
  vfe <input_file> <max_frames|all> [options]

Arguments:
  <input_file>        Path to the input video file.
  <num_frames|all>    Specify the number of frames to extract or use 'all' to extract all frames.

Options:
  --frames                     Extract frames between the specified start and end time.
  --dir <dir>                  Specify a directory to save the extracted frames.
  --verbose                    Enable verbose output for detailed information during the extraction process.
  --h                          Show this help message.

Example Usage:
  vfe input.mp4 all                           # Extract all frames from a video file.
  vfe input.mp4 10                            # Extract the first 10 frames from a video file.
  vfe input.mp4 all --frames 10 20            # Extract frames from 10 to 20 seconds of the video.
  vfe input.mp4 all --dir ./frames            # Specify a directory to save the extracted frames.

---------------------------------------------------------
This program utilizes FFmpeg libraries for efficient video processing.
```

## Known Issues

- Ensure that the input video file is accessible and in a supported format.
- The `--frames` option requires both start and end times to be valid.
- Verbose mode might slow down frame extraction due to extensive logging.

## Contributing

Contributions are welcome! Please open an issue or submit a pull request if you have suggestions or fixes.

## License

This project is licensed under the [?? License].
