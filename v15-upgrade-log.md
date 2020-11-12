# Upgrade log for SpatialOS 15.0.0

This log provides the list of changes made to this project to upgrade it from
SpatialOS 14.9.0 to SpatialOS 15.0.0. You can follow a similar sequence of steps
to upgrade your project.

We will first migrate away from any deprecated functionality that we can on
14.9.0. This step very much depends on what functionality a project uses, and
whether or not you have already made similar changes when APIs got deprecated.

1. Use the new logging API instead of `LogMessageOp`s. For simplicity, we log
   all messages with log-level Warning and above, and Info messages related to
   establishing the connection.

Now we will upgrade to SpatialOS 15.0.0.

1. Update the version numbers used in `spatialos.json`.
1. Remove unsupported bridge settings from all `worker.json`s. The settings
   were used for legacy load balancing and interest systems. Both of these
   systems have been re-worked and replaced, and these options are no longer
   relevant.
1. Update which worker packages are used by the project. Debug packages are no
   longer available by default, and Linux packages have been renamed to reflect
   compiler upgrades. Static linking against a release library when building in
   debug mode on Windows is problematic, and so we switch our project to link
   against the Worker SDK dynamically to keep things simple. Refer to the full
   release notes to see how this changed the runtime requirements. For example,
   the required minimum version of `glibc` has increased on Linux, which might
   cause incompatibility with old Linux distributions.
1. Update the connection types. For simplicity, we are using Insecure connections
   for all workers. If we were to use TLS or DTLS connection security, we would
   have to turn this off when running locally, as the runtime Local Edition does
   not support establishing secure connections.
1. Address changes to entity queries. The only result type is now the snapshot
   result type.
1. Remove the use of EntityAcl from code, as it's been removed. We will deal with
   fixing the snapshot and getting authority to work later.
1. Address changes made to command request op metadata. We only have to change
   some logging messages for now.
