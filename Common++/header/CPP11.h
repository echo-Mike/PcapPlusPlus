#ifndef PCAPPP_CPP11DETECTION
#define PCAPPP_CPP11DETECTION

#include <cstdlib>
#include <cstddef>

/// @file

#if !defined(SUPPRESS_CPP11_DETECTION)

	// C++11 autodetect feature is supported for: GCC/G++, ICC, CLANG(Xcode partially), VS(MSVC) 

	/* Information sources */
	// See: http://nadeausoftware.com/articles/2012/01/c_c_tip_how_use_compiler_predefined_macros_detect_operating_system
	// See: https://sourceforge.net/p/predef/wiki/Compilers/
	// See: http://en.cppreference.com/w/cpp/compiler_support
	// See: http://nadeausoftware.com/articles/2012/10/c_c_tip_how_detect_compiler_name_and_version_using_compiler_predefined_macros
	/* Information sources */

	/* Compilers with unsupported C++11 detection feature */
	#if defined(_ACC_) || defined(__CMB__) || defined(__CHC__) || defined(__ACK__) || defined(__CC_ARM) || defined(__AZTEC_C__) || \
		defined(__CC65__) || defined(__COMO__) || defined(__DECC) || defined(__convexc__) || defined(__COVERITY__) || defined(_CRAYC) || \
		defined(__DCC__) || defined(_DICE) || defined(__DMC__) || defined(__SYSC__) || defined(__DJGPP__) || defined(__PATHCC__) || \
		defined(__FCC_VERSION) || defined(__ghs__) || defined(__HP_cc) || defined(__HP_aCC) || defined(__IAR_SYSTEMS_ICC__) || defined(__xlc__) || \
		defined(__IBMC__) || defined(__IBMCPP__) || defined(__IMAGECRAFT__) || defined(__KCC) || defined(__CA__) || defined(__C166__) || \
		defined(__C51__) || defined(__LCC__) || defined(__llvm__) || defined(__HIGHC__) || defined(__MWERKS__) || defined(_MRI) || \
		defined(__NDPC__) || defined(__NDPX__) || defined(__sgi) || defined(MIRACLE) || defined(__MRC__) || defined(__CC_NORCROFT) || \
		defined(__NWCC__) || defined(__OPEN64__) || defined(ORA_PROC) || defined(__SUNPRO_C) || defined(__PACIFIC__) || defined(_PACC_VER) || \
		defined(__POCC__) || defined(__PGI) || defined(__RENESAS__) || defined(SASC) || defined(__SASC) || defined(__SASC__) || \
		defined(_SCO_DS) || defined(SDCC) || defined(__SNC__) || defined(__VOSC__) || defined(__SC__) || defined(__TenDRA__) || \
		defined(__TI_COMPILER_VERSION__) || defined(THINKC3) || defined(THINKC4) || defined(__TINYC__) || defined(__TURBOC__) || defined(_UCC) || \
		defined(__USLC__) || defined(__VBCC__) || defined(__WATCOMC__) || defined(__ZTC__)

		// defined(__EDG__) // EDG C++ Frontend - this macro is defined by VS2015
		#error \
		This compiler is a target to be done for automatic C++11 detection. \
		Please specify ENABLE_CPP11_MOVE_SEMANTICS macro by hand if you compiler version support C++11 rvalue ref v2.0. \
		To suppress automatic C++11 detection feature define SUPPRESS_CPP11_DETECTION macro. \
		This error does NOT mean that library can't be build on Your compiler. \
		And if You can please add issue to https://github.com/seladb/PcapPlusPlus
	#endif
	/* Compilers with unsupported C++11 detection feature */

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
	/* Compiler based definitions */

	/* Detection */
	#if defined(WIN32) || defined(_WIN64) || defined(_WIN32) || defined(__MINGW32__) || defined(__MINGW64__)
		#if defined(__MINGW32__) || defined(__CYGWIN32__) || defined(__CYGWIN__)
			// MinGW or CygWin builds : CLANG and GCC are supported
			#if defined(__clang__)
				// CLANG
				#define ENABLE_CPP11_MOVE_SEMANTICS
				#if (__clang_VERSION__ < 30100)
					#ifndef constexpr
						#define constexpr const
					#endif
				#endif
				#if (__clang_VERSION__ < 30000)
					#ifndef noexcept
						#define noexcept
					#endif
				#endif
				#if (__clang_VERSION__ < 20900)
					#ifndef override
						#define override
					#endif
					#ifndef final
						#define final
					#endif
					#ifndef nullptr
						#define nullptr NULL
					#endif
					#define NO_TEMPLATE_FUNCTION_DEF_ARGS
				#endif
			#elif defined(__GNUC__) && !(defined(__INTEL_COMPILER) || defined(__ICC) || defined(__clang__))
				// GCC
				#if (__GNUC_VERSION__ < 40700)
					#ifndef override
						#define override
					#endif
					#ifndef final
						#define final
					#endif
				#endif
				#if (__GNUC_VERSION__ < 40600)
					#ifndef nullptr
						#define nullptr NULL
					#endif
					#ifndef noexcept
						#define noexcept
					#endif
					#ifndef constexpr
						#define constexpr const
					#endif
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
				#ifndef constexpr
					#define constexpr const
				#endif
				#ifndef noexcept
					#define noexcept
				#endif
			#endif
			#if (_MSC_VER < 1800)
				#define NO_TEMPLATE_FUNCTION_DEF_ARGS
			#endif
			#if (_MSC_VER < 1600)
				#ifndef nullptr
					#define nullptr NULL
				#endif
			#else
				#define ENABLE_CPP11_MOVE_SEMANTICS
			#endif
			#if (_MSC_VER < 1400)
				#ifndef override
					#define override
				#endif
				#ifndef final
					#define final
				#endif
			#endif
		#elif defined(__INTEL_COMPILER) || defined(__ICC)
			// ICC
			#if (__ICC_VERSION__ < 1400)
				#ifndef constexpr
					#define constexpr const
				#endif
				#ifndef noexcept
					#define noexcept
				#endif
				#ifndef override
					#define override
				#endif
				#ifndef final
					#define final
				#endif
			#else
				#define ENABLE_CPP11_MOVE_SEMANTICS
			#endif
			#if (__ICC_VERSION__ < 1210)
				#ifndef nullptr
					#define nullptr NULL
				#endif
				#define NO_TEMPLATE_FUNCTION_DEF_ARGS
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
				#define constexpr const
			#endif
			#ifdef __cpp_rvalue_references
				#define ENABLE_CPP11_MOVE_SEMANTICS
			#endif
			// Next def may cause problems if override keyword is supported
			#ifndef override
				#define override
			#endif
			#ifndef nullptr
				#define nullptr NULL
			#endif
			#define NO_TEMPLATE_FUNCTION_DEF_ARGS
		#else
			#if defined(__clang__)
				// CLANG
				#define ENABLE_CPP11_MOVE_SEMANTICS
				#if (__clang_VERSION__ < 30100)
					#ifndef constexpr
						#define constexpr const
					#endif
				#endif
				#if (__clang_VERSION__ < 30000)
					#ifndef noexcept
						#define noexcept
					#endif
				#endif
				#if (__clang_VERSION__ < 20900)
					#ifndef override
						#define override
					#endif
					#ifndef final
						#define final
					#endif
					#ifndef nullptr
						#define nullptr NULL
					#endif
					#define NO_TEMPLATE_FUNCTION_DEF_ARGS
				#endif
			#elif defined(__GNUC__) && !(defined(__INTEL_COMPILER) || defined(__ICC) || defined(__clang__))
				// GCC
				#if (__GNUC_VERSION__ < 40700)
					#ifndef override
						#define override
					#endif
					#ifndef final
						#define final
					#endif
				#endif
				#if (__GNUC_VERSION__ < 40600)
					#ifndef nullptr
						#define nullptr NULL
					#endif
					#ifndef noexcept
						#define noexcept
					#endif
					#ifndef constexpr
						#define constexpr const
					#endif
				#endif
				#if (__GNUC_VERSION__ < 40300)
					#define NO_TEMPLATE_FUNCTION_DEF_ARGS
				#else
					#define ENABLE_CPP11_MOVE_SEMANTICS
				#endif
			#elif defined(__INTEL_COMPILER) || defined(__ICC)
				// ICC
				#if (__ICC_VERSION__ < 1400)
					#ifndef constexpr
						#define constexpr const
					#endif
					#ifndef noexcept
						#define noexcept
					#endif
					#ifndef override
						#define override
					#endif
					#ifndef final
						#define final
					#endif
				#else
					#define ENABLE_CPP11_MOVE_SEMANTICS
				#endif
				#if (__ICC_VERSION__ < 1210)
					#ifndef nullptr
						#define nullptr NULL
					#endif
					#define NO_TEMPLATE_FUNCTION_DEF_ARGS
				#endif
			#else
				#error PCAPPP::C++11_Detection Unknown compiler type.
			#endif
		#endif
	#else
		// Try to check C++11 features based on standard macros:
		// See : http://en.cppreference.com/w/cpp/experimental/feature_test
		// We currentry have interest only in:
		#ifndef __cpp_constexpr
			#define constexpr const
		#endif
		// Next def may cause problems in unknown compilers with override keyword support
		#ifndef override
			#define override
		#endif
		#ifndef nullptr
			#define nullptr NULL
		#endif
		#ifdef __cpp_rvalue_references
			#define ENABLE_CPP11_MOVE_SEMANTICS
		#endif
		#define NO_TEMPLATE_FUNCTION_DEF_ARGS
	#endif
	/* Detection */

#endif /* SUPPRESS_CPP11_DETECTION */

#endif /* PCAPPP_CPP11DETECTION */