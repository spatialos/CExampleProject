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
1. Add component sets. The physics worker always had authority over the EntityAcl,
   the Position, and the Login component, while the client had authority over the
   ClientData component. We will therefore need two component sets to represent
   the two different units of authority in our project. We could replace the
   EntityAcl with the AuthorityDelegation component, but with the new user-space
   load balancing, we won't need to update authority dynamically.
1. Update the component registry for the C++ physics worker. We use the
   `worker::Schema` template to combine components and component sets. We need
   both, as component sets are now the unit of authority.
1. Create and implement an authority model. Our requirements in this example are
   very simple. We want two different workers to be authoritative over two
   disctinct component sets on one entity. We will therefore need two partitions.
   To keep things clean, we will use two separate entities to represent the
   partitions. We will fix the snapshot later. For now, let's assume that these
   partitions have IDs 2 for the physics simulation, and 3 for the client
   simulation. We only have one entity we want to simulate, and the authority
   delegation can be fully static. All we need to do is:
   * When the physics worker starts, it needs to assign partition with ID 2 to
     itself.
   * When a client worker requests authority, the physics worker needs to assign
     it the partition with ID 3.
   * We need to ensure the physics worker has the right permissions to send
     system commands. We will address this later, when fixing the launch
     configurations.

   Please note that this is the very simplest example of how to distribute work
   and authority. The number of partitions in the world can be dynamic. The
   mapping of workers to partitions can be dynamic, and a worker can simulate
   multiple partitions at once. The delegation of component sets to partitions
   can be dynamic. Which component sets are delegated can be dynamic too, as long
   as they don't overlap at any time, on any given entity.
1. Fix the initial snapshot. We need to remove the EntityAcl and replace it with
   AuthorityDelegation. We will also need the entities which serve as our
   Partitions. Additionally, interest is now defined in terms of component sets
   rather than components to match the authority model, and so we will need to
   address that as well. We modify the json snapshot directly to account for the
   changes mentioned above, and update the binary snapshot by converting the
   json one using the following command:
   `spatial project history snapshot convert --input snapshots/default.json --input-format=text --output snapshots/default.snapshot --output-format=binary`.
   Note that we had to manually add some new fields to the json snapshot, as the
   json format requires all fields to be specified (even if they are empty).
