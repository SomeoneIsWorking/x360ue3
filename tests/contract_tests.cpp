#include "x360ue3/engine_contract.hpp"

#include <cstdio>
#include <cstdlib>

namespace
{

void Require(bool condition, const char* message)
{
    if (!condition)
    {
        std::fprintf(stderr, "FAIL %s\n", message);
        std::abort();
    }
}

x360ue3::BindingSchema Schema()
{
    return {.abi = {},
            .object_size = 32U,
            .fields = {{"owner", 0x82200000U, 0U, 4U, x360ue3::BindingKind::Pointer},
                       {"flags", 0x82200004U, 4U, 4U, x360ue3::BindingKind::Scalar}}};
}

} // namespace

int main()
{
    const auto valid = x360ue3::ValidateBindingSchema(Schema());
    Require(static_cast<bool>(valid), "valid title-supplied binding schema refused");

    auto overlap = Schema();
    overlap.fields[1].object_offset = 2U;
    Require(!x360ue3::ValidateBindingSchema(overlap) &&
                x360ue3::ValidateBindingSchema(overlap).error ==
                    x360ue3::ContractError::BindingOutsideObject,
            "overlapping binding fields were accepted");

    x360ue3::FrameContract frame;
    Require(static_cast<bool>(frame.BeginFrame(1U)), "frame did not begin");
    Require(static_cast<bool>(frame.RegisterObject(7U)), "object did not register");
    Require(static_cast<bool>(frame.RegisterResource(11U)), "resource did not register");
    Require(static_cast<bool>(frame.Append({x360ue3::RhiOperationKind::BindRenderTarget, 1U, 11U})),
            "render-target operation was refused");
    Require(static_cast<bool>(frame.Append({x360ue3::RhiOperationKind::DrawIndexed, 2U, 11U})),
            "draw operation was refused");
    Require(static_cast<bool>(frame.Append({x360ue3::RhiOperationKind::Present, 3U, 0U})),
            "present operation was refused");
    Require(static_cast<bool>(frame.EndFrame()), "frame did not end after present");
    Require(!frame.frame_open() && frame.operation_count() == 3U,
            "frame lifetime did not close cleanly");

    x360ue3::FrameContract negative;
    Require(static_cast<bool>(negative.BeginFrame(2U)), "negative frame did not begin");
    Require(!negative.Append({x360ue3::RhiOperationKind::DrawIndexed, 1U, 99U}) &&
                negative.Append({x360ue3::RhiOperationKind::DrawIndexed, 1U, 99U}).error ==
                    x360ue3::ContractError::UnknownResource,
            "unknown resource did not refuse");
    Require(!negative.EndFrame() &&
                negative.EndFrame().error == x360ue3::ContractError::InvalidOperationOrder,
            "frame without present did not refuse");
    std::puts("x360ue3 contract: binding schema and frame lifetime checks passed");
}
