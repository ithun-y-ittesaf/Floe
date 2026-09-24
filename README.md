# Floe

**A programming language for media.**

Floe is a high-level language for creating, transforming, composing, and automating video, audio, and other media.

Instead of manipulating media exclusively through a graphical editor or low-level APIs, Floe lets you describe media workflows as code.

```floe
video
    .trim(10s, 30s)
    .scale(1920, 1080)
    .overlay(logo)
    .encode("h264")
    .output("result.mp4")
```

Floe is powered by FFmpeg, but it is not intended to be a thin wrapper around FFmpeg. It provides its own language, media abstractions, type system, and intermediate representation while using FFmpeg as its underlying media engine.

The long-term goal is to make media creation as programmable as software itself.
