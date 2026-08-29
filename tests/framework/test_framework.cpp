//----------------------------------------------------------------------
// test_framework.cpp
//----------------------------------------------------------------------

#include "test_framework.h"

#include <cstdio>
#include <vector>

namespace testfw {

namespace {

struct TestCase
{
	const char*	suite;
	const char*	name;
	TestFn		fn;
};

//----------------------------------------------------------------------
// Held in a function-local static so registration during static
// initialisation cannot race the container's own construction.
//----------------------------------------------------------------------
std::vector<TestCase>&	Registry()
{
	static std::vector<TestCase> registry;
	return registry;
}

int	g_TotalChecks	= 0;
int	g_TestFailures	= 0;	// failed checks inside the running test

} // namespace

//----------------------------------------------------------------------
Registrar::Registrar(const char* suite, const char* name, TestFn fn)
{
	TestCase testCase;

	testCase.suite	= suite;
	testCase.name	= name;
	testCase.fn	= fn;

	Registry().push_back(testCase);
}

//----------------------------------------------------------------------
void	RecordCheck()
{
	g_TotalChecks++;
}

//----------------------------------------------------------------------
void	RecordFailure(const char* file, int line, const char* expr)
{
	g_TestFailures++;

	std::printf("      %s(%d): failed: %s\n", file, line, expr);
}

//----------------------------------------------------------------------
void	RecordFailureInt(const char* file, int line, const char* expr,
			 long long expected, long long actual)
{
	g_TestFailures++;

	std::printf("      %s(%d): failed: %s (expected %lld, actual %lld)\n",
		    file, line, expr, expected, actual);
}

//----------------------------------------------------------------------
int	RunAll()
{
	const std::vector<TestCase>&	tests	= Registry();

	int	failedTests	= 0;

	std::printf("Running %d test(s)\n\n", (int)tests.size());

	for (size_t i = 0; i < tests.size(); i++)
	{
		const TestCase&	testCase = tests[i];

		g_TestFailures = 0;

		testCase.fn();

		if (g_TestFailures == 0)
		{
			std::printf("  [PASS] %s.%s\n", testCase.suite, testCase.name);
		}
		else
		{
			std::printf("  [FAIL] %s.%s (%d failed check(s))\n",
				    testCase.suite, testCase.name, g_TestFailures);

			failedTests++;
		}
	}

	std::printf("\n%d test(s), %d check(s), %d failed\n",
		    (int)tests.size(), g_TotalChecks, failedTests);

	return failedTests;
}

} // namespace testfw
