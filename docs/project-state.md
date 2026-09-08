# Project state

Comparison baseline: no independently authored UE3-on-Xbox-360 contract; title
ports previously had to choose between duplicated engine policy and reference
source coupling.

| ID | Capability | State | Evidence |
|---|---|---|---|
| S001 | Title-supplied ABI/binding-schema validation | verified | `x360ue3_contract_tests` validates a positive schema and overlapping-field refusal. |
| S002 | Semantic RHI operation contract | verified | `FrameContract` accepts ordered bind/draw/present operations and rejects lifecycle-owned operations. |
| S003 | Object/resource/frame lifetime contract | verified | Focused contract test covers registration, duplicate/unknown resources, present-before-end, and closed-frame state. |
| S004 | Gears production consumption | partial | Gears consumes the exact revision in its asset-free contract test; title runtime composition remains open. |

## Current focus

S004: use this contract from the Gears title adapter without moving title
addresses or UE3 reference code into the shared layer.
