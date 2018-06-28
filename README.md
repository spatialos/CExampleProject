# SpatialOS C example project

## Quick start

Build the project and start it with the default launch configuration:

```
spatial worker build --target windows
spatial local launch
```

(Replacing `windows` with `macos` on macOS, or `linux` on Linux).

This will launch SpatialOS locally with a single C# "physics" worker that updates the position of
a single entity. You can then connect either one of the two C client workers (one implemented using
"direct" serialization, the other implemented using "vtable" serialization). These workers can be
launched with the following commands:

* Client (direct): `spatial local worker launch client_direct local`
* Client (vtable): `spatial local worker launch client_vtable local`

## Scenario

This project is used to showcase the C API and how it can be used to implement a simple client
worker which visualizes the state of a single entity whose position is updated by a "physics"
worker. As serialization in the C API can be implemented in two different ways, we provide two
implementations of the same worker in `workers/c_client_direct` and `workers/c_client_vtable`.
Either one of these can be used as a basis for further experimentation, and the client worker that's
not being used can easily be deleted without breaking any other functionality.

When a client worker connects, it sends a command to the C# worker (on the `sample.Login` component).
The C# worker then modifies the entity's write ACLs to delegate component 1001 (`sample.ClientData`)
to the client, using the `CallerWorkerAttributes` field of the `CommandRequestOp`. This causes the
entity to be checked out by the client worker, and the client worker will begin to receive component
updates for position changes. The physics worker will also begin to send a simple command to the
client every few seconds.

## Snapshot

The snapshot exists in both text and binary format in the `snapshots` folder. There is no script
to generate the snapshot as the snapshot was written by hand in the text format, but it's possible
to make simple changes to the text format and regenerate the binary snapshot from it. To update the
binary snapshot after making a change, run the following command:

```
spatial project history snapshot convert --input-format=text --input=snapshots/default.txt --output-format=binary --output=snapshots/default.snapshot
```