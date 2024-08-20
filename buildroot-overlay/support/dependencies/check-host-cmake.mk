# The cmake minimum version is set to either 3.18 or higher,
# depending on the highest minimum version required by any
# of the packages bundled in Buildroot. If a package is
# bumped or a new one added, and it requires a higher
# cmake version than the one provided by the host, our
# cmake infra will catch it and build its own.
#

BR2_CMAKE = $(HOST_DIR)/bin/cmake
BR2_CMAKE_HOST_DEPENDENCY = host-cmake
