#ifndef X360UE3_ENGINE_CONTRACT_HPP
#define X360UE3_ENGINE_CONTRACT_HPP

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

#include <x360port/module_contract.hpp>

namespace x360ue3
{

enum class BindingKind : std::uint8_t
{
    Scalar,
    Pointer,
    Function,
};

struct AbiDescription
{
    std::uint32_t schema_version = 1;
    std::uint32_t guest_pointer_size = 4;
    std::uint32_t object_alignment = 4;
};

struct BindingField
{
    std::string name;
    x360port::GuestAddress guest_address = 0;
    std::uint32_t object_offset = 0;
    std::uint32_t width = 0;
    BindingKind kind = BindingKind::Scalar;
};

struct BindingSchema
{
    AbiDescription abi;
    std::uint32_t object_size = 0;
    std::vector<BindingField> fields;
};

enum class ContractError : std::uint8_t
{
    None,
    InvalidAbi,
    EmptyBindingName,
    DuplicateBindingName,
    InvalidBindingKind,
    InvalidBindingWidth,
    BindingOutsideObject,
    MisalignedGuestAddress,
    FrameAlreadyOpen,
    FrameNotOpen,
    InvalidFrameId,
    InvalidObjectId,
    DuplicateObject,
    UnknownObject,
    InvalidResourceId,
    DuplicateResource,
    UnknownResource,
    InvalidOperationKind,
    InvalidOperationOrder,
    SequenceRegression,
};

struct ContractResult
{
    ContractError error = ContractError::None;
    std::string detail;

    [[nodiscard]] explicit operator bool() const noexcept { return error == ContractError::None; }
};

[[nodiscard]] ContractResult ValidateBindingSchema(const BindingSchema& schema);
[[nodiscard]] std::string_view ToString(ContractError error) noexcept;

enum class RhiOperationKind : std::uint8_t
{
    BeginFrame,
    BindRenderTarget,
    DrawIndexed,
    Resolve,
    Present,
    EndFrame,
};

struct RhiOperation
{
    RhiOperationKind kind = RhiOperationKind::BeginFrame;
    std::uint64_t sequence = 0;
    std::uint32_t resource_id = 0;
};

class FrameContract final
{
  public:
    FrameContract() = default;
    FrameContract(const FrameContract&) = delete;
    FrameContract& operator=(const FrameContract&) = delete;
    FrameContract(FrameContract&&) = delete;
    FrameContract& operator=(FrameContract&&) = delete;

    [[nodiscard]] ContractResult BeginFrame(std::uint64_t frame_id);
    [[nodiscard]] ContractResult RegisterObject(std::uint32_t object_id);
    [[nodiscard]] ContractResult RetireObject(std::uint32_t object_id);
    [[nodiscard]] ContractResult RegisterResource(std::uint32_t resource_id);
    [[nodiscard]] ContractResult RetireResource(std::uint32_t resource_id);
    [[nodiscard]] ContractResult Append(const RhiOperation& operation);
    [[nodiscard]] ContractResult EndFrame();

    [[nodiscard]] bool frame_open() const noexcept { return frame_open_; }
    [[nodiscard]] std::uint64_t frame_id() const noexcept { return frame_id_; }
    [[nodiscard]] std::size_t operation_count() const noexcept { return operation_count_; }

  private:
    [[nodiscard]] ContractResult RequireFrame() const;
    [[nodiscard]] ContractResult RequireResource(std::uint32_t resource_id) const;

    bool frame_open_ = false;
    bool presented_ = false;
    std::uint64_t frame_id_ = 0;
    std::uint64_t last_sequence_ = 0;
    std::size_t operation_count_ = 0;
    std::vector<std::uint32_t> objects_;
    std::vector<std::uint32_t> resources_;
};

} // namespace x360ue3

#endif
