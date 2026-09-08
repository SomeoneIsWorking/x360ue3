#include "x360ue3/engine_contract.hpp"

#include <algorithm>
#include <utility>

namespace x360ue3
{
namespace
{

[[nodiscard]] ContractResult Refuse(ContractError error, std::string detail)
{
    return {error, std::move(detail)};
}

[[nodiscard]] bool Contains(const std::vector<std::uint32_t>& values, std::uint32_t value)
{
    return std::ranges::find(values, value) != values.end();
}

[[nodiscard]] bool IsValidKind(BindingKind kind) noexcept
{
    return kind == BindingKind::Scalar || kind == BindingKind::Pointer ||
           kind == BindingKind::Function;
}

[[nodiscard]] bool IsValidOperation(RhiOperationKind kind) noexcept
{
    return kind >= RhiOperationKind::BeginFrame && kind <= RhiOperationKind::EndFrame;
}

} // namespace

ContractResult ValidateBindingSchema(const BindingSchema& schema)
{
    if (schema.abi.schema_version != 1U || schema.abi.guest_pointer_size != 4U ||
        schema.abi.object_alignment == 0U ||
        (schema.abi.object_alignment & (schema.abi.object_alignment - 1U)) != 0U ||
        schema.object_size == 0U)
    {
        return Refuse(ContractError::InvalidAbi,
                      "UE3 binding schema requires ABI v1, 32-bit guest pointers, a power-of-two "
                      "object alignment, and a non-empty object");
    }

    for (std::size_t index = 0; index < schema.fields.size(); ++index)
    {
        const BindingField& field = schema.fields[index];
        if (field.name.empty())
        {
            return Refuse(ContractError::EmptyBindingName,
                          "UE3 binding schema contains an empty field name");
        }
        if (!IsValidKind(field.kind))
        {
            return Refuse(ContractError::InvalidBindingKind,
                          "UE3 binding schema contains an unknown field kind");
        }
        if (field.width != 1U && field.width != 2U && field.width != 4U && field.width != 8U)
        {
            return Refuse(ContractError::InvalidBindingWidth,
                          "UE3 binding field width must be 1, 2, 4, or 8 bytes");
        }
        if (field.object_offset > schema.object_size ||
            field.width > schema.object_size - field.object_offset)
        {
            return Refuse(ContractError::BindingOutsideObject,
                          "UE3 binding field extends beyond its declared object");
        }
        if (field.guest_address == 0U || (field.guest_address & 3U) != 0U)
        {
            return Refuse(ContractError::MisalignedGuestAddress,
                          "UE3 binding guest addresses must be nonzero and 4-byte aligned");
        }
        for (std::size_t previous = 0; previous < index; ++previous)
        {
            const BindingField& prior = schema.fields[previous];
            if (prior.name == field.name)
            {
                return Refuse(ContractError::DuplicateBindingName,
                              "UE3 binding schema contains duplicate field names");
            }
            const std::uint64_t field_end =
                static_cast<std::uint64_t>(field.object_offset) + field.width;
            const std::uint64_t prior_end =
                static_cast<std::uint64_t>(prior.object_offset) + prior.width;
            if (field.object_offset < prior_end && prior.object_offset < field_end)
            {
                return Refuse(ContractError::BindingOutsideObject,
                              "UE3 binding fields overlap in the declared object");
            }
        }
    }
    return {};
}

std::string_view ToString(ContractError error) noexcept
{
    switch (error)
    {
    case ContractError::None:
        return "none";
    case ContractError::InvalidAbi:
        return "invalid-abi";
    case ContractError::EmptyBindingName:
        return "empty-binding-name";
    case ContractError::DuplicateBindingName:
        return "duplicate-binding-name";
    case ContractError::InvalidBindingKind:
        return "invalid-binding-kind";
    case ContractError::InvalidBindingWidth:
        return "invalid-binding-width";
    case ContractError::BindingOutsideObject:
        return "binding-outside-object";
    case ContractError::MisalignedGuestAddress:
        return "misaligned-guest-address";
    case ContractError::FrameAlreadyOpen:
        return "frame-already-open";
    case ContractError::FrameNotOpen:
        return "frame-not-open";
    case ContractError::InvalidFrameId:
        return "invalid-frame-id";
    case ContractError::InvalidObjectId:
        return "invalid-object-id";
    case ContractError::DuplicateObject:
        return "duplicate-object";
    case ContractError::UnknownObject:
        return "unknown-object";
    case ContractError::InvalidResourceId:
        return "invalid-resource-id";
    case ContractError::DuplicateResource:
        return "duplicate-resource";
    case ContractError::UnknownResource:
        return "unknown-resource";
    case ContractError::InvalidOperationKind:
        return "invalid-operation-kind";
    case ContractError::InvalidOperationOrder:
        return "invalid-operation-order";
    case ContractError::SequenceRegression:
        return "sequence-regression";
    }
    return "unknown";
}

ContractResult FrameContract::RequireFrame() const
{
    if (!frame_open_)
    {
        return Refuse(ContractError::FrameNotOpen, "UE3 frame operation requires an open frame");
    }
    return {};
}

ContractResult FrameContract::RequireResource(std::uint32_t resource_id) const
{
    if (resource_id == 0U)
    {
        return Refuse(ContractError::InvalidResourceId, "UE3 RHI operation requires a resource");
    }
    if (!Contains(resources_, resource_id))
    {
        return Refuse(ContractError::UnknownResource,
                      "UE3 RHI operation references an unregistered resource");
    }
    return {};
}

ContractResult FrameContract::BeginFrame(std::uint64_t frame_id)
{
    if (frame_open_)
    {
        return Refuse(ContractError::FrameAlreadyOpen, "UE3 frame cannot begin twice");
    }
    if (frame_id == 0U)
    {
        return Refuse(ContractError::InvalidFrameId, "UE3 frame IDs are nonzero");
    }
    frame_open_ = true;
    presented_ = false;
    frame_id_ = frame_id;
    last_sequence_ = 0;
    operation_count_ = 0;
    return {};
}

ContractResult FrameContract::RegisterObject(std::uint32_t object_id)
{
    if (!frame_open_)
    {
        return RequireFrame();
    }
    if (object_id == 0U)
    {
        return Refuse(ContractError::InvalidObjectId, "UE3 object IDs are nonzero");
    }
    if (Contains(objects_, object_id))
    {
        return Refuse(ContractError::DuplicateObject,
                      "UE3 object lifetime registered an object twice");
    }
    objects_.push_back(object_id);
    return {};
}

ContractResult FrameContract::RetireObject(std::uint32_t object_id)
{
    ContractResult frame = RequireFrame();
    if (!frame)
    {
        return frame;
    }
    const auto found = std::ranges::find(objects_, object_id);
    if (found == objects_.end())
    {
        return Refuse(ContractError::UnknownObject,
                      "UE3 object lifetime retired an unknown object");
    }
    objects_.erase(found);
    return {};
}

ContractResult FrameContract::RegisterResource(std::uint32_t resource_id)
{
    ContractResult frame = RequireFrame();
    if (!frame)
    {
        return frame;
    }
    if (resource_id == 0U)
    {
        return Refuse(ContractError::InvalidResourceId, "UE3 resource IDs are nonzero");
    }
    if (Contains(resources_, resource_id))
    {
        return Refuse(ContractError::DuplicateResource,
                      "UE3 resource lifetime registered a resource twice");
    }
    resources_.push_back(resource_id);
    return {};
}

ContractResult FrameContract::RetireResource(std::uint32_t resource_id)
{
    ContractResult frame = RequireFrame();
    if (!frame)
    {
        return frame;
    }
    const auto found = std::ranges::find(resources_, resource_id);
    if (found == resources_.end())
    {
        return Refuse(ContractError::UnknownResource,
                      "UE3 resource lifetime retired an unknown resource");
    }
    resources_.erase(found);
    return {};
}

ContractResult FrameContract::Append(const RhiOperation& operation)
{
    ContractResult frame = RequireFrame();
    if (!frame)
    {
        return frame;
    }
    if (!IsValidOperation(operation.kind) || operation.kind == RhiOperationKind::BeginFrame ||
        operation.kind == RhiOperationKind::EndFrame)
    {
        return Refuse(ContractError::InvalidOperationKind,
                      "UE3 frame lifecycle owns begin/end operations");
    }
    if (operation.sequence == 0U || operation.sequence <= last_sequence_)
    {
        return Refuse(ContractError::SequenceRegression,
                      "UE3 semantic RHI operation sequence must increase strictly");
    }
    if (operation.kind == RhiOperationKind::BindRenderTarget ||
        operation.kind == RhiOperationKind::DrawIndexed ||
        operation.kind == RhiOperationKind::Resolve)
    {
        const ContractResult resource = RequireResource(operation.resource_id);
        if (!resource)
        {
            return resource;
        }
    }
    if (operation.kind == RhiOperationKind::Present)
    {
        if (presented_)
        {
            return Refuse(ContractError::InvalidOperationOrder, "UE3 frame cannot present twice");
        }
        presented_ = true;
    }
    last_sequence_ = operation.sequence;
    ++operation_count_;
    return {};
}

ContractResult FrameContract::EndFrame()
{
    ContractResult frame = RequireFrame();
    if (!frame)
    {
        return frame;
    }
    if (!presented_)
    {
        return Refuse(ContractError::InvalidOperationOrder,
                      "UE3 frame must present before it ends");
    }
    frame_open_ = false;
    presented_ = false;
    return {};
}

} // namespace x360ue3
