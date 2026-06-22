# Changelog

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [1.0.0] - 2026-06-22

Documentation and packaging refresh of the original 2019 library. No behavior changes.

### Added
- `keywords.txt` for Arduino IDE syntax highlighting.
- `examples/PHReading` sketch.
- This changelog.
- `.gitattributes` enforcing LF line endings.

### Changed
- Rewrote README (features, installation, sync/async usage, API overview).
- Updated `library.properties` (author/maintainer/version/url/category).
- Cleaned up source comments; removed dead commented-out code. Logic unchanged.

### Fixed
- Removed the orphan `AtlasStampPh::slope()` declaration (declared in the header but never defined — a latent linker error).

### Removed
- Relicensed from GPLv3 to MIT (removed `LICENSE.txt`, added `LICENSE`).

## [0.1.0] - 2019-02-13

First working version (legacy).

### Added
- Base `AtlasStamp` class with the EZO I2C command protocol, synchronous and
  asynchronous reads, LED control, sleep/wakeup and supply voltage readout.
- `AtlasStampTemperatureCompensated` with temperature compensation.
- `AtlasStampPh`, `AtlasStampDo`, `AtlasStampOrp`, `AtlasStampEc` module classes.
