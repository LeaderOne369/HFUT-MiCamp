# Xiaomi Engineering Training Camp - C++ Intensive Course

This repository documents my comprehensive participation in the Xiaomi Engineering Training Camp, a rigorous 15-day intensive program jointly organized by Hefei University of Technology and Xiaomi Corporation in early 2025. This bootcamp was led by senior engineers from Xiaomi and focused on advanced C++ programming, multithreaded concurrency control, network programming, and audio-visual processing.

## Program Overview

![C++ Course Content](resource/context.png)

The training camp covered a comprehensive curriculum spanning 11 days, with each day focusing on progressively advanced topics:

- **Day 1-2**: C++ fundamentals, object-oriented programming, memory management
- **Day 3-4**: Advanced pointers, dynamic allocation, STL containers and algorithms
- **Day 5**: Multithreaded programming, thread synchronization, concurrent data structures
- **Day 6**: Network programming, TCP/UDP sockets, protocol implementation
- **Day 7**: Android development foundations
- **Day 8**: Network protocols, HTTP processing, cross-platform compilation
- **Day 9**: Audio-visual fundamentals, FFmpeg integration
- **Day 10-11**: Graphics programming, OpenGL, native Android integration

## Capstone Project: High-Performance Audio-Visual Transcoding System

![Capstone Project Requirements](resource/final.png)

My capstone project implemented a sophisticated **audio-visual transcoding system** using FFmpeg SDK, capable of processing multimedia files through a complete pipeline: demuxing, decoding, filtering, encoding, and remuxing. The system was developed using **C++17** and demonstrated production-grade engineering practices, featuring support for MP4-to-YUV/PCM decoding, 90° video rotation, and variable-speed playback (0.5x-3.0x) with pitch preservation.

### Project Architecture

The system is architected into five independent modules with thread-safe queue-based communication:

1. **Demuxer Module**: Extracts video and audio streams from container formats (MP4, TS, FLV)
2. **Decoder Modules**: Parallel video (H.264) and audio (AAC) decoding pipelines with synchronized output
3. **Filter Pipeline**: Advanced video/audio processing capabilities including geometric transforms and tempo adjustment
4. **Encoder Modules**: Efficient codec integration (MPEG-4 video, AC3 audio) for output generation
5. **Muxer Module**: Container format assembly with precise A/V synchronization

### Technical Achievements

#### Day 1: Decoder Implementation and Cross-Platform Deployment

**Task**: Decode MP4 files to YUV video and PCM audio with FFmpeg compiled as a single shared library.

**Key Challenges Solved**:

- **Monolithic Library Compilation**: Compiled FFmpeg into a unified dynamic library (`libffmpeg_merge.so`) by merging multiple static libraries using `--whole-archive` linker flags, enabling simplified deployment
- **Cross-Platform Compatibility**: Implemented dual-platform support with platform-specific configurations (macOS via Homebrew; Ubuntu with custom monolithic library), demonstrating strong system adaptation capabilities
- **Thread-Safe Queue Architecture**: Designed producer-consumer queues using `std::mutex` and `std::condition_variable` for inter-thread communication between demuxer and decoder modules
- **Audio Processing Integrity**: Resolved audio corruption artifacts (pops and clicks) through proper buffer alignment and sample format conversion using `swr_convert` with correct byte-per-sample calculations
- **Thread Synchronization**: Fixed race conditions in Linux environments that caused incomplete video transcoding (from 200MB partial output to full 700MB output) by implementing proper thread waiting mechanisms and buffer flushing logic

#### Day 2: Video Rotation Pipeline

**Task**: Implement 90° video rotation using FFmpeg video filters.

**Implementation**:

- Created video filter graph with `transpose` filter for 90° rotation
- Proper dimension swapping (width↔height) to maintain aspect ratio
- Custom log filtering system to suppress "too many B-frames" warnings
- Comprehensive memory leak prevention with RAII principles

#### Day 3: Advanced Audio Processing with Synchronization

**Task**: Implement variable-speed playback (0.5x-3.0x) with pitch preservation and perfect A/V sync.

**Technical Highlights**:

1. **Ring Buffer Optimization**

   - Custom thread-safe ring buffer implementation with 1MB capacity
   - Chunked audio processing (4KB blocks) to smooth CPU usage and prevent memory spikes
   - Eliminated audio burst distortion by ensuring frame-aligned reads

2. **Perfect Audio-Visual Synchronization**

   - Precise PTS (Presentation Time Stamp) calculation with per-sample tracking
   - Dynamic PTS increment computation: `ptsPerSample = ptsDifference / samplesPerFrame`
   - Eliminated ~500ms synchronization drift through incremental sample positioning
   - Synchronized output of AC3 audio and MPEG4 video into MP4 container

3. **Audio Speed Control**
   - Chained `atempo` filters for >2.0x speed: `atempo=2.0,atempo=1.5` for 3.0x playback
   - Pitch preservation through time-domain stretching (no frequency-domain distortion)
   - Zero-copy frame passing where possible for optimal performance

### Multi-Threading Architecture

The system employs a sophisticated multi-threaded design:

```
┌─────────────┐     ┌──────────────────┐     ┌─────────────────┐
│  Demuxer    │────▶│  Packet Queues   │────▶│  Video Decoder  │
│   Thread    │     │  (Thread-Safe)   │     │     Thread      │
└─────────────┘     └──────────────────┘     └─────────────────┘
                            │                          │
                            ▼                          ▼
                    ┌──────────────────┐     ┌─────────────────┐
                    │  Audio Decoder   │     │  File Writers   │
                    │     Thread       │     │  (Synchronized) │
                    └──────────────────┘     └─────────────────┘
```

**Synchronization Mechanisms**:

- `std::mutex` for exclusive file access
- `std::condition_variable` for queue-based producer-consumer patterns
- Atomic flags for thread lifecycle management
- Graceful shutdown with buffer flushing

### Performance Optimizations

1. **Memory Efficiency**: Ring buffer design reduced peak CPU usage from 95% to ~60% during heavy audio processing
2. **Zero-Copy Operations**: Minimized memory allocations through smart pointer management and frame cloning
3. **Parallel Pipeline**: Independent video/audio processing enables CPU-core utilization
4. **RAII Pattern**: Automatic resource cleanup prevents memory leaks

### Build System and Portability

**CMake-based build system** supporting:

- macOS with Homebrew FFmpeg (`brew install ffmpeg`)
- Ubuntu with custom monolithic library or system packages
- Conditional compilation for platform-specific libraries
- Parallel compilation (`-j4`) for faster builds

**Dependencies**:

- FFmpeg 4.x/5.x (libavcodec, libavformat, libavfilter, libswscale, libswresample)
- C++17 standard library
- POSIX threads (pthread)

### Testing and Validation

Comprehensive validation across:

- Multiple input formats (MP4, TS, FLV)
- Variable playback speeds (0.5x, 1.0x, 2.0x, 3.0x)
- Different resolution/bitrate combinations
- Long-form content (>1 hour videos)
- Frame-accurate output verification using `ffprobe`

### Project Deliverables

All source code, documentation, and the compiled FFmpeg library are available in the `VideoTransCode/` directory:

- **Source Code**: 14 C++ modules implementing the complete pipeline
- **Header Files**: 13 header files with clean APIs and documentation
- **Library**: Pre-compiled `libffmpeg_merge.so` for Ubuntu deployment
- **Documentation**: Comprehensive README with architecture diagrams and troubleshooting
- **Testing**: Automated build scripts and example media files

**Run the Project**:

```bash
cd VideoTransCode
./run.sh 2.0  # Process video at 2x speed
```

### Academic Recognition

**Outstanding Student Award**: Among 240 graduates, I was selected as an "Outstanding Student" based on:

- Complete functional implementation of all requirements
- Production-quality code organization and documentation
- Independent resolution of complex technical challenges
- Exceptional performance optimization

**Career Outcome**: Successfully secured a **full-time software engineering offer** from Xiaomi Corporation for the Fall 2026 recruiting season.

### Key Learnings

This intensive program significantly advanced my expertise in:

1. **Systems Programming**: Low-level audio/video codec integration, memory management, cross-platform compilation
2. **Concurrent Programming**: Multi-threaded architecture design, synchronization primitives, lock-free patterns
3. **Software Engineering**: Modular architecture, RAII principles, comprehensive testing
4. **Performance Engineering**: Profiling, optimization, resource efficiency
5. **Professional Development**: Working under time constraints, debugging complex systems, technical communication

### Repository Structure

```
MiCamp/
├── Day01-04/          # C++ fundamentals and advanced topics
├── Day05/             # Multithreaded programming exercises
├── Day06/             # Network programming with TCP/UDP
├── Day07-08/          # Android and network protocol development
├── Day09/             # FFmpeg and audio-visual processing
├── Day10-11/          # Graphics programming with OpenGL
├── VideoTransCode/    # Capstone project (complete implementation)
├── README.md          # This document
└── resource/          # Project requirements and diagrams
```

Each day's directory contains detailed README files, source code, build configurations, and execution results.

---

**Note**: This repository serves as a comprehensive portfolio of my engineering capabilities and demonstrates proficiency in systems programming, multimedia processing, and software architecture—foundational skills for graduate-level research in computer systems, multimedia, and performance engineering.
