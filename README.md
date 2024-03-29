# SpatialOS C example project

## Dependencies

This project contains workers written in both C and C++ and use [CMake](https://cmake.org/download/)
as their build system. Your system needs to satisfy the
[C++ prerequisites](https://docs.improbable.io/reference/latest/cppsdk/setting-up#prerequisites).
In practice, this just means having a fairly recent compiler that supports C++11 or later.

## Quick start

Build the project and start it with the default launch configuration:

```
spatial worker build --target windows
spatial local launch --runtime_version=16.1.0
```

(Replacing `windows` with `macos` on macOS, or `linux` on Linux).

This will launch SpatialOS locally with a single C++ "physics" worker that updates the position of
a single entity. You may also see a 2nd entity called "physics-worker" created. This entity
represents the managed worker itself.

Note: If you run `spatial worker build` without a `--target` parameter (or with the wrong target
parameter), then the CMake cache for each worker (`workers/<worker>/cmake_build`) may end up in
a corrupt state. To recover, just run `spatial worker clean` to delete the CMake caches.

Now, you can connect either  the C client workers. This worker can be
launched with the following command:

```
spatial local worker launch client_direct local
```

## Scenario

This project is used to showcase the C API and how it can be used to implement a simple client
worker which visualizes the state of a single entity whose position is updated by a "physics"
worker. 

When a client worker connects, it sends a command to the C++ worker (on the `sample.Login`
component). The C++ worker then modifies the entity's write ACLs to delegate component 1001
(`sample.ClientData`) to the client, using the `CallerWorkerAttributes` field of the
`CommandRequestOp`. This causes the entity to be checked out by the client worker, and the client
worker will begin to receive component updates for position changes. The physics worker will also
begin to send a simple command to the client every few seconds. Due to the entity's QBI query,
the client worker will check in/out another entity due to its movement.

## Snapshot

The snapshot exists in both JSON and binary format in the `snapshots` folder. There is no script
to generate the snapshot as the snapshot was written by hand in JSON format, but it's possible
to make simple changes to the JSON snapshot and regenerate the binary snapshot from it. To update the
binary snapshot after making a change, run the following command:

```
spatial project history snapshot convert --input-format=text --input=snapshots/default.json --output-format=binary --output=snapshots/default.snapshot
```
