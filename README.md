# core-service

[![Test](https://github.com/shalex88/core-service/actions/workflows/test.yml/badge.svg)](https://github.com/shalex88/core-service/actions/workflows/test.yml)
[![Coverage](https://img.shields.io/codecov/c/github/shalex88/core-service)](https://codecov.io/github/shalex88/core-service)
[![Release](https://img.shields.io/github/v/release/shalex88/core-service.svg)](https://github.com/shalex88/core-service/releases/latest)

## Build

```bash
./scripts/build.sh
```

## Install

```bash
./scripts/install.sh
sudo dpkg -i ./build-native/core-service-tests_0.0.0_amd64.deb
```

## Usage

```bash
A camera control service
core-service [OPTIONS]
OPTIONS:
-h,     --help              Print this help message and exit
-v,     --version           Show version information
-c,     --config TEXT:FILE  Configuration file path
```

## Run

```bash
./core-service -c ../config/config-wfov.yaml

# Run client
grpcui -plaintext 0.0.0.0:50051
```

## Test

### Unit tests

```bash
./core-service-unit-tests
```

### Integration tests

```bash
./core-service-integration-tests
```

### System tests

```bash
./core-service-system-tests
```

## Add new functionality

### Camera

1. Add new functionality in `src/common/types/CameraCapabilities.h`
2. Extend ICameraHal with the new capability
3. Implement the capability in CameraHal class
4. Implement the new capability in the concrete camera class

### Core

1. Add new function in ICore interface
2. Implement new function in Core class

### API

1. Add new function in IRequestHandler interface
2. Implement new function in RequestHandler class
3. Define new RPC in `proto/camera_service.proto`
4. Create new RPC in GrpcCallbackHandler class

## TODO

- Add monitoring for lower layers to be able to stop execution requested by lower layers
- Set Nagle's algorithm on TCP sockets?
- Core still does almost nothing, it's config file is also unnecessary
- Add noexcept contract for public methods
- Add fixes-sized thread pool instead async
-