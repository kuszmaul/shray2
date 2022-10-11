# defines should always use UPPER_CASE naming
string(TOUPPER "${PROJECT_NAME}" PROJECT_NAME_UPPER)

# add c header which contains project info
configure_file(
	"${PROJECT_SOURCE_DIR}/cmake/version.h.in"
	"${PROJECT_BINARY_DIR}/include/${PROJECT_NAME}/version.h")

set(PROJECT_INCLUDE_DIRS ${PROJECT_INCLUDE_DIRS}
	"${PROJECT_BINARY_DIR}/include")
