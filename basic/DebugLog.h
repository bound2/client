//-----------------------------------------------------------------------------
// DebugLog.h
//
// Lightweight, cross-platform logging system for Dark Eden client
// Replaces the old DEBUG_ADD series of macros
//
// Features:
// - Automatic __FILE__ and __LINE__ inclusion
// - Log levels: DEBUG/INFO/WARN/ERROR/NONE
// - Configurable output: console/file/memory array
// - Thread-safe (using CRITICAL_SECTION)
// - Conditional compilation: DEBUG logs removed in Release builds
//-----------------------------------------------------------------------------

#ifndef __DEBUG_LOG_H__
#define __DEBUG_LOG_H__

#include <stdio.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

//-----------------------------------------------------------------------------
// Log Level Enumeration
//-----------------------------------------------------------------------------
typedef enum {
	LOG_LEVEL_DEBUG = 0,	// Detailed diagnostic information
	LOG_LEVEL_INFO,		// General informational messages
	LOG_LEVEL_WARN,		// Warning messages
	LOG_LEVEL_ERROR,	// Error messages
	LOG_LEVEL_NONE		// Disable all logging
} LogLevel;

//-----------------------------------------------------------------------------
// Public Interface
//-----------------------------------------------------------------------------

// Initialization and cleanup
void log_init(void);
void log_cleanup(void);

// Configuration
void log_set_level(LogLevel level);
void log_set_console_output(bool enable);
void log_set_file_output(const char *path);
void log_set_array_output(bool enable);

// Core logging function (called by macros)
void log_write(LogLevel level,
			   const char *file,
			   int line,
			   const char *fmt,
			   ...);

//-----------------------------------------------------------------------------
// Logging Macros
//-----------------------------------------------------------------------------


#define LOG_DEBUG(fmt, ...) \
	log_write(LOG_LEVEL_DEBUG, __FILE__, __LINE__, fmt, ##__VA_ARGS__)

#define LOG_INFO(fmt, ...) \
	log_write(LOG_LEVEL_INFO, __FILE__, __LINE__, fmt, ##__VA_ARGS__)

#define LOG_WARN(fmt, ...) \
	log_write(LOG_LEVEL_WARN, __FILE__, __LINE__, fmt, ##__VA_ARGS__)

#define LOG_ERROR(fmt, ...) \
	log_write(LOG_LEVEL_ERROR, __FILE__, __LINE__, fmt, ##__VA_ARGS__)

//-----------------------------------------------------------------------------
// Backward Compatibility - Map old DEBUG_ADD macros to new system
//-----------------------------------------------------------------------------

#define DEBUG_ADD(msg)			LOG_INFO("%s", msg)
#define DEBUG_ADD_ERR(msg)		LOG_ERROR("%s", msg)
#define DEBUG_ADD_WAR(msg)		LOG_WARN("%s", msg)
#define DEBUG_ADD_FORMAT(fmt, ...)	LOG_INFO(fmt, ##__VA_ARGS__)
#define DEBUG_ADD_FORMAT_ERR(fmt, ...)	LOG_ERROR(fmt, ##__VA_ARGS__)
#define DEBUG_ADD_FORMAT_WAR(fmt, ...)	LOG_WARN(fmt, ##__VA_ARGS__)

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus

#include <source_location>

//-----------------------------------------------------------------------------
// C++20 diagnostics seam
//-----------------------------------------------------------------------------
//
// log_write is a C variadic function, so a std::source_location parameter can
// neither follow its '...' nor be defaulted there. LogSite is that parameter
// moved in front of the format: default-construct one and it captures the
// caller's file, line and function, because a
// std::source_location::current() written as a default argument is evaluated
// at the call site rather than here. The standard states that as recommended
// practice rather than a guarantee; MSVC, Clang and GCC all do it, and
// tests/unit/test_source_location_diagnostics.cpp pins it against __LINE__.
//
//     log_write_at(LogSite(), LOG_LEVEL_INFO, "zone %d loaded", nZoneID);
//
// The LOG_* and DEBUG_ADD* macros above are untouched and still forward
// __FILE__ and __LINE__. Both entry points reach the same core and produce
// the same log line: the function name is captured but deliberately NOT
// printed, so the console and on-disk format is exactly what it was.
//-----------------------------------------------------------------------------
struct LogSite
{
	const char	*file;
	int		line;
	const char	*function;	// NULL when only a file and a line were supplied

	LogSite(const std::source_location &location = std::source_location::current())
		: file(location.file_name()),
		  line((int)location.line()),
		  function(location.function_name())
	{
	}

	LogSite(const char *site_file, int site_line)
		: file(site_file),
		  line(site_line),
		  function(NULL)
	{
	}
};

// C++20 entry point. The site comes first because it cannot follow the '...'.
void log_write_at(const LogSite &site,
				  LogLevel level,
				  const char *fmt,
				  ...);

//-----------------------------------------------------------------------------
// Test seam
//-----------------------------------------------------------------------------
//
// Reports the site of every log call, from either entry point, BEFORE the
// level filter - so a test can observe what was recorded without initialising
// the logging system, moving the level or redirecting the output. The
// observer is NULL in every shipped build and nothing else about logging
// changes; tests/unit/test_source_location_diagnostics.cpp is the only user.
//-----------------------------------------------------------------------------
typedef void (*LogSiteObserver)(const LogSite &site, LogLevel level);

// Installs an observer and returns the one it replaced.
LogSiteObserver log_set_site_observer(LogSiteObserver observer);

#endif // __cplusplus

#endif // __DEBUG_LOG_H__
