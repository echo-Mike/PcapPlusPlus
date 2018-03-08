#ifndef PCAPPP_CPP11DETECTION
#define PCAPPP_CPP11DETECTION

#include <cstdlib>
#include <cstddef>

/// @file

#if !defined(SUPPRESS_CPP11_DETECTION)

	// C++11 auto-detect feature is supported for: GCC/G++, ICC, CLANG(Xcode partially), VS(MSVC) 

	/* Information sources */
	// See: http://nadeausoftware.com/articles/2012/01/c_c_tip_how_use_compiler_predefined_macros_detect_operating_system
	// See: https://sourceforge.net/p/predef/wiki/Compilers/
	// See: http://en.cppreference.com/w/cpp/compiler_support
	// See: http://nadeausoftware.com/articles/2012/10/c_c_tip_how_detect_compiler_name_and_version_using_compiler_predefined_macros
	/* Information sources */

	/* Show error message for compilers for which C++11 detection feature is not supported */
	#if !( defined(__GNUC__) || defined(__INTEL_COMPILER) || defined(__ICC) || defined(__clang__) || defined(_MSC_VER) )

		#error This compiler is a target to be done for automatic C++11 detection. Please specify ENABLE_CPP11_MOVE_SEMANTICS macro by hand if you compiler version support C++11 rvalue ref v2.0. \
To suppress automatic C++11 detection feature define SUPPRESS_CPP11_DETECTION macro. This error does NOT mean that library can not be build on Your compiler. \
And if You can please add issue to https://github.com/seladb/PcapPlusPlus

	#endif

	/* Compiler based definitions */

	// GCC
	#if defined(__GNUC__) && !(defined(__INTEL_COMPILER) || defined(__ICC) || defined(__clang__))
		// See : https://gcc.gnu.org/projects/cxx-status.html#cxx11
		#if defined(__GNUC_PATCHLEVEL__)
			#define __GNUC_VERSION__ (__GNUC__ * 10000 \
										+ __GNUC_MINOR__ * 100 \
										+ __GNUC_PATCHLEVEL__)
		#else
			#define __GNUC_VERSION__ (__GNUC__ * 10000 \
										+ __GNUC_MINOR__ * 100)
		#endif
	#endif /* __GNUC__ */
	// CLANG
	#if defined(__clang__)
		// See : https://clang.llvm.org/cxx_status.html
		// See : https://trac.macports.org/wiki/XcodeVersionInfo
		// See : http://clang.llvm.org/docs/LanguageExtensions.html#feature-checking-macros
		#if defined(__clang_patchlevel__)
			#define __clang_VERSION__ (__clang_major__ * 10000 \
										+ __clang_minor__ * 100 \
										+ __clang_patchlevel__)
		#else
			#define __clang_VERSION__ (__clang_major__ * 10000 \
										+ __clang_minor__ * 100 )
		#endif
	#endif /* __clang__ */
	// ICC
	#if defined(__INTEL_COMPILER) || defined(__ICC)
		// See : https://software.intel.com/en-us/node/524490
		// See : https://software.intel.com/en-us/articles/c0x-features-supported-by-intel-c-compiler
		// See : http://clang.llvm.org/docs/LanguageExtensions.html#feature-checking-macros
		#if defined(__ICC) && !defined(__INTEL_COMPILER)
			#define __ICC_VERSION__ __ICC
		#else
			#define __ICC_VERSION__ __INTEL_COMPILER
		#endif
	#endif /* ICC */

	/* Detection */

	#if defined(WIN32) || defined(_WIN64) || defined(_WIN32) || defined(__MINGW32__) || defined(__MINGW64__)
		#if defined(__MINGW32__) || defined(__CYGWIN32__) || defined(__CYGWIN__)
			// MinGW or CygWin builds : CLANG and GCC are supported
			#if defined(__clang__)
				// CLANG
				#define ENABLE_CPP11_MOVE_SEMANTICS
				#if (__clang_VERSION__ < 30100)
					#define PCAPPP_CONSTEXPR const
				#else
					#define PCAPPP_CONSTEXPR constexpr
				#endif
				#if (__clang_VERSION__ < 30000)
					#define PCAPPP_NOEXCEPT
				#else
					#define PCAPPP_NOEXCEPT noexcept
				#endif
				#if (__clang_VERSION__ < 20900)
					#define PCAPPP_FINAL
					#define PCAPPP_OVERRIDE
					#define PCAPPP_NULLPTR NULL
					#define NO_TEMPLATE_FUNCTION_DEF_ARGS
				#else
					#define PCAPPP_FINAL final
					#define PCAPPP_OVERRIDE override
					#define PCAPPP_HAVE_NULLPTR_T
					#define PCAPPP_NULLPTR nullptr
				#endif
			#elif defined(__GNUC__) && !(defined(__INTEL_COMPILER) || defined(__ICC) || defined(__clang__))
				// GCC
				#if (__GNUC_VERSION__ < 40700)
					#define PCAPPP_FINAL
					#define PCAPPP_OVERRIDE
				#else
					#define PCAPPP_FINAL final
					#define PCAPPP_OVERRIDE override
				#endif
				#if (__GNUC_VERSION__ < 40600)
					#define PCAPPP_NOEXCEPT
					#define PCAPPP_NULLPTR NULL
					#define PCAPPP_CONSTEXPR const
				#else
					#define PCAPPP_NOEXCEPT noexcept
					#define PCAPPP_HAVE_NULLPTR_T
					#define PCAPPP_NULLPTR nullptr
					#define PCAPPP_CONSTEXPR constexpr
				#endif
				#if (__GNUC_VERSION__ < 40300)
					#define NO_TEMPLATE_FUNCTION_DEF_ARGS
				#else
					#define ENABLE_CPP11_MOVE_SEMANTICS
				#endif
			#else
				#error PCAPPP::C++11_Detection Unknown compiler type.
			#endif
		#elif defined(_MSC_VER)
			// Visual Studio
			// See : https://en.wikipedia.org/wiki/Microsoft_Visual_C++
			// See : https://msdn.microsoft.com/en-us/library/hh567368.aspx#featurelist
			#if (_MSC_VER < 1900)				
				#define PCAPPP_NOEXCEPT
				#define PCAPPP_CONSTEXPR const
			#else
				#define PCAPPP_CONSTEXPR constexpr
				#define PCAPPP_NOEXCEPT noexcept
			#endif
			#if (_MSC_VER < 1800)
				#define NO_TEMPLATE_FUNCTION_DEF_ARGS
			#endif
			#if (_MSC_VER < 1600)
				#define PCAPPP_NULLPTR NULL
			#else
				#define PCAPPP_HAVE_NULLPTR_T
				#define PCAPPP_NULLPTR nullptr
				#define ENABLE_CPP11_MOVE_SEMANTICS
			#endif
			#if (_MSC_VER < 1400)
				#define PCAPPP_FINAL
				#define PCAPPP_OVERRIDE
			#else
				#define PCAPPP_FINAL final
				#define PCAPPP_OVERRIDE override
			#endif
		#elif defined(__INTEL_COMPILER) || defined(__ICC)
			// ICC
			#if (__ICC_VERSION__ < 1400)				
				#define PCAPPP_CONSTEXPR const
				#define PCAPPP_NOEXCEPT
				#define PCAPPP_FINAL
				#define PCAPPP_OVERRIDE
			#else
				#define PCAPPP_CONSTEXPR constexpr
				#define PCAPPP_NOEXCEPT noexcept
				#define PCAPPP_FINAL final
				#define PCAPPP_OVERRIDE override
				#define ENABLE_CPP11_MOVE_SEMANTICS
			#endif
			#if (__ICC_VERSION__ < 1210)
				#define PCAPPP_NULLPTR NULL
				#define NO_TEMPLATE_FUNCTION_DEF_ARGS
			#else
				#define PCAPPP_HAVE_NULLPTR_T
				#define PCAPPP_NULLPTR nullptr
			#endif
		#else
			#error PCAPPP::C++11_Detection Unknown compiler type.
		#endif
	#elif defined(__unix__) || defined(__unix) || defined(unix) || defined(__linux__) || (defined(__APPLE__) && defined(__MACH__))
		#if defined(__APPLE__) && defined(__MACH__)
			// It is hard to detect version on Apple platform -> use standard macro defs
			// There may be CLANG or GCC
			// TODO: Add information sources about CLANG and GCC C++11 compatibility on Apple platforms
			#ifndef __cpp_constexpr
				#define PCAPPP_CONSTEXPR const
			#else
				#define PCAPPP_CONSTEXPR constexpr
			#endif
			#ifdef __cpp_rvalue_references
				#define ENABLE_CPP11_MOVE_SEMANTICS
			#endif
			#define PCAPPP_NOEXCEPT
			#define PCAPPP_FINAL
			#define PCAPPP_OVERRIDE
			#define PCAPPP_NULLPTR NULL
			#define NO_TEMPLATE_FUNCTION_DEF_ARGS
		#else
			#if defined(__clang__)
				// CLANG
				#define ENABLE_CPP11_MOVE_SEMANTICS
				#if (__clang_VERSION__ < 30100)
					#define PCAPPP_CONSTEXPR const
				#else
					#define PCAPPP_CONSTEXPR constexpr
				#endif
				#if (__clang_VERSION__ < 30000)
					#define PCAPPP_NOEXCEPT
				#else
					#define PCAPPP_NOEXCEPT noexcept
				#endif
				#if (__clang_VERSION__ < 20900)
					#define PCAPPP_FINAL
					#define PCAPPP_OVERRIDE
					#define PCAPPP_NULLPTR NULL
					#define NO_TEMPLATE_FUNCTION_DEF_ARGS
				#else
					#define PCAPPP_FINAL final
					#define PCAPPP_OVERRIDE override
					#define PCAPPP_HAVE_NULLPTR_T
					#define PCAPPP_NULLPTR nullptr
				#endif
			#elif defined(__GNUC__) && !(defined(__INTEL_COMPILER) || defined(__ICC) || defined(__clang__))
				// GCC
				#if (__GNUC_VERSION__ < 40700)
					#define PCAPPP_FINAL
					#define PCAPPP_OVERRIDE
				#else
					#define PCAPPP_FINAL final
					#define PCAPPP_OVERRIDE override
				#endif
				#if (__GNUC_VERSION__ < 40600)
					#define PCAPPP_NOEXCEPT
					#define PCAPPP_NULLPTR NULL
					#define PCAPPP_CONSTEXPR const
				#else
					#define PCAPPP_NOEXCEPT noexcept
					#define PCAPPP_HAVE_NULLPTR_T
					#define PCAPPP_NULLPTR nullptr
					#define PCAPPP_CONSTEXPR constexpr
				#endif
				#if (__GNUC_VERSION__ < 40300)
					#define NO_TEMPLATE_FUNCTION_DEF_ARGS
				#else
					#define ENABLE_CPP11_MOVE_SEMANTICS
				#endif
			#elif defined(__INTEL_COMPILER) || defined(__ICC)
				// ICC
				#if (__ICC_VERSION__ < 1400)				
					#define PCAPPP_CONSTEXPR const
					#define PCAPPP_NOEXCEPT
					#define PCAPPP_FINAL
					#define PCAPPP_OVERRIDE
				#else
					#define PCAPPP_CONSTEXPR constexpr
					#define PCAPPP_NOEXCEPT noexcept
					#define PCAPPP_FINAL final
					#define PCAPPP_OVERRIDE override
					#define ENABLE_CPP11_MOVE_SEMANTICS
				#endif
				#if (__ICC_VERSION__ < 1210)
					#define PCAPPP_NULLPTR NULL
					#define NO_TEMPLATE_FUNCTION_DEF_ARGS
				#else
					#define PCAPPP_HAVE_NULLPTR_T
					#define PCAPPP_NULLPTR nullptr
				#endif
			#else
				#error PCAPPP::C++11_Detection Unknown compiler type.
			#endif
		#endif
	#else
		// Try to check C++11 features based on standard macros:
		// See : http://en.cppreference.com/w/cpp/experimental/feature_test
		// We currently have interest only in:
		#ifndef __cpp_constexpr
			#define PCAPPP_CONSTEXPR const
		#else
			#define PCAPPP_CONSTEXPR constexpr
		#endif
		#ifdef __cpp_rvalue_references
			#define ENABLE_CPP11_MOVE_SEMANTICS
		#endif
		#define PCAPPP_NOEXCEPT
		#define PCAPPP_FINAL
		#define PCAPPP_OVERRIDE
		#define PCAPPP_NULLPTR NULL
		#define NO_TEMPLATE_FUNCTION_DEF_ARGS
	#endif

#else
	#ifndef __cpp_constexpr
		#define PCAPPP_CONSTEXPR const
	#else
		#define PCAPPP_CONSTEXPR constexpr
	#endif
	#ifdef __cpp_rvalue_references
		//#define ENABLE_CPP11_MOVE_SEMANTICS
	#endif
	#define PCAPPP_NOEXCEPT
	#define PCAPPP_FINAL
	#define PCAPPP_OVERRIDE
	#define PCAPPP_NULLPTR NULL
	#define NO_TEMPLATE_FUNCTION_DEF_ARGS
#endif /* SUPPRESS_CPP11_DETECTION */

#endif /* PCAPPP_CPP11DETECTION */