# Project goals

## G001 — Provide a title-neutral UE3-on-Xbox-360 contract

### Success conditions

- Validate title-supplied ABI and binding schemas without embedding title
  addresses, image identities, or gameplay policy.
- Represent semantic UE3 RHI operations independently of guest CPU execution.
- Enforce object, resource, and frame lifetime transitions with deterministic
  positive and negative tests.

### Constraints

- Depend only on public `x360port` vocabulary and contracts.
- Never compile, link, package, or copy the private/reference `shared/ue3`
  checkout.
- Guest execution, memory, imports, and executable invalidation remain owned by
  `x360port`; title adapters own exact identities and bindings.
