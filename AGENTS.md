# x360ue3 working contract

`x360ue3` is the title-neutral UE3-on-Xbox-360 layer above `shared/x360port`.
It owns reusable ABI descriptions, title-supplied binding-schema validation,
UE3 semantic RHI operation vocabulary, and object/resource/frame lifetime
contracts. It never owns title addresses, image identities, shader hashes,
navigation, saves, gameplay policy, or application composition.

The local `shared/ue3` checkout is reference material only. Do not include it,
copy from it, or make it a build/runtime dependency. Keep this library free of
Xenia types and CPU execution policy; execution belongs to `x360port` and
title-specific bindings belong to the consuming title.

Build and verify C++ with Clang, clang-format, and clang-tidy. Consumers pin
this repository to an immutable revision and must use the same public
contracts rather than copying them.
