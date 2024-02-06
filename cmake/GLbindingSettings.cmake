set(OPTION_SELF_CONTAINED          OFF CACHE BOOL "Create a self-contained install with all dependencies." FORCE)
set(OPTION_BUILD_TESTS             OFF CACHE BOOL "Build tests." FORCE)
set(OPTION_BUILD_DOCS              OFF CACHE BOOL "Build documentation." FORCE)
set(OPTION_BUILD_TOOLS             OFF CACHE BOOL "Build tools." FORCE)
set(OPTION_BUILD_EXAMPLES          OFF CACHE BOOL "Build examples." FORCE)
set(OPTION_BUILD_WITH_BOOST_THREAD OFF CACHE BOOL "Use boost::thread instead of std::thread." FORCE)
set(OPTION_BUILD_CHECK             OFF CACHE BOOL "Enable source code check targets" FORCE)
set(OPTION_BUILD_OWN_KHR_HEADERS   OFF CACHE BOOL "Build and deploy glbinding with in-source KHR headers." FORCE)
set(OPTION_USE_GIT_INFORMATION     ON  CACHE BOOL "Fetches commit information from the git source repository" FORCE)