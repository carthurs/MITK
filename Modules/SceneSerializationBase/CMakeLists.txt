MITK_CREATE_MODULE(
  INCLUDE_DIRS BasePropertySerializer
  DEPENDS MitkCore
  PACKAGE_DEPENDS Poco
  WARNINGS_AS_ERRORS
)
add_subdirectory(Testing)

