#----------------------------------------------------------------------
# check_wire_inventory_fresh.cmake
#----------------------------------------------------------------------
#
# Fails when tests/generated/WireInventory.inc has drifted from what
# tests/tools/gen_wire_inventory.pl produces from the packet headers, so
# that adding or changing a factory cannot leave the inventory stale.
# The inventory is what tests/unit/test_wire_layout.cpp checks the wire
# layout against, and what the server repo diffs its own copy with; a
# stale one silently stops covering the packets it no longer describes.
#
# Driven by the wire_inventory_fresh test registered in
# tests/CMakeLists.txt, which passes:
#
#     -DPERL_EXECUTABLE=<perl>  -DSOURCE_DIR=<repository root>
#
# Under `cmake -P` the current binary directory is the working directory
# ctest runs the test in, i.e. the build tree, so the freshly generated
# copy lands there and never in the source tree.
#----------------------------------------------------------------------

foreach(required PERL_EXECUTABLE SOURCE_DIR)
	if(NOT DEFINED ${required})
		message(FATAL_ERROR "${required} is not defined; this script is meant to be run by the wire_inventory_fresh test")
	endif()
endforeach()

set(committed "${SOURCE_DIR}/tests/generated/WireInventory.inc")
set(fresh     "${CMAKE_CURRENT_BINARY_DIR}/WireInventory.fresh.inc")

if(NOT EXISTS "${committed}")
	message(FATAL_ERROR "no committed inventory at ${committed}; run: perl tests/tools/gen_wire_inventory.pl")
endif()

#----------------------------------------------------------------------
# The generator resolves Client/Packet relative to the working directory
# and refuses to run anywhere else, so it is invoked from the root.
#----------------------------------------------------------------------
execute_process(
	COMMAND "${PERL_EXECUTABLE}" tests/tools/gen_wire_inventory.pl "${fresh}"
	WORKING_DIRECTORY "${SOURCE_DIR}"
	RESULT_VARIABLE  gen_result
	OUTPUT_VARIABLE  gen_output
	ERROR_VARIABLE   gen_error
)

if(NOT gen_result EQUAL 0)
	message(FATAL_ERROR
		"gen_wire_inventory.pl failed (${gen_result}):\n${gen_output}${gen_error}")
endif()

execute_process(
	COMMAND "${CMAKE_COMMAND}" -E compare_files "${committed}" "${fresh}"
	RESULT_VARIABLE compare_result
	OUTPUT_QUIET
	ERROR_QUIET
)

if(NOT compare_result EQUAL 0)
	message(FATAL_ERROR
		"tests/generated/WireInventory.inc is stale.\n"
		"\n"
		"  committed : ${committed}\n"
		"  fresh     : ${fresh}\n"
		"\n"
		"A packet factory's id or max size has changed without the inventory "
		"being regenerated. Re-record it from the repository root with\n"
		"\n"
		"    perl tests/tools/gen_wire_inventory.pl\n"
		"\n"
		"and commit the result, then re-check the server repo's copy against "
		"it: a difference there is a protocol bug in one repo or the other.\n"
		"\n"
		"If instead every line differs, the checkout has CRLF endings; "
		".gitattributes pins tests/generated/*.inc to LF because this "
		"comparison is byte for byte.")
endif()

message(STATUS "WireInventory.inc matches a fresh generation")
