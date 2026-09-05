#include "BasicException.h"
#include <stdio.h>
#include <stdlib.h>

// Test seam (see BasicException.h). NULL in every shipped build. Named
// without the g_p prefix on purpose: ratchet R4 counts a library .cpp
// mentioning a g_p* symbol as a reach into the executable's globals, and this
// is a file-static of basic's own.
static BasicExceptionReporter g_ExceptionReporter = NULL;

void g_SetNewHandler()
{
	// Set new handler for memory allocation failures
	// For SDL backend, this is a stub
}

BasicExceptionReporter g_SetBasicExceptionReporter(BasicExceptionReporter reporter)
{
	BasicExceptionReporter previous = g_ExceptionReporter;
	g_ExceptionReporter = reporter;
	return previous;
}

void g_BasicException(EXCEPTION_CODE code, const char* sz_error, const char* file, unsigned long line)
{
	// Compatibility wrapper: records exactly the file and the line it was
	// given, and reports no function name rather than inventing one.
	g_BasicException(code, sz_error, ExceptionSite(file, line));
}

void g_BasicException(EXCEPTION_CODE code, const char* sz_error, const ExceptionSite& site)
{
	// Test seam: a reporter takes the diagnostic and the site, and the fatal
	// action below is skipped so the caller can carry on. Nothing outside the
	// unit tests installs one.
	if (g_ExceptionReporter != NULL)
	{
		g_ExceptionReporter(code, sz_error, site);
		return;
	}

	const char* const	file = site.file;
	const unsigned long	line = site.line;

	// Basic exception handler
	if (sz_error == NULL)
	{
		switch (code)
		{
			case MEM_ALLOC:
				fprintf(stderr, "Memory allocation failed at %s:%lu\n", file, line);
				break;
			case FILE_OPEN:
				fprintf(stderr, "File open failed at %s:%lu\n", file, line);
				break;
			case FILE_LOAD:
				fprintf(stderr, "File load failed at %s:%lu\n", file, line);
				break;
			case FAILED_JOB:
				fprintf(stderr, "Job failed at %s:%lu\n", file, line);
				break;
			case NULL_REF:
				fprintf(stderr, "Null reference at %s:%lu\n", file, line);
				break;
			default:
				fprintf(stderr, "Unknown error at %s:%lu\n", file, line);
				break;
		}
	}
	else
	{
		fprintf(stderr, "Error: %s at %s:%lu\n", sz_error, file, line);
	}

	// In debug mode, assert
	#ifdef _DEBUG
	assert(false);
	#else
	// In release mode, exit
	exit(1);
	#endif
}
