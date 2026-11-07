# x360ue3

`x360ue3` is the independently authored, title-neutral UE3-on-Xbox-360
contract layer. It sits above [`x360port`](../x360port) and below a consuming
title adapter.

It currently owns three narrow contracts:

- title-supplied ABI/binding schemas with fail-closed object-layout checks;
- semantic UE3 RHI operation vocabulary and ordering;
- object, resource, and frame lifetime transitions.

It does not parse XEX files, execute guest code, own Xenos translation, or
contain Gears addresses or title policy. The local `shared/ue3` checkout is not
a source or dependency.

Build the focused contract test with Clang from a checkout that also contains
`shared/x360port`:

```sh
cmake -S . -B build -G Ninja -DCMAKE_CXX_COMPILER=clang++ \
  -DX360PORT_ROOT=../x360port
cmake --build build
ctest --test-dir build --output-on-failure
```

Built with AI assistance (Claude and other LLMs).
