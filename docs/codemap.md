# Codemap

| Responsibility | Owner | Boundary |
|---|---|---|
| ABI and title-supplied binding schema | `include/x360ue3/engine_contract.hpp`, `src/engine_contract.cpp` | Validates layout and guest-address shape; does not know a title identity. |
| Semantic UE3 RHI operation order | `FrameContract` in the same module | Tracks ordered bind/draw/resolve/present operations; does not execute a backend. |
| Object/resource/frame lifetime | `FrameContract` in the same module | Owns deterministic lifecycle state; title adapters supply IDs. |
| Guest CPU, memory, imports, invalidation | `shared/x360port` | Consumed boundary; never reimplemented here. |
| Exact title identity and bindings | consuming title | Gears or another title owns these facts. |

The module is intentionally small. New title-neutral execution behavior belongs
in `x360port` or a separately justified shared owner, not in this contract file.
