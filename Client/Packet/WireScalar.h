//////////////////////////////////////////////////////////////////////////////
// Filename    : WireScalar.h
// Description : C++20 constraints for fixed-width packet scalar values
//////////////////////////////////////////////////////////////////////////////

#ifndef __WIRE_SCALAR_H__
#define __WIRE_SCALAR_H__

#include <bit>
#include <concepts>
#include <cstdint>
#include <type_traits>

namespace packetwire {

template <typename T, bool IsEnum = std::is_enum_v<std::remove_cv_t<T>>>
struct WireStorage
{
	using type = std::remove_cv_t<T>;
};

template <typename T>
struct WireStorage<T, true>
{
	using type = std::underlying_type_t<std::remove_cv_t<T>>;
};

template <typename T>
using WireStorageT = typename WireStorage<T>::type;

template <typename T>
concept FixedWidthInteger =
	std::same_as<std::remove_cv_t<T>, std::int8_t> ||
	std::same_as<std::remove_cv_t<T>, std::uint8_t> ||
	std::same_as<std::remove_cv_t<T>, std::int16_t> ||
	std::same_as<std::remove_cv_t<T>, std::uint16_t> ||
	std::same_as<std::remove_cv_t<T>, std::int32_t> ||
	std::same_as<std::remove_cv_t<T>, std::uint32_t> ||
	std::same_as<std::remove_cv_t<T>, std::int64_t> ||
	std::same_as<std::remove_cv_t<T>, std::uint64_t>;

template <typename T>
concept WireScalar =
	(std::is_integral_v<std::remove_cv_t<T>> ||
	 std::is_enum_v<std::remove_cv_t<T>>) &&
	FixedWidthInteger<WireStorageT<T>>;

static_assert(std::endian::native == std::endian::little,
	"The packet protocol requires a little-endian target");

} // namespace packetwire

#endif // __WIRE_SCALAR_H__
