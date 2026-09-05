/*-----------------------------------------------------------------------------

	DirectoryListing.h

	Directory enumeration on std::filesystem, replacing the _findfirst /
	_findnext / _findclose walks that were spread through the client
	(docs/cpp17-cpp20-compatibility-assessment-2026-09-04.md, priority 6).

	The point of the helper is that it owns no search handle: the caller
	gets a snapshot vector and there is no _findclose to forget, no
	intptr_t handle to truncate into a long, and no window in which the
	loop body can be re-entered by an entry it has just created.

	-------------------------------------------------------------------------
	Semantics PRESERVED from _findfirst / FindFirstFileA
	-------------------------------------------------------------------------

	  * Wildcards. '*' matches zero or more characters, dots included.
	    '?' matches exactly one character. Verified against FindFirstFileW
	    on NTFS: "*.spk*" matches "x.spk.tmp" - that is, the '*' before a
	    '.' is an ordinary star here, not the "up to the final dot"
	    DOS_STAR of the kernel's own expression evaluator.

	  * Case-insensitive matching, so "*.spk*" finds "Upper.SPK".

	  * Return ORDER. _findfirst on NTFS yields entries in the directory
	    index order, which is a case-insensitive ordinal sort of the name.
	    ListDirectory sorts its result the same way, so an NTFS listing
	    comes back in the order the legacy walk produced. (On FAT/exFAT
	    _findfirst yields creation order instead; there the helper is
	    sorted where the legacy walk was not.)

	  * Directories are enumerated when the caller asks for them, exactly
	    as the legacy patterns did - "*.*" matched subdirectories too.

	-------------------------------------------------------------------------
	Semantics DELIBERATELY NOT preserved
	-------------------------------------------------------------------------

	  * 8.3 short names. FindFirstFile also matches a pattern against the
	    short name NTFS keeps for a long file name, so "*.SPK" can find a
	    file called "long name.spkbackup" through its "LONGNA~1.SPK" alias.
	    That is not emulated and will not be; a caller that needs it has
	    the wrong tool.

	  * DOS_DOT. Win32 treats "*.*" as "*": verified against FindFirstFileW
	    on NTFS, the two return the identical set, a name with no dot
	    included. Here the '.' in a pattern is a literal, so "*.*" finds
	    only names that contain a dot. A walk being migrated off a "*.*"
	    pattern has to ask for "*" instead - InitProfiles does - or it
	    silently loses every dotless entry.

	  * "." and "..". std::filesystem::directory_iterator never produces
	    them, so they are never in the result. _findfirst returned both,
	    first, for any pattern that matched them ("*" and, on Win32,
	    "*.*" do).

	  * DOS_QM. The kernel lets a trailing '?' match zero characters, so
	    "ab?" finds "ab" as well as "abc". Here '?' is exactly one
	    character. No pattern in this tree uses '?'.

	  * Byte-wise, ASCII-only case folding. Matching and ordering fold
	    'a'-'z' only and compare the remaining bytes as unsigned values.
	    Win32 matches and NTFS sorts on upcased UTF-16, so for a name
	    outside ASCII in the active narrow code page (CP949 here) both the
	    fold and the order can differ - a CP949 trail byte that happens to
	    look like an ASCII letter is folded, which Win32 would not do.

	  * A snapshot, not a live walk. Entries created or removed while the
	    caller iterates the returned vector are invisible to it, where
	    _findnext might or might not have surfaced them.

	  * All or nothing on a mid-walk error. If the iterator fails part way
	    through, ListDirectory clears the result and returns false, so the
	    caller processes none of the entries. _findnext reported such an
	    error with the same non-zero return as end-of-list, so the legacy
	    loop kept whatever it had already handled.

	Nothing here throws: a directory that is absent or cannot be read is
	reported by a false return with an empty result, and every filesystem
	call is made through its std::error_code overload.

	2026.09.05

-----------------------------------------------------------------------------*/

#ifndef __DIRECTORY_LISTING_H__
#define __DIRECTORY_LISTING_H__

#include <string>
#include <vector>

namespace Basic {

/*-----------------------------------------------------------------------------
  One enumerated entry. The name carries no directory part, matching
  _finddata_t::name, and is in the narrow encoding the rest of the client
  uses for paths.
-----------------------------------------------------------------------------*/
struct SDirectoryEntry
{
	std::string	sName;
	bool		bIsDirectory;
};

/*-----------------------------------------------------------------------------
  Whether subdirectories join the result. LIST_FILES_ONLY is the sane
  default for new code. LIST_FILES_AND_DIRECTORIES is what a legacy
  _findfirst walk saw; a migrated walk asks for it only when its body
  could act on a directory (InitProfiles does), and a walk that only
  remove()s or opens each entry lists files, because neither call can
  act on a directory.
-----------------------------------------------------------------------------*/
enum EListFilter
{
	LIST_FILES_ONLY,
	LIST_FILES_AND_DIRECTORIES
};

/*-----------------------------------------------------------------------------
  DOS-style wildcard match, case-insensitive over ASCII. A NULL name or
  pattern matches nothing. An empty pattern matches only an empty name,
  which no directory entry has.
-----------------------------------------------------------------------------*/
bool	MatchesWildcard(const char* pName, const char* pPattern);

/*-----------------------------------------------------------------------------
  Lists the entries of pDirectory whose name matches pPattern, sorted as
  described above. Returns false - with vEntries empty - when the
  directory does not exist or cannot be enumerated; returns true with an
  empty vector when it is readable but nothing matches.
-----------------------------------------------------------------------------*/
bool	ListDirectory(const char* pDirectory, const char* pPattern,
		std::vector<SDirectoryEntry>& vEntries,
		EListFilter eFilter = LIST_FILES_ONLY);

} // namespace Basic

#endif
